#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/error.h"
#include "mbedtls/platform.h"

#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_crypto.h"
#include "iotjs_module_buffer.h"

#define DEBUG_LEVEL 1

typedef struct {
  iotjs_jobjectwrap_t   jobjectwrap;

  /**
   * SSL common structure
   */
  mbedtls_x509_crt      ca_;
  mbedtls_ssl_context   ssl_;
  mbedtls_ssl_config    config_;
  uv_work_t             worker;

  /**
   * read
   */
  uv_work_t             read_req;
  unsigned char*        read_buf_;
  size_t                read_buflen_;
  int                   read_result_;

  /**
   * output function
   */
  uv_async_t            sender_;
  const unsigned char*  sender_buf_;
  size_t                sender_buflen_;
  uv_sem_t              sender_locker_;

  /**
   * input function
   */
  uv_async_t            receiver_;
  const unsigned char*  receiver_buf_;
  size_t                receiver_buflen_;
} IOTJS_VALIDATED_STRUCT(iotjs_tlswrap_t);

static JNativeInfoType this_module_native_info = { .free_cb = NULL };

/* List of trusted root CA certificates
 * currently only GlobalSign, the CA for os.mbed.com
 *
 * To add more than one root, just concatenate them.
 */
const char SSL_CA_PEM[] = 
  "-----BEGIN CERTIFICATE-----\n"
  "MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4GA1UECxMX\n"
  "R2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNpZ24xEzARBgNVBAMT\n"
  "Ckdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1MDgwMDAwWjBMMSAwHgYDVQQL\n"
  "ExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEGA1UEChMKR2xvYmFsU2lnbjETMBEGA1UE\n"
  "AxMKR2xvYmFsU2lnbjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8o\n"
  "mUVCxKs+IVSbC9N/hHD6ErPLv4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7\n"
  "SqbKSaZeqKeMWhG8eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQ\n"
  "BoZfXklqtTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n"
  "C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pazq+r1feq\n"
  "CapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCBmTAOBgNVHQ8BAf8E\n"
  "BAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IHV2ccHsBqBt5ZtJot39wZhi4w\n"
  "NgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5nbG9iYWxzaWduLm5ldC9yb290LXIyLmNy\n"
  "bDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEA\n"
  "mYFThxxol4aR7OBKuEQLq4GsJ0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkI\n"
  "k7mpM0sYmsL4h4hO291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRD\n"
  "LenVOavSot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n"
  "AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7TBj0/VLZ\n"
  "jmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n"
  "-----END CERTIFICATE-----\n";

static iotjs_tlswrap_t* iotjs_tlswrap_create(const jerry_value_t value) {
  iotjs_tlswrap_t* tlswrap = IOTJS_ALLOC(iotjs_tlswrap_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_tlswrap_t, tlswrap);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, 
                               value,
                               &this_module_native_info);

  mbedtls_x509_crt_init(&_this->ca_);
  mbedtls_ssl_init(&_this->ssl_);
  mbedtls_ssl_config_init(&_this->config_);
  return tlswrap;
}

static void print_mbedtls_error(const char *name, int err) {
  char buf[128];
  mbedtls_strerror(err, buf, sizeof (buf));
  mbedtls_printf("%s() failed: -0x%04x (%d): %s\n", name, -err, err, buf);
}

// static void iotjs_tlswrap_destroy(iotjs_tlswrap_t* tlswrap) {
//   IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_tlswrap_t, tlswrap);
//   iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
//   IOTJS_RELEASE(tlswrap);
// }

static int iotjs_tlswrap_bio_recv(void *ctx, unsigned char *buf, size_t len) {
  iotjs_tlswrap_t_impl_t* _this = (iotjs_tlswrap_t_impl_t*)ctx;
  _this->receiver_buf_ = NULL;
  _this->receiver_buflen_ = len;

  while (_this->receiver_buf_ == NULL) {
    uv_sleep(100);
    uv_async_send(&_this->receiver_);
  }

  memcpy(buf, _this->receiver_buf_, _this->receiver_buflen_);
  free((void*)_this->receiver_buf_);
  _this->receiver_buflen_ = 0;
  return len;
}

static void iotjs_tlswrap_reciver_cb(uv_async_t* handle) {
  iotjs_tlswrap_t_impl_t* _this = (iotjs_tlswrap_t_impl_t*)(handle->data);
  jerry_value_t jthis = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  jerry_value_t fn = iotjs_jval_get_property(jthis, "onread");

  iotjs_jargs_t jargv = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&jargv, (double)(_this->receiver_buflen_));
  jerry_value_t ret = iotjs_make_callback_with_result(fn, jthis, &jargv);

  if (!jerry_value_is_null(ret)) {
    iotjs_bufferwrap_t* bufwrap = iotjs_bufferwrap_from_jbuffer(ret);
    char* src = iotjs_bufferwrap_buffer(bufwrap);
    _this->receiver_buf_ = (void*)malloc(_this->receiver_buflen_);
    memcpy((void*)_this->receiver_buf_, src, _this->receiver_buflen_);
  }
  iotjs_jargs_destroy(&jargv);
  jerry_release_value(fn);
}

static int iotjs_tlswrap_bio_send(void *ctx, const unsigned char *buf, size_t len) {
  iotjs_tlswrap_t_impl_t* _this = (iotjs_tlswrap_t_impl_t*)ctx;
  uv_sem_wait(&_this->sender_locker_);
  _this->sender_buf_ = (void*)malloc(len);
  memcpy((void*)_this->sender_buf_, buf, len);
  _this->sender_buflen_ = len;
  uv_async_send(&_this->sender_);
  return len;
}

static void iotjs_tlswrap_sender_cb(uv_async_t* handle) {
  iotjs_tlswrap_t_impl_t* _this = (iotjs_tlswrap_t_impl_t*)(handle->data);
  jerry_value_t jthis = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  jerry_value_t fn = iotjs_jval_get_property(jthis, "onwrite");

  iotjs_jargs_t jargv = iotjs_jargs_create(1);
  jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer((size_t)_this->sender_buflen_);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);

  iotjs_bufferwrap_copy(buffer_wrap, 
                        (const char*)_this->sender_buf_, 
                        (size_t)_this->sender_buflen_);
  iotjs_jargs_append_jval(&jargv, jbuffer);
  iotjs_make_callback(fn, jthis, &jargv);
  free((void*)_this->sender_buf_);
  _this->sender_buflen_ = 0;

  iotjs_jargs_destroy(&jargv);
  jerry_release_value(fn);
  uv_sem_post(&_this->sender_locker_);
}

JS_FUNCTION(TlsConstructor) {
  DJS_CHECK_THIS();

  const jerry_value_t jtls = JS_GET_THIS();
  iotjs_tlswrap_t* tlswrap = iotjs_tlswrap_create(jtls);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_tlswrap_t, tlswrap);
  
  jerry_value_t opts = jargv[0];

  uv_sem_init(&_this->sender_locker_, 1);
  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_async_init(loop, &_this->receiver_, iotjs_tlswrap_reciver_cb);
  _this->receiver_.data = (void*)_this;
  uv_async_init(loop, &_this->sender_, iotjs_tlswrap_sender_cb);
  _this->sender_.data = (void*)_this;

  int ret;
  if ((ret = mbedtls_x509_crt_parse(&_this->ca_, 
                                    (const unsigned char *) SSL_CA_PEM,
                                    sizeof(SSL_CA_PEM))) != 0) {
    return JS_CREATE_ERROR(COMMON, "parse x509 failed.");
  }

  if ((ret = mbedtls_ssl_config_defaults(&_this->config_,
                                         MBEDTLS_SSL_IS_CLIENT,
                                         MBEDTLS_SSL_TRANSPORT_STREAM,
                                         MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
    return JS_CREATE_ERROR(COMMON, "SSL configuration failed.");
  }

  mbedtls_ssl_conf_ca_chain(&_this->config_, &_this->ca_, NULL);
  mbedtls_ssl_conf_rng(&_this->config_, mbedtls_ctr_drbg_random, &drgb_ctx);

  /* It is possible to disable authentication by passing
   * MBEDTLS_SSL_VERIFY_NONE in the call to mbedtls_ssl_conf_authmode()
   */
  jerry_value_t jrejectUnauthorized = iotjs_jval_get_property(opts, "rejectUnauthorized");
  bool rejectUnauthorized = iotjs_jval_as_boolean(jrejectUnauthorized);
  mbedtls_ssl_conf_authmode(&_this->config_, 
                            rejectUnauthorized ? 
                              MBEDTLS_SSL_VERIFY_REQUIRED : 
                              MBEDTLS_SSL_VERIFY_NONE);
  jerry_release_value(jrejectUnauthorized);

  if ((ret = mbedtls_ssl_setup(&_this->ssl_, &_this->config_)) != 0) {
    return JS_CREATE_ERROR(COMMON, "SSL failed.");
  }

  jerry_value_t jservername = iotjs_jval_get_property(opts, "servername");
  iotjs_string_t servername = iotjs_jval_as_string(jservername);
  mbedtls_ssl_set_hostname(&_this->ssl_, iotjs_string_data(&servername));
  jerry_release_value(jservername);

  return jerry_create_undefined();
}

static void iotjs_tlswrap_do_handshake(uv_work_t* req) {
  iotjs_tlswrap_t_impl_t* _this = (iotjs_tlswrap_t_impl_t*)(req->data);
  mbedtls_ssl_set_bio(&_this->ssl_,
                      (void*)_this,
                      iotjs_tlswrap_bio_send,
                      iotjs_tlswrap_bio_recv,
                      NULL);

  int ret;
  do {
    ret = mbedtls_ssl_handshake(&_this->ssl_);
  } while (ret != 0 && (ret == MBEDTLS_ERR_SSL_WANT_READ ||
           ret == MBEDTLS_ERR_SSL_WANT_WRITE));

  if (ret < 0) {
    print_mbedtls_error("mbedtls_ssl_handshake", ret);
    uv_cancel((uv_req_t*)req);
  }
}

static void iotjs_tlswrap_after_handshake(uv_work_t* req, int status) {
  iotjs_tlswrap_t_impl_t* _this = (iotjs_tlswrap_t_impl_t*)(req->data);
  jerry_value_t jthis = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  jerry_value_t fn = iotjs_jval_get_property(jthis, "onhandshakedone");

  iotjs_jargs_t jargv = iotjs_jargs_create(1);
  iotjs_jargs_append_number(&jargv, status);
  iotjs_make_callback(fn, jthis, &jargv);
  iotjs_jargs_destroy(&jargv);
  jerry_release_value(fn);
}

JS_FUNCTION(TlsHandshake) {
  JS_DECLARE_THIS_PTR(tlswrap, tlswrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_tlswrap_t, tlswrap);

  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  _this->worker.data = (void*)_this;
  _this->read_req.data = (void*)_this;
  uv_queue_work(loop, 
                &_this->worker, 
                iotjs_tlswrap_do_handshake, 
                iotjs_tlswrap_after_handshake);
  return jerry_create_undefined();
}

JS_FUNCTION(TlsWrite) {
  JS_DECLARE_THIS_PTR(tlswrap, tlswrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_tlswrap_t, tlswrap);

  iotjs_bufferwrap_t* buf = iotjs_bufferwrap_from_jbuffer(jargv[0]);
  int ret = mbedtls_ssl_write(&_this->ssl_, 
                              (const unsigned char*)iotjs_bufferwrap_buffer(buf), 
                              (size_t)iotjs_bufferwrap_length(buf));
  if (ret < 0) {
    print_mbedtls_error("mbedtls_ssl_write", ret);
  }
  return jerry_create_number(ret);
}

static void iotjs_tlswrap_do_read(uv_work_t* req) {
  iotjs_tlswrap_t_impl_t* _this = (iotjs_tlswrap_t_impl_t*)(req->data);
  size_t len = _this->read_buflen_;
  unsigned char src[len];
  
  int bytes = -1;
  while (true) {
    bytes = mbedtls_ssl_read(&_this->ssl_, src, len);
    if (bytes == MBEDTLS_ERR_SSL_WANT_READ ||
        bytes == MBEDTLS_ERR_SSL_WANT_WRITE)
      continue;
    break;
  }

  if (bytes > 0) {
    _this->read_buf_ = (unsigned char*)malloc(len);
    if (_this->read_buf_ == NULL) {
      fprintf(stderr, "Out of Memory\n");
    }
    memset(_this->read_buf_, 0, bytes);
    memcpy(_this->read_buf_, src, bytes);
  } else {
    _this->read_buf_ = NULL;
    print_mbedtls_error("mbedtls_ssl_read", bytes);
  }
  _this->read_result_ = bytes;
}

static void iotjs_tlswrap_after_read(uv_work_t* req, int status) {
  iotjs_tlswrap_t_impl_t* _this = (iotjs_tlswrap_t_impl_t*)(req->data);
  jerry_value_t jthis = iotjs_jobjectwrap_jobject(&_this->jobjectwrap);
  jerry_value_t fn = iotjs_jval_get_property(jthis, "ondata");

  iotjs_jargs_t jargv = iotjs_jargs_create(2);
  iotjs_jargs_append_number(&jargv, _this->read_result_);
  // push buffer
  if (_this->read_buf_ != NULL) {
    jerry_value_t jdata = iotjs_bufferwrap_create_buffer((size_t)_this->read_buflen_);
    iotjs_bufferwrap_t* data = iotjs_bufferwrap_from_jbuffer(jdata);
    iotjs_bufferwrap_copy(data,
                          (const char*)_this->read_buf_,
                          (size_t)_this->read_buflen_);
    iotjs_jargs_append_jval(&jargv, jdata);
    jerry_release_value(jdata);
    if (_this->read_buf_ != NULL)
      free((void*)_this->read_buf_);
  }

  iotjs_make_callback(fn, jthis, &jargv);
  iotjs_jargs_destroy(&jargv);
  jerry_release_value(fn);

  if (_this->read_buf_ != NULL) {
  }
}

JS_FUNCTION(TlsRead) {
  JS_DECLARE_THIS_PTR(tlswrap, tlswrap);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_tlswrap_t, tlswrap);

  int size = JS_GET_ARG(0, number);
  _this->read_buflen_ = (size_t)size;
  _this->read_buf_ = malloc(_this->read_buflen_);
  if (_this->read_buf_ == NULL) {
    return JS_CREATE_ERROR(COMMON, "Out of Memory");
  }

  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_queue_work(loop, &_this->read_req, iotjs_tlswrap_do_read, iotjs_tlswrap_after_read);
  return jerry_create_undefined();
}

jerry_value_t InitTls() {
  jerry_value_t tls = jerry_create_object();
  jerry_value_t tlsConstructor =
      jerry_create_external_function(TlsConstructor);
  iotjs_jval_set_property_jval(tls, "TlsWrap", tlsConstructor);

  jerry_value_t proto = jerry_create_object();
  iotjs_jval_set_method(proto, "handshake", TlsHandshake);
  iotjs_jval_set_method(proto, "write", TlsWrite);
  iotjs_jval_set_method(proto, "read", TlsRead);
  iotjs_jval_set_property_jval(tlsConstructor, "prototype", proto);

  jerry_release_value(proto);
  jerry_release_value(tlsConstructor);
  return tls;
}
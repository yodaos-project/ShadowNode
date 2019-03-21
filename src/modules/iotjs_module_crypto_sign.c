#include <string.h>
#include "iotjs_def.h"
#include "iotjs_module_buffer.h"
#include "iotjs_objectwrap.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"
#include "mbedtls/platform.h"

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  mbedtls_pk_context pk;
  mbedtls_md_type_t hash_type;
  unsigned char buf;
  unsigned char hash;
} IOTJS_VALIDATED_STRUCT(iotjs_crypto_sign_t);

static iotjs_crypto_sign_t* iotjs_crypto_sign_create(const jerry_value_t jval);
static void iotjs_crypto_sign_destroy(iotjs_crypto_sign_t* wrap);

static JNativeInfoType this_module_native_info = {
  .free_cb = (jerry_object_native_free_callback_t)iotjs_crypto_sign_destroy
};

static iotjs_crypto_sign_t* iotjs_crypto_sign_create(const jerry_value_t jval) {
  iotjs_crypto_sign_t* sign = IOTJS_ALLOC(iotjs_crypto_sign_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_crypto_sign_t, sign);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jval,
                               &this_module_native_info);
  return sign;
}

static void iotjs_crypto_sign_destroy(iotjs_crypto_sign_t* hashwrap) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_crypto_sign_t, hashwrap);
  iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
  mbedtls_pk_free(&_this->pk);
  IOTJS_RELEASE(hashwrap);
}

JS_FUNCTION(SignConstructor) {
  DJS_CHECK_THIS();

  const jerry_value_t val = JS_GET_THIS();
  iotjs_crypto_sign_t* sign = iotjs_crypto_sign_create(val);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_crypto_sign_t, sign);

  mbedtls_pk_init(&_this->pk);
  size_t type = jerry_get_number_value(jargv[0]);
  _this->hash_type = jerry_get_number_value(type);

  return jerry_create_undefined();
}

JS_FUNCTION(SignUpdate) {
  JS_DECLARE_THIS_PTR(crypto_sign, sign);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_crypto_sign_t, sign);

  jerry_value_t jval = jargv[0];
  iotjs_bufferwrap_t* buf_wrap = iotjs_bufferwrap_from_jbuffer(jval);
  char* buf = iotjs_bufferwrap_buffer(buf_wrap);
  unsigned char o_hash;
  mbedtls_md(
    mbedtls_md_info_from_type(_this->hash_type),
    (const unsigned char *) buf,
    sizeof(buf),
    &o_hash);
  _this->hash = o_hash;
  return jerry_create_null();
}

JS_FUNCTION(SignSign) {
  JS_DECLARE_THIS_PTR(crypto_sign, sign);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_crypto_sign_t, sign);

  jerry_value_t jval = jargv[0];
  iotjs_bufferwrap_t* pri_key_wrap = iotjs_bufferwrap_from_jbuffer(jval);
  size_t pri_key_size = iotjs_bufferwrap_length(pri_key_wrap);
  char* pri_key = iotjs_bufferwrap_buffer(pri_key_wrap);

  size_t size = MBEDTLS_MD_MAX_SIZE;
  unsigned char contents[size];

  mbedtls_pk_parse_key(&_this->pk, 
    (const unsigned char*)pri_key, 
    pri_key_size, NULL, 0);
  mbedtls_pk_sign(
    &_this->pk,
    _this->hash_type,
    &_this->hash, 0,
    (unsigned char*)contents, &size,
    NULL,
    NULL);

  jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer(size);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  iotjs_bufferwrap_copy(buffer_wrap, (char*)contents, size);
  return jbuffer;
}

jerry_value_t InitCryptoSign() {
  jerry_value_t exports = jerry_create_object();
  jerry_value_t signConstructor = 
      jerry_create_external_function(SignConstructor);
  iotjs_jval_set_property_jval(exports, "Sign", signConstructor);

  jerry_value_t proto = jerry_create_object();
  iotjs_jval_set_method(proto, "update", SignUpdate);
  iotjs_jval_set_method(proto, "sign", SignSign);
  iotjs_jval_set_property_jval(signConstructor, "prototype", proto);

  jerry_release_value(proto);
  jerry_release_value(signConstructor);
  return exports;
}

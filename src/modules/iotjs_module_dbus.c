#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include <dbus/dbus.h>
#include <stdlib.h>

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  jerry_value_t jcallback;
  jerry_value_t signal_handler;
  DBusConnection* connection;
  bool initialized;
  uv_async_t connection_handle;
  uv_poll_t watcher;

} IOTJS_VALIDATED_STRUCT(iotjs_dbus_t);

typedef struct iotjs_dbus_method_data_s {
  jerry_value_t jcallback;
  DBusPendingCall* pending;
} iotjs_dbus_method_data_t;

static iotjs_dbus_t* iotjs_dbus_create(const jerry_value_t jdbus);
// static void iotjs_dbus_destroy(iotjs_dbus_t* dbus);

static JNativeInfoType this_module_native_info = {
  .free_cb = NULL
};

iotjs_dbus_t* iotjs_dbus_create(const jerry_value_t jdbus) {
  iotjs_dbus_t* dbus = IOTJS_ALLOC(iotjs_dbus_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_dbus_t, dbus);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jdbus,
                               &this_module_native_info);
  return dbus;
}

// void iotjs_dbus_destroy(iotjs_dbus_t* dbus) {
//   IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_dbus_t, dbus);
//   iotjs_jobjectwrap_destroy(&_this->jobjectwrap);
//   IOTJS_RELEASE(dbus);
// }

static void iotjs_dbus_watcher_handle(uv_poll_t* watcher, int status,
                                      int events) {
  DBusWatch* watch = (DBusWatch*)(watcher->data);
  unsigned int flags = 0;
  if (events & UV_READABLE)
    flags |= DBUS_WATCH_READABLE;
  if (events & UV_WRITABLE)
    flags |= DBUS_WATCH_WRITABLE;
  dbus_watch_handle(watch, flags);
}

static void iotjs_dbus_watcher_close(void* data) {
  uv_poll_t* watcher = (uv_poll_t*)data;
  if (watcher == NULL || uv_is_closing((uv_handle_t*)watcher))
    return;

  watcher->data = NULL;
  // Stop watching
  uv_ref((uv_handle_t*)watcher);
  uv_poll_stop(watcher);
  uv_close((uv_handle_t*)watcher, NULL);
}

/**
 * Watcher Functions
 */
static dbus_bool_t iotjs_dbus_watch_add(DBusWatch* watch, void* data) {
  if (!dbus_watch_get_enabled(watch) || dbus_watch_get_data(watch) != NULL) {
    return true;
  }

  int events = 0;
  int fd = dbus_watch_get_unix_fd(watch);
  unsigned int flags = dbus_watch_get_flags(watch);
  if (flags & DBUS_WATCH_READABLE)
    events |= UV_READABLE;
  if (flags & DBUS_WATCH_WRITABLE)
    events |= UV_WRITABLE;

  iotjs_dbus_t* dbus = (iotjs_dbus_t*)data;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  // Initializing watcher
  _this->watcher.data = (void*)watch;
  uv_poll_init(uv_default_loop(), &_this->watcher, fd);
  uv_poll_start(&_this->watcher, events, iotjs_dbus_watcher_handle);
  dbus_watch_set_data(watch, (void*)&_this->watcher, iotjs_dbus_watcher_close);
  return true;
}

static void iotjs_dbus_watch_remove(DBusWatch* watch, void* data) {
  uv_poll_t* watcher = (uv_poll_t*)(dbus_watch_get_data(watch));
  if (watcher == NULL)
    return;
  dbus_watch_set_data(watch, NULL, NULL);
}

static void iotjs_dbus_watch_handle(DBusWatch* watch, void* data) {
  if (dbus_watch_get_data(watch) == NULL) {
    return;
  }
  if (dbus_watch_get_enabled(watch)) {
    iotjs_dbus_watch_add(watch, data);
  } else {
    iotjs_dbus_watch_remove(watch, data);
  }
}

/**
 * Timeout Functions
 */
static dbus_bool_t iotjs_dbus_timeout_add(DBusTimeout* timeout, void* data) {
  if (!dbus_timeout_get_enabled(timeout) ||
      dbus_timeout_get_data(timeout) != NULL) {
    return true;
  }
  dbus_timeout_set_data(timeout, NULL, NULL);
  return true;
}

static void iotjs_dbus_timeout_remove(DBusTimeout* timeout, void* data) {
  dbus_timeout_set_data(timeout, NULL, NULL);
}

static void iotjs_dbus_timeout_toggled(DBusTimeout* timeout, void* data) {
  // TODO
}

/**
 * Wakeup Callbacks
 */
static void iotjs_dbus_connection_wakeup(void* data) {
  uv_async_t* connection_handle = (uv_async_t*)data;
  uv_async_send(connection_handle);
}

static void iotjs_dbus_connection_cb(uv_async_t* handle) {
  iotjs_dbus_t* dbus = (iotjs_dbus_t*)handle->data;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  dbus_connection_read_write(_this->connection, 0);
  while (dbus_connection_dispatch(_this->connection) ==
         DBUS_DISPATCH_DATA_REMAINS)
    ;
}

static void iotjs_dbus_connection_close_cb(void* data) {
  fprintf(stdout, "the dbus connection is closed\n");
}

/**
 * Message Callbacks
 */
static bool iotjs_dbus_encode_jobject(jerry_value_t val, DBusMessageIter* iter,
                                      const char* signature) {
  DBusSignatureIter signatureIter;
  int type;

  dbus_signature_iter_init(&signatureIter, signature);
  type = dbus_signature_iter_get_current_type(&signatureIter);

  switch (type) {
    case DBUS_TYPE_INVALID:
      break;
    case DBUS_TYPE_BOOLEAN: {
      dbus_bool_t data = jerry_get_boolean_value(val);
      if (!dbus_message_iter_append_basic(iter, type, &data))
        return false;
    } break;
    case DBUS_TYPE_INT16: {
      dbus_int16_t data = jerry_get_number_value(val);
      if (!dbus_message_iter_append_basic(iter, type, &data))
        return false;
    } break;
    case DBUS_TYPE_UINT16: {
      dbus_uint16_t data = jerry_get_number_value(val);
      if (!dbus_message_iter_append_basic(iter, type, &data))
        return false;
    } break;
    case DBUS_TYPE_STRING:
    case DBUS_TYPE_OBJECT_PATH:
    case DBUS_TYPE_SIGNATURE: {
      jerry_size_t size = jerry_get_string_size(val);
      jerry_char_t buffer[size + 1];
      jerry_string_to_utf8_char_buffer(val, buffer, size);

      char* data = (char*)malloc(size + 1);
      memset(data, 0, size + 1);
      strncpy(data, (char*)buffer, size);

      dbus_bool_t r = dbus_message_iter_append_basic(iter, type, &data);
      dbus_free(data);
      if (!r)
        return false;
    } break;
  }
  return true;
}

static jerry_value_t iotjs_dbus_decode_message(DBusMessage* message) {
  DBusMessageIter iter;
  dbus_message_iter_init(message, &iter);

  if (dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_ERROR)
    return jerry_create_null();

  char* signature = NULL;
  uint32_t index = 0;
  jerry_value_t results = jerry_create_object();

  do {
    // DBusSignatureIter signatureIter;
    signature = dbus_message_iter_get_signature(&iter);
    int type = dbus_message_iter_get_arg_type(&iter);
    switch (type) {
      case DBUS_TYPE_BOOLEAN: {
        dbus_bool_t value = false;
        dbus_message_iter_get_basic(&iter, &value);
        jerry_set_property_by_index(results, index,
                                    jerry_create_boolean(value));
      } break;
      case DBUS_TYPE_BYTE: {
        unsigned char value = 0;
        dbus_message_iter_get_basic(&iter, &value);
        jerry_set_property_by_index(results, index, jerry_create_number(value));
      } break;
#define IOTJS_DBUS_DECODE_NUMBER(type)                                       \
  {                                                                          \
    type value = 0;                                                          \
    dbus_message_iter_get_basic(&iter, &value);                              \
    jerry_set_property_by_index(results, index, jerry_create_number(value)); \
  }
      case DBUS_TYPE_INT16:
        IOTJS_DBUS_DECODE_NUMBER(dbus_int16_t);
        break;
      case DBUS_TYPE_UINT16:
        IOTJS_DBUS_DECODE_NUMBER(dbus_uint16_t);
        break;
      case DBUS_TYPE_INT32:
        IOTJS_DBUS_DECODE_NUMBER(dbus_int32_t);
        break;
      case DBUS_TYPE_UINT32:
        IOTJS_DBUS_DECODE_NUMBER(dbus_uint32_t);
        break;
      case DBUS_TYPE_INT64:
        IOTJS_DBUS_DECODE_NUMBER(dbus_int64_t);
        break;
      case DBUS_TYPE_UINT64:
        IOTJS_DBUS_DECODE_NUMBER(dbus_uint64_t);
        break;
      case DBUS_TYPE_DOUBLE:
        IOTJS_DBUS_DECODE_NUMBER(double);
        break;
#undef IOTJS_DBUS_DECODE_NUMBER

      case DBUS_TYPE_OBJECT_PATH:
      case DBUS_TYPE_SIGNATURE:
      case DBUS_TYPE_STRING: {
        const char* value;
        dbus_message_iter_get_basic(&iter, &value);
        jerry_set_property_by_index(results, index,
                                    jerry_create_string((jerry_char_t*)value));
      } break;
    }

    dbus_free(signature);
    index += 1;
  } while (dbus_message_iter_next(&iter));
  return results;
}

static DBusHandlerResult iotjs_dbus_handle_message(DBusConnection* conn,
                                                   DBusMessage* msg,
                                                   void* data) {
  iotjs_dbus_t* dbus = (iotjs_dbus_t*)data;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  const jerry_char_t* sender = (jerry_char_t*)dbus_message_get_sender(msg);
  const jerry_char_t* object_path = (jerry_char_t*)dbus_message_get_path(msg);
  const jerry_char_t* interface =
      (jerry_char_t*)dbus_message_get_interface(msg);
  const jerry_char_t* member = (jerry_char_t*)dbus_message_get_member(msg);

  jerry_value_t jmsg = iotjs_dbus_decode_message(msg);
  jerry_set_object_native_pointer(jmsg, msg, &this_module_native_info);

  iotjs_jargs_t jargs = iotjs_jargs_create(5);
  jerry_value_t jsender = jerry_create_string(sender);
  jerry_value_t jobject_path = jerry_create_string(object_path);
  jerry_value_t jinterface = jerry_create_string(interface);
  jerry_value_t jmember = jerry_create_string(member);
  iotjs_jargs_append_jval(&jargs, jsender);
  iotjs_jargs_append_jval(&jargs, jobject_path);
  iotjs_jargs_append_jval(&jargs, jinterface);
  iotjs_jargs_append_jval(&jargs, jmember);
  iotjs_jargs_append_jval(&jargs, jmsg);
  iotjs_make_callback(_this->jcallback, jerry_create_undefined(), &jargs);
  jerry_release_value(jsender);
  jerry_release_value(jobject_path);
  jerry_release_value(jinterface);
  jerry_release_value(jmember);
  jerry_release_value(jmsg);
  iotjs_jargs_destroy(&jargs);

  return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult iotjs_dbus_signal_filter(DBusConnection* connection,
                                                  DBusMessage* msg,
                                                  void* data) {
  if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL) {
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }
  iotjs_dbus_t* dbus = (iotjs_dbus_t*)data;
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  const jerry_char_t* sender = (jerry_char_t*)dbus_message_get_sender(msg);
  const jerry_char_t* object_path = (jerry_char_t*)dbus_message_get_path(msg);
  const jerry_char_t* interface =
      (jerry_char_t*)dbus_message_get_interface(msg);
  const jerry_char_t* signal = (jerry_char_t*)dbus_message_get_member(msg);

  jerry_value_t jmsg = iotjs_dbus_decode_message(msg);
  jerry_set_object_native_pointer(jmsg, msg, &this_module_native_info);

  iotjs_jargs_t jargs = iotjs_jargs_create(5);
  if (sender) {
    jerry_value_t jsender = jerry_create_string(sender);
    iotjs_jargs_append_jval(&jargs, jsender);
    jerry_release_value(jsender);
  } else {
    iotjs_jargs_append_null(&jargs);
  }
  jerry_value_t jobject_path = jerry_create_string(object_path);
  jerry_value_t jinterface = jerry_create_string(interface);
  jerry_value_t jsignal = jerry_create_string(signal);
  iotjs_jargs_append_jval(&jargs, jobject_path);
  iotjs_jargs_append_jval(&jargs, jinterface);
  iotjs_jargs_append_jval(&jargs, jsignal);
  iotjs_jargs_append_jval(&jargs, jmsg);
  iotjs_make_callback(_this->signal_handler, jerry_create_undefined(), &jargs);
  jerry_release_value(jobject_path);
  jerry_release_value(jinterface);
  jerry_release_value(jsignal);
  jerry_release_value(jmsg);
  iotjs_jargs_destroy(&jargs);
  return DBUS_HANDLER_RESULT_HANDLED;
}

static void iotjs_dbus_unregister_message_handler(DBusConnection* conn,
                                                  void* data) {
  fprintf(stdout, "unregister handler\n");
}

static void iotjs_dbus_call_method(DBusPendingCall* pending, void* data) {
  iotjs_dbus_method_data_t* method_data = (iotjs_dbus_method_data_t*)data;
  DBusError error;
  DBusMessage* msg;

  dbus_error_init(&error);
  msg = dbus_pending_call_steal_reply(pending);
  dbus_pending_call_unref(pending);
  if (!msg)
    return;

  iotjs_jargs_t jargs = iotjs_jargs_create(1);
  jerry_value_t jmsg = iotjs_dbus_decode_message(msg);
  iotjs_jargs_append_jval(&jargs, jmsg);
  iotjs_make_callback(method_data->jcallback, jerry_create_undefined(), &jargs);
  jerry_release_value(jmsg);
  iotjs_jargs_destroy(&jargs);
  dbus_message_unref(msg);
}

static void iotjs_dbus_after_call_method(void* data) {
  free(data);
}

JS_FUNCTION(DbusConstructor) {
  DJS_CHECK_THIS();

  // Create DBUS object
  const jerry_value_t jdbus = JS_GET_THIS();
  iotjs_dbus_t* dbus = iotjs_dbus_create(jdbus);
  // IOTJS_ASSERT(dbus == iotjs_dbus_instance_from_jval(jdbus));
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  _this->initialized = true;
  _this->connection_handle.data = _this;
  uv_async_init(uv_default_loop(), &_this->connection_handle,
                iotjs_dbus_connection_cb);

  return jerry_create_undefined();
}

JS_FUNCTION(GetBus) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  DBusError error;
  dbus_error_init(&error);

  DJS_CHECK_ARGS(1, number);
  int type = JS_GET_ARG(0, number);
  if (type == 0 /* BUS_SYSTEM */) {
    _this->connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
  } else if (type == 1 /* BUS_SESSION */) {
    _this->connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
  }

  if (_this->connection == NULL) {
    if (dbus_error_is_set(&error)) {
      return JS_CREATE_ERROR(COMMON, error.message);
    } else {
      return JS_CREATE_ERROR(COMMON, "failed to get dbus object");
    }
  }

  dbus_connection_set_exit_on_disconnect(_this->connection, false);
  dbus_connection_set_watch_functions(_this->connection, iotjs_dbus_watch_add,
                                      iotjs_dbus_watch_remove,
                                      iotjs_dbus_watch_handle, (void*)dbus, NULL);
  dbus_connection_set_timeout_functions(_this->connection,
                                        iotjs_dbus_timeout_add,
                                        iotjs_dbus_timeout_remove,
                                        iotjs_dbus_timeout_toggled, (void*)dbus, NULL);
  dbus_connection_set_wakeup_main_function(_this->connection,
                                           iotjs_dbus_connection_wakeup,
                                           &_this->connection_handle,
                                           iotjs_dbus_connection_close_cb);
  dbus_connection_add_filter(_this->connection, iotjs_dbus_signal_filter,
                             (void*)dbus, NULL);
  return jerry_create_null();
}

JS_FUNCTION(ReleaseBus) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);
  dbus_connection_unref(_this->connection);
  jerry_release_value(_this->signal_handler);
  jerry_release_value(_this->jcallback);

  iotjs_dbus_watcher_close((void*)&_this->watcher);
  uv_close((uv_handle_t*)&_this->connection_handle, NULL);
  return jerry_create_undefined();
}

JS_FUNCTION(CallMethod) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  DBusError error;
  dbus_error_init(&error);

  iotjs_string_t service_name = JS_GET_ARG(0, string);
  iotjs_string_t object_path = JS_GET_ARG(1, string);
  iotjs_string_t iface = JS_GET_ARG(2, string);
  iotjs_string_t method = JS_GET_ARG(3, string);
  iotjs_string_t signature = JS_GET_ARG(4, string);
  jerry_value_t args = JS_GET_ARG(5, object);
  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(6, function);

  DBusMessage* msg =
      dbus_message_new_method_call(iotjs_string_data(&service_name),
                                   iotjs_string_data(&object_path),
                                   iotjs_string_data(&iface),
                                   iotjs_string_data(&method));
  iotjs_string_destroy(&service_name);
  iotjs_string_destroy(&object_path);
  iotjs_string_destroy(&iface);
  iotjs_string_destroy(&method);

  jerry_size_t length = jerry_get_array_length(args);
  if (length > 0) {
    DBusMessageIter iter;
    DBusSignatureIter signatureIter;
    dbus_message_iter_init_append(msg, &iter);
    dbus_signature_iter_init(&signatureIter, iotjs_string_data(&signature));

    for (uint32_t i = 0; i < length; i++) {
      char* arg_sig = dbus_signature_iter_get_signature(&signatureIter);
      jerry_value_t val = jerry_get_property_by_index(args, i);
      iotjs_dbus_encode_jobject(val, &iter, arg_sig);
      jerry_release_value(val);
      dbus_free(arg_sig);

      if (!dbus_signature_iter_next(&signatureIter))
        break;
    }
  }
  iotjs_string_destroy(&signature);

  DBusPendingCall* pending;
  if (!dbus_connection_send_with_reply(_this->connection, msg, &pending,
                                       5 * 1000) ||
      !pending) {
    if (msg != NULL)
      dbus_message_unref(msg);
    return JS_CREATE_ERROR(COMMON, "failed to call method: Out of Memory");
  }

  iotjs_dbus_method_data_t* method_data =
      malloc(sizeof(iotjs_dbus_method_data_t));
  // TODO(Yorkie): check if malloc is succeed.
  method_data->jcallback = jerry_acquire_value(jcallback);
  method_data->pending = pending;

  if (!dbus_pending_call_set_notify(pending, iotjs_dbus_call_method,
                                    method_data,
                                    iotjs_dbus_after_call_method)) {
    if (msg != NULL)
      dbus_message_unref(msg);
    return JS_CREATE_ERROR(COMMON, "failed to call method: Out of Memory");
  }

  if (msg != NULL) {
    dbus_message_unref(msg);
  }
  dbus_connection_flush(_this->connection);
  return jerry_create_undefined();
}

JS_FUNCTION(RequestName) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);
  iotjs_string_t name = JS_GET_ARG(0, string);

  dbus_bus_request_name(_this->connection, iotjs_string_data(&name), 0, NULL);
  iotjs_string_destroy(&name);
  dbus_connection_flush(_this->connection);
  return jerry_create_null();
}

JS_FUNCTION(RegisterObjectPath) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);
  iotjs_string_t path = JS_GET_ARG(0, string);

  DBusObjectPathVTable vt;
  vt.message_function = iotjs_dbus_handle_message;
  vt.unregister_function = iotjs_dbus_unregister_message_handler;

  dbus_bool_t r = dbus_connection_register_object_path(_this->connection,
                                                       iotjs_string_data(&path),
                                                       &vt, (void*)dbus);
  iotjs_string_destroy(&path);
  dbus_connection_flush(_this->connection);
  if (!r) {
    return JS_CREATE_ERROR(COMMON, "Out of memory");
  }
  return jerry_create_null();
}

JS_FUNCTION(UnregisterObjectPath) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);
  iotjs_string_t path = JS_GET_ARG(0, string);

  dbus_bool_t r =
      dbus_connection_unregister_object_path(_this->connection,
                                             iotjs_string_data(&path));
  iotjs_string_destroy(&path);
  dbus_connection_flush(_this->connection);
  if (!r)
    return JS_CREATE_ERROR(COMMON, "Out of memory");
  return jerry_create_undefined();
}

JS_FUNCTION(SendMessageReply) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  iotjs_string_t signature = JS_GET_ARG(2, string);
  DBusMessageIter iter;
  DBusMessage* reply;
  dbus_uint32_t serial = 0;

  void* data;
  const jerry_object_native_info_t* type;
  jerry_get_object_native_pointer(jargv[0], &data, &type);

  reply = dbus_message_new_method_return((DBusMessage*)data);

  dbus_message_iter_init_append(reply, &iter);
  iotjs_dbus_encode_jobject(jargv[1], &iter, iotjs_string_data(&signature));
  iotjs_string_destroy(&signature);
  dbus_connection_send(_this->connection, reply, &serial);
  dbus_connection_flush(_this->connection);
  dbus_message_unref(reply);
  return jerry_create_null();
}

JS_FUNCTION(SetMessageHandler) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  jerry_value_t jcallback = JS_GET_ARG_IF_EXIST(0, function);
  _this->jcallback = jerry_acquire_value(jcallback);
  return jerry_create_null();
}

JS_FUNCTION(SetSignalHandler) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  jerry_value_t handler = JS_GET_ARG_IF_EXIST(0, function);
  _this->signal_handler = jerry_acquire_value(handler);
  return jerry_create_null();
}

JS_FUNCTION(AddSignalFilter) {
  DBusError err;
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  iotjs_string_t rule = JS_GET_ARG(0, string);
  const char* rule_str = iotjs_string_data(&rule);

  dbus_error_init(&err);
  dbus_bus_add_match(_this->connection, rule_str, &err);
  dbus_connection_flush(_this->connection);
  iotjs_string_destroy(&rule);

  if (dbus_error_is_set(&err)) {
    return JS_CREATE_ERROR(COMMON, "failed to add rule");
  }
  return jerry_create_null();
}

JS_FUNCTION(EmitSignal) {
  JS_DECLARE_THIS_PTR(dbus, dbus);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_dbus_t, dbus);

  DBusMessage* msg;
  dbus_uint32_t serial = 0;

  iotjs_string_t object_path = JS_GET_ARG(0, string);
  iotjs_string_t iface = JS_GET_ARG(1, string);
  iotjs_string_t signal = JS_GET_ARG(2, string);
  iotjs_string_t signature = JS_GET_ARG(3, string);
  jerry_value_t args = JS_GET_ARG(4, object);
  msg = dbus_message_new_signal(iotjs_string_data(&object_path),
                                iotjs_string_data(&iface),
                                iotjs_string_data(&signal));
  iotjs_string_destroy(&object_path);
  iotjs_string_destroy(&iface);
  iotjs_string_destroy(&signal);

  jerry_size_t length = jerry_get_array_length(args);
  if (length > 0) {
    DBusMessageIter iter;
    DBusSignatureIter signatureIter;
    dbus_message_iter_init_append(msg, &iter);
    dbus_signature_iter_init(&signatureIter, iotjs_string_data(&signature));

    for (uint32_t i = 0; i < length; i++) {
      char* arg_sig = dbus_signature_iter_get_signature(&signatureIter);
      jerry_value_t val = jerry_get_property_by_index(args, i);
      iotjs_dbus_encode_jobject(val, &iter, arg_sig);
      dbus_free(arg_sig);
      jerry_release_value(val);

      if (!dbus_signature_iter_next(&signatureIter))
        break;
    }
  }
  iotjs_string_destroy(&signature);

  dbus_connection_send(_this->connection, msg, &serial);
  dbus_connection_flush(_this->connection);
  dbus_message_unref(msg);
  return jerry_create_null();
}

jerry_value_t InitDBus() {
  jerry_value_t jdbus = jerry_create_object();
  jerry_value_t jdbusConstructor =
      jerry_create_external_function(DbusConstructor);
  iotjs_jval_set_property_jval(jdbus, "DBus", jdbusConstructor);

  jerry_value_t proto = jerry_create_object();
  iotjs_jval_set_method(proto, "getBus", GetBus);
  iotjs_jval_set_method(proto, "releaseBus", ReleaseBus);
  iotjs_jval_set_method(proto, "requestName", RequestName);
  iotjs_jval_set_method(proto, "callMethod", CallMethod);
  iotjs_jval_set_method(proto, "registerObjectPath", RegisterObjectPath);
  iotjs_jval_set_method(proto, "unregisterObjectPath", UnregisterObjectPath);
  iotjs_jval_set_method(proto, "sendMessageReply", SendMessageReply);
  iotjs_jval_set_method(proto, "setMessageHandler", SetMessageHandler);
  iotjs_jval_set_method(proto, "setSignalHandler", SetSignalHandler);
  iotjs_jval_set_method(proto, "addSignalFilter", AddSignalFilter);
  iotjs_jval_set_method(proto, "emitSignal", EmitSignal);
  iotjs_jval_set_property_jval(jdbusConstructor, "prototype", proto);

  jerry_release_value(proto);
  jerry_release_value(jdbusConstructor);
  return jdbus;
}

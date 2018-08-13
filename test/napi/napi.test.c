#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>

napi_value SayHello(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 0;
  // test if `napi_get_cb_info` tolerants NULL pointers.
  status = napi_get_cb_info(env, info, &argc, NULL, NULL, NULL);

  napi_value str;
  status = napi_create_string_utf8(env, "Hello", 5, &str);
  if (status != napi_ok)
    return NULL;

  return str;
}

napi_value SayError(napi_env env, napi_callback_info info) {
  napi_status status;

  status = napi_throw_error(env, "foo", "bar");
  if (status != napi_ok)
    return NULL;

  return NULL;
}

napi_value StrictEquals(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 2;
  napi_value argv[2];
  napi_value thisArg;
  void* data;
  status = napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);
  if (status != napi_ok)
    return NULL;

  bool result = false;
  status = napi_strict_equals(env, argv[0], argv[1], &result);
  if (status != napi_ok)
    return NULL;

  napi_value ret;
  status = napi_get_boolean(env, result, &ret);
  if (status != napi_ok)
    return NULL;

  return ret;
}

napi_value Instanceof(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 2;
  napi_value argv[2];
  napi_value thisArg;
  void* data;
  status = napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);
  if (status != napi_ok)
    return NULL;

  bool result = false;
  status = napi_instanceof(env, argv[0], argv[1], &result);
  if (status != napi_ok)
    return NULL;

  napi_value ret;
  status = napi_get_boolean(env, result, &ret);
  if (status != napi_ok)
    return NULL;

  return ret;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;

#define set_named_method(env, target, prop_name, handler)            \
  do {                                                               \
    napi_value fn;                                                   \
    status = napi_create_function(env, NULL, 0, handler, NULL, &fn); \
    if (status != napi_ok)                                           \
      return NULL;                                                   \
                                                                     \
    status = napi_set_named_property(env, target, prop_name, fn);    \
    if (status != napi_ok)                                           \
      return NULL;                                                   \
  } while (0);

  set_named_method(env, exports, "sayHello", SayHello);
  set_named_method(env, exports, "sayError", SayError);
  set_named_method(env, exports, "strictEquals", StrictEquals);
  set_named_method(env, exports, "instanceof", Instanceof);

  napi_value id;
  status = napi_create_int32(env, 321, &id);
  if (status != napi_ok)
    return NULL;

  status = napi_set_named_property(env, exports, "id", id);
  if (status != napi_ok)
    return NULL;

  return exports;
}

NAPI_MODULE(napi_test, Init);

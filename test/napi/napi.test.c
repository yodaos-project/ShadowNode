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

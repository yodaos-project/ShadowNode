#include <node_api.h>
#include <stdlib.h>
#include <stdio.h>

napi_value SayHello(napi_env env, napi_callback_info info) {
  napi_status status;

  napi_value str;
  status = napi_create_string_utf8(env, "Hello", 5, &str);
  if (status != napi_ok)
    return NULL;

  return str;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;

  napi_value fn;
  status = napi_create_function(env, NULL, 0, SayHello, NULL, &fn);
  if (status != napi_ok)
    return NULL;

  status = napi_set_named_property(env, exports, "sayHello", fn);
  if (status != napi_ok)
    return NULL;

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

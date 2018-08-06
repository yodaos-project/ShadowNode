#include <node_api.h>
#include <stdlib.h>

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;

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

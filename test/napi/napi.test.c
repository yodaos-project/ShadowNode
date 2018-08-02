#include <node_api.h>
#include <stdlib.h>

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;

  napi_value ret;
  status = napi_create_int32(env, 321, &ret);
  if (status != napi_ok) {
    exit(1);
  }
  return ret;
}

NAPI_MODULE(napi_test, Init);

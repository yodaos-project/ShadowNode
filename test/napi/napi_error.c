#include <node_api.h>
#include "common.h"

static napi_value ThrowError(napi_env env, napi_callback_info info) {
  napi_status status;

  NAPI_CALL(env, napi_throw_error(env, NULL, "foobar"));

  return NULL;
}

static napi_value RethrowError(napi_env env, napi_callback_info info) {
  napi_status status;

  NAPI_CALL(env, napi_throw_error(env, NULL, "foobar"));

  napi_value err;
  NAPI_CALL(env, napi_get_and_clear_last_exception(env, &err));

  NAPI_CALL(env, napi_throw(env, err));

  return NULL;
}

static napi_value Init(napi_env env, napi_value exports) {
  SET_NAMED_METHOD(env, exports, "ThrowError", ThrowError);
  SET_NAMED_METHOD(env, exports, "RethrowError", RethrowError);

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)

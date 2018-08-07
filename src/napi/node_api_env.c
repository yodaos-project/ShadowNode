/* Copyright 2018-present Rokid Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "internal/node_api_internal.h"

static iotjs_napi_env_t current_env = {
  .pending_exception = NULL,
  .pending_fatal_exception = NULL,
};

inline napi_env iotjs_get_current_napi_env() {
  return (napi_env)&current_env;
}

bool iotjs_napi_is_exception_pending(iotjs_napi_env_t *env) {
  return !(env->pending_exception == NULL &&
           env->pending_fatal_exception == NULL);
}

napi_status iotjs_napi_env_set_exception(napi_env env, napi_value error) {
  if (env != iotjs_get_current_napi_env())
    return napi_invalid_arg;
  iotjs_napi_env_t *cur_env = (iotjs_napi_env_t *)env;

  cur_env->pending_exception = error;
  return napi_ok;
}

napi_status iotjs_napi_env_set_fatal_exception(napi_env env, napi_value error) {
  if (env != iotjs_get_current_napi_env())
    return napi_invalid_arg;
  iotjs_napi_env_t *cur_env = (iotjs_napi_env_t *)env;

  cur_env->pending_exception = error;
  return napi_ok;
}

napi_status napi_is_exception_pending(napi_env env, bool *result) {
  if (env != iotjs_get_current_napi_env())
    return napi_invalid_arg;
  *result = iotjs_napi_is_exception_pending((iotjs_napi_env_t *)env);
  return napi_ok;
}

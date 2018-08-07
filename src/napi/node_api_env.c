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

#include "iotjs_def.h"
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

// Methods to support error handling
napi_status napi_throw(napi_env env, napi_value error) {
  if (env != iotjs_get_current_napi_env())
    return napi_invalid_arg;
  iotjs_napi_env_t *curr_env = (iotjs_napi_env_t *)env;
  if (iotjs_napi_is_exception_pending(curr_env))
    return napi_pending_exception;
  curr_env->pending_exception = error;
  return napi_ok;
}

#define NAPI_THROWS(type, jerry_error_type)                        \
  napi_status napi_throw_##type(napi_env env, const char *code,    \
                                const char *msg) {                 \
    if (env != iotjs_get_current_napi_env())                       \
      return napi_invalid_arg;                                     \
    if (iotjs_napi_is_exception_pending((iotjs_napi_env_t *)env))  \
      return napi_pending_exception;                               \
                                                                   \
    jerry_value_t jval_error =                                     \
        jerry_create_error(jerry_error_type, (jerry_char_t *)msg); \
    iotjs_jval_set_property_string_raw(jval_error, "code", code);  \
    return napi_throw(env, AS_NAPI_VALUE(jval_error));             \
  }

NAPI_THROWS(error, JERRY_ERROR_COMMON);
NAPI_THROWS(type_error, JERRY_ERROR_TYPE);
NAPI_THROWS(range_error, JERRY_ERROR_RANGE);
#undef NAPI_THROWS

napi_status napi_fatal_exception(napi_env env, napi_value err) {
  if (env != iotjs_get_current_napi_env())
    return napi_invalid_arg;
  iotjs_napi_env_t *curr_env = (iotjs_napi_env_t *)env;
  if (iotjs_napi_is_exception_pending(curr_env))
    return napi_pending_exception;
  curr_env->pending_fatal_exception = err;
  return napi_ok;
}


// Methods to support catching exceptions
napi_status napi_is_exception_pending(napi_env env, bool *result) {
  if (env != iotjs_get_current_napi_env())
    return napi_invalid_arg;
  *result = iotjs_napi_is_exception_pending((iotjs_napi_env_t *)env);
  return napi_ok;
}

napi_status napi_get_and_clear_last_exception(napi_env env,
                                              napi_value *result) {
  if (env != iotjs_get_current_napi_env())
    return napi_invalid_arg;
  iotjs_napi_env_t *curr_env = (iotjs_napi_env_t *)env;

  if (curr_env->pending_exception != NULL) {
    *result = curr_env->pending_exception;
    curr_env->pending_exception = NULL;
  } else if (curr_env->pending_fatal_exception != NULL) {
    *result = curr_env->pending_fatal_exception;
    curr_env->pending_fatal_exception = NULL;
  }
  return napi_ok;
}


napi_status napi_get_last_error_info(napi_env env,
                                     const napi_extended_error_info **result) {
  if (env != iotjs_get_current_napi_env())
    return napi_invalid_arg;
  iotjs_napi_env_t *curr_env = (iotjs_napi_env_t *)env;
  napi_extended_error_info *error_info = &curr_env->extended_error_info;

  *result = error_info;
  return napi_ok;
}

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

static const char* NAPI_GENERIC_ERROR_MESSAGE = "Unexpected error.";

static iotjs_napi_env_t current_env = {
  .pending_exception = NULL,
  .pending_fatal_exception = NULL,
};

inline napi_env iotjs_get_current_napi_env() {
  return (napi_env)&current_env;
}

inline bool iotjs_napi_is_exception_pending(iotjs_napi_env_t* env) {
  return !(env->pending_exception == NULL &&
           env->pending_fatal_exception == NULL);
}

void iotjs_napi_set_error_info(napi_env env, napi_status error_code,
                               const char* error_message,
                               uint32_t engine_error_code,
                               void* engine_reserved) {
  iotjs_napi_env_t* cur_env = (iotjs_napi_env_t*)env;

  if (error_message == NULL && error_code != napi_ok) {
    error_message = NAPI_GENERIC_ERROR_MESSAGE;
  }

  cur_env->extended_error_info.error_code = error_code;
  cur_env->extended_error_info.error_message = error_message;
  cur_env->extended_error_info.engine_error_code = engine_error_code;
  cur_env->extended_error_info.engine_reserved = engine_reserved;
}

void iotjs_napi_clear_error_info(napi_env env) {
  iotjs_napi_env_t* cur_env = (iotjs_napi_env_t*)env;

  cur_env->extended_error_info.error_code = napi_ok;
  cur_env->extended_error_info.error_message = NULL;
  cur_env->extended_error_info.engine_error_code = 0;
  cur_env->extended_error_info.engine_reserved = NULL;
}

jerry_value_t iotjs_napi_env_get_and_clear_exception(napi_env env) {
  iotjs_napi_env_t* cur_env = (iotjs_napi_env_t*)env;

  jerry_value_t jval_ret = AS_JERRY_VALUE(cur_env->pending_exception);
  cur_env->pending_exception = NULL;

  return jval_ret;
}

jerry_value_t iotjs_napi_env_get_and_clear_fatal_exception(napi_env env) {
  iotjs_napi_env_t* cur_env = (iotjs_napi_env_t*)env;

  jerry_value_t jval_ret = AS_JERRY_VALUE(cur_env->pending_fatal_exception);
  cur_env->pending_fatal_exception = NULL;

  return jval_ret;
}

// Methods to support error handling
napi_status napi_throw(napi_env env, napi_value error) {
  NAPI_TRY_ENV(env);
  iotjs_napi_env_t* curr_env = (iotjs_napi_env_t*)env;
  NAPI_TRY_NO_PENDING_EXCEPTION(curr_env);

  jerry_value_t jval_err = AS_JERRY_VALUE(error);
  if (!jerry_value_has_error_flag(jval_err)) {
    jerry_value_set_error_flag(&jval_err);
  }

  curr_env->pending_exception = AS_NAPI_VALUE(jval_err);
  /** should not clear last error info */
  return napi_ok;
}

#define DEF_NAPI_THROWS(type, jerry_error_type)                              \
  napi_status napi_throw_##type(napi_env env, const char* code,              \
                                const char* msg) {                           \
    NAPI_TRY_ENV(env);                                                       \
    NAPI_TRY_NO_PENDING_EXCEPTION(env);                                      \
                                                                             \
    JERRYX_CREATE(jval_error,                                                \
                  jerry_create_error(jerry_error_type, (jerry_char_t*)msg)); \
    if (code != NULL) {                                                      \
      iotjs_jval_set_property_string_raw(jval_error, "code", code);          \
    }                                                                        \
    return napi_throw(env, AS_NAPI_VALUE(jval_error));                       \
  }

DEF_NAPI_THROWS(error, JERRY_ERROR_COMMON);
DEF_NAPI_THROWS(type_error, JERRY_ERROR_TYPE);
DEF_NAPI_THROWS(range_error, JERRY_ERROR_RANGE);
#undef DEF_NAPI_THROWS

napi_status napi_fatal_exception(napi_env env, napi_value err) {
  NAPI_TRY_ENV(env);
  NAPI_TRY_NO_PENDING_EXCEPTION(env);
  iotjs_napi_env_t* curr_env = (iotjs_napi_env_t*)env;

  jerry_value_t jval_err = AS_JERRY_VALUE(err);
  if (!jerry_value_has_error_flag(jval_err)) {
    jerry_value_set_error_flag(&jval_err);
  }

  curr_env->pending_fatal_exception = AS_NAPI_VALUE(jval_err);
  /** should not clear last error info */
  return napi_ok;
}


// Methods to support catching exceptions
napi_status napi_is_exception_pending(napi_env env, bool* result) {
  NAPI_TRY_ENV(env);
  NAPI_ASSIGN(result, iotjs_napi_is_exception_pending((iotjs_napi_env_t*)env));
  /** should not clear last error info */
  return napi_ok;
}

napi_status napi_get_and_clear_last_exception(napi_env env,
                                              napi_value* result) {
  NAPI_TRY_ENV(env);
  iotjs_napi_env_t* curr_env = (iotjs_napi_env_t*)env;

  if (curr_env->pending_exception != NULL) {
    NAPI_ASSIGN(result, curr_env->pending_exception);
    curr_env->pending_exception = NULL;
  } else if (curr_env->pending_fatal_exception != NULL) {
    NAPI_ASSIGN(result, curr_env->pending_fatal_exception);
    curr_env->pending_fatal_exception = NULL;
  }
  /** should not clear last error info */
  return napi_ok;
}


napi_status napi_get_last_error_info(napi_env env,
                                     const napi_extended_error_info** result) {
  NAPI_TRY_ENV(env);
  iotjs_napi_env_t* curr_env = (iotjs_napi_env_t*)env;
  napi_extended_error_info* error_info = &curr_env->extended_error_info;

  NAPI_ASSIGN(result, error_info);
  return napi_ok;
}

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

#include "jerryscript-ext/handle-scope.h"
#include "jerryscript.h"
#include <stdlib.h>
#include "internal/node_api_internal.h"
#include "node_api.h"

static const jerry_object_native_info_t native_obj_type_info = { .free_cb =
                                                                     free };

static jerry_value_t iotjs_napi_function_handler(
    const jerry_value_t function_obj, const jerry_value_t this_val,
    const jerry_value_t args_p[], const jerry_length_t args_cnt) {
  iotjs_function_info_t* function_info;
  jerry_get_object_native_pointer(function_obj, (void*)&function_info, NULL);
  iotjs_callback_info_t* callback_info = IOTJS_ALLOC(iotjs_callback_info_t);
  callback_info->argc = args_cnt;
  callback_info->argv = (jerry_value_t*)args_p;
  callback_info->jval_this = this_val;
  callback_info->function_info = function_info;

  napi_env env = function_info->env;
  jerry_value_t jval_ret;

  jerryx_handle_scope scope;
  jerryx_open_handle_scope(&scope);

  napi_value nvalue_ret =
      function_info->cb(env, (napi_callback_info)callback_info);
  free(callback_info);

  iotjs_napi_env_t* iotjs_napi_env = (iotjs_napi_env_t*)env;
  if (iotjs_napi_is_exception_pending(iotjs_napi_env)) {
    jerry_value_t jval_err = iotjs_napi_env_get_and_clear_exception(env);
    if (jval_err != (uintptr_t)NULL) {
      jval_ret = jval_err;
    } else {
      jval_err = iotjs_napi_env_get_and_clear_fatal_exception(env);
      IOTJS_ASSERT(jval_err != (uintptr_t)NULL);

      iotjs_uncaught_exception(jval_err);
      jval_ret = jval_err;
    }

    goto cleanup;
  }

  // TODO: check if nvalue_ret is escaped
  /**
   * Do not turn NULL pointer into undefined since number value `0` in
   * jerryscript also represented by NULL
   */
  jval_ret = AS_JERRY_VALUE(nvalue_ret);


cleanup:
  /**
   * for N-API created value: value is scoped, would be released on :cleanup
   * for passed-in params: value would be automatically release on end of invocation
   */
  jerry_acquire_value(jval_ret);
  jerryx_close_handle_scope(scope);
  /**
   * Clear N-API env extended error info on end of external function
   * execution to prevent error info been passed to next external function.
   */
  iotjs_napi_clear_error_info(env);
  return jval_ret;
}

napi_status napi_create_function(napi_env env, const char* utf8name,
                                 size_t length, napi_callback cb, void* data,
                                 napi_value* result) {
  jerry_value_t jval_func =
      jerry_create_external_function(iotjs_napi_function_handler);
  jerryx_create_handle(jval_func);

  iotjs_function_info_t* function_info = (iotjs_function_info_t*)
      iotjs_get_object_native_info(jval_func, sizeof(iotjs_function_info_t));
  function_info->env = env;
  function_info->cb = cb;
  function_info->data = data;
  jerry_set_object_native_pointer(jval_func, function_info,
                                  &native_obj_type_info);

  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_func));
  NAPI_RETURN(napi_ok);
}

napi_status napi_call_function(napi_env env, napi_value recv, napi_value func,
                               size_t argc, const napi_value* argv,
                               napi_value* result) {
  NAPI_TRY_NO_PENDING_EXCEPTION(env);

  jerry_value_t jval_func = AS_JERRY_VALUE(func);
  jerry_value_t jval_this = AS_JERRY_VALUE(recv);

  NAPI_TRY_TYPE(function, jval_func);

  jerry_value_t jval_ret =
      jerry_call_function(jval_func, jval_this, (jerry_value_t*)argv, argc);
  jerryx_create_handle(jval_ret);
  if (jerry_value_has_error_flag(jval_ret)) {
    NAPI_INTERNAL_CALL(napi_throw(env, AS_NAPI_VALUE(jval_ret)));
    NAPI_RETURN(napi_pending_exception,
                "Unexpected error flag on jerry_call_function.");
  }

  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_ret));
  NAPI_RETURN(napi_ok);
}

napi_status napi_get_cb_info(napi_env env, napi_callback_info cbinfo,
                             size_t* argc, napi_value* argv,
                             napi_value* thisArg, void** data) {
  iotjs_callback_info_t* callback_info = (iotjs_callback_info_t*)cbinfo;

  size_t _argc = argc == NULL ? 0 : *argc;
  for (size_t i = 0; i < _argc; ++i) {
    if (i < callback_info->argc) {
      NAPI_ASSIGN(argv + i, AS_NAPI_VALUE(callback_info->argv[i]));
    } else {
      NAPI_ASSIGN(argv + i, AS_NAPI_VALUE(jerry_create_undefined()));
    }
  }
  NAPI_ASSIGN(argc, callback_info->argc);

  if (thisArg != NULL) {
    NAPI_ASSIGN(thisArg, AS_NAPI_VALUE(callback_info->jval_this));
  }

  if (data != NULL) {
    NAPI_ASSIGN(data, callback_info->function_info->data);
  }

  NAPI_RETURN(napi_ok);
}

napi_status napi_new_instance(napi_env env, napi_value constructor, size_t argc,
                              const napi_value* argv, napi_value* result) {
  NAPI_TRY_NO_PENDING_EXCEPTION(env);

  jerry_value_t jval_cons = AS_JERRY_VALUE(constructor);

  NAPI_TRY_TYPE(function, jval_cons);

  jerry_value_t jval_ret =
      jerry_construct_object(jval_cons, (jerry_value_t*)argv, argc);
  jerryx_create_handle(jval_ret);
  if (jerry_value_has_error_flag(jval_ret)) {
    NAPI_INTERNAL_CALL(napi_throw(env, AS_NAPI_VALUE(jval_ret)));
    NAPI_RETURN(napi_generic_failure,
                "Unexpected error flag on jerry_construct_object.");
  }

  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_ret));
  NAPI_RETURN(napi_ok);
}

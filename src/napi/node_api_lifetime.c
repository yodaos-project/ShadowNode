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
#include "internal/node_api_internal.h"

inline napi_status jerryx_status_to_napi(jerryx_handle_scope_status status) {
  switch (status) {
    case jerryx_handle_scope_mismatch:
      return napi_handle_scope_mismatch;
    case jerryx_escape_called_twice:
      return napi_escape_called_twice;
    default:
      return napi_ok;
  }
}

napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result) {
  jerryx_handle_scope_status status;
  status = jerryx_open_handle_scope((jerryx_handle_scope*)result);

  return jerryx_status_to_napi(status);
}

napi_status napi_open_escapable_handle_scope(
    napi_env env, napi_escapable_handle_scope* result) {
  jerryx_handle_scope_status status;
  status = jerryx_open_escapable_handle_scope(
      (jerryx_escapable_handle_scope*)result);

  return jerryx_status_to_napi(status);
}

napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
  jerryx_handle_scope_status status;
  status = jerryx_close_handle_scope((jerryx_handle_scope)scope);

  return jerryx_status_to_napi(status);
}

napi_status napi_close_escapable_handle_scope(
    napi_env env, napi_escapable_handle_scope scope) {
  jerryx_handle_scope_status status;
  status =
      jerryx_close_escapable_handle_scope((jerryx_escapable_handle_scope)scope);

  return jerryx_status_to_napi(status);
}

napi_status napi_escape_handle(napi_env env, napi_escapable_handle_scope scope,
                               napi_value escapee, napi_value* result) {
  jerryx_handle_scope_status status;
  status =
      jerryx_escape_handle((jerryx_escapable_handle_scope)scope,
                           AS_JERRY_VALUE(escapee), (jerry_value_t*)result);

  return jerryx_status_to_napi(status);
}

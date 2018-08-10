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

#ifndef IOTJS_NODE_API_H
#define IOTJS_NODE_API_H

#include "iotjs_def.h"
#include "jerryscript-ext/handle-scope.h"
#include "jerryscript.h"
#include "internal/node_api_internal_types.h"
#include "node_api.h"

#define AS_JERRY_VALUE(nvalue) (jerry_value_t)(uintptr_t) nvalue
#define AS_NAPI_VALUE(jval) (napi_value)(uintptr_t) jval

#define NAPI_WEAK_ASSERT(error_t, assertion) \
  do {                                       \
    if (!assertion)                          \
      return error_t;                        \
  } while (0)

#define NAPI_TRY_TYPE(type, jval) \
  NAPI_WEAK_ASSERT(napi_##type##_expected, jerry_value_is_##type(jval))

#define NAPI_TRY_NO_PENDING_EXCEPTION(env) \
  NAPI_WEAK_ASSERT(napi_pending_exception, \
                   iotjs_napi_is_exception_pending((iotjs_napi_env_t*)env))

#define NAPI_INTERNAL_CALL(call) \
  do {                           \
    napi_status status;          \
    status = call;               \
    if (status != napi_ok) {     \
      return status;             \
    }                            \
  } while (0)

#define JERRYX_CREATE(var, create) \
  jerry_value_t var = create;      \
  jerryx_create_handle(var);

/** MARK: - node_api_module.c */
int napi_module_init_pending(jerry_value_t* exports);
/** MARK: - END node_api_module.c */

/** MARK: - node_api_env.c */
napi_env iotjs_get_current_napi_env();
bool iotjs_napi_is_exception_pending(iotjs_napi_env_t* env);
napi_status iotjs_napi_env_set_exception(napi_env env, napi_value error);
napi_status iotjs_napi_env_set_fatal_exception(napi_env env, napi_value error);
/** MARK: - END node_api_env.c */

/** MARK: - node_api_lifetime.c */
napi_status jerryx_status_to_napi_status(jerryx_handle_scope_status status);
iotjs_object_info_t* iotjs_get_object_native_info(jerry_value_t jval,
                                                  size_t native_info_size);
/** MARK: - END node_api_lifetime.c */

#endif // IOTJS_NODE_API_H

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

#include "jerryscript.h"
#include "internal/node_api_internal_types.h"
#include "node_api.h"

#define AS_JERRY_VALUE(nvalue) (jerry_value_t)(uintptr_t) nvalue
#define AS_NAPI_VALUE(jval) (napi_value)(uintptr_t) jval
#define napi_malloc(size) malloc(size)

int napi_module_init_pending(jerry_value_t *exports);
napi_env iotjs_get_current_napi_env();
bool iotjs_napi_is_exception_pending(iotjs_napi_env_t *env);
napi_status iotjs_napi_env_set_exception(napi_env env, napi_value error);
napi_status iotjs_napi_env_set_fatal_exception(napi_env env, napi_value error);

#endif // IOTJS_NODE_API_H

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

#include "node_api.h"
#include "jerryscript-ext/handle-scope.h"
#include "jerryscript.h"

napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result) {
  jerry_value_t jval = jerry_create_number((double)value);
  jerryx_create_handle(jval);
  *result = (napi_value)(uintptr_t)jval;
  return napi_ok;
}

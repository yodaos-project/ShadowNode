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
#include "internal/node_api_internal.h"

napi_status napi_set_named_property(napi_env env, napi_value object,
                                    const char* utf8name, napi_value value) {
  jerry_value_t jval = AS_JERRY_VALUE(object);
  jerry_value_t jval_prop_val = AS_JERRY_VALUE(value);
  jerry_value_t jval_prop_name =
      jerry_create_string_from_utf8((jerry_char_t*)utf8name);

  jerry_value_t jval_result =
      jerry_set_property(jval, jval_prop_name, jval_prop_val);
  jerry_release_value(jval_prop_name);

  if (jerry_value_has_error_flag(jval_result)) {
    jerry_release_value(jval_result);
    return napi_invalid_arg;
  }

  jerry_release_value(jval_result);
  return napi_ok;
}

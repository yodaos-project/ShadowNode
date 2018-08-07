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

#define NAPI_NUMBER_CONVERT_FROM_C_TYPE(type, name)          \
  napi_status napi_create_##name(napi_env env, type value,   \
                                 napi_value* result) {       \
    jerry_value_t jval = jerry_create_number((double)value); \
    jerryx_create_handle(jval);                              \
    *result = AS_NAPI_VALUE(jval);                           \
    return napi_ok;                                          \
  }

NAPI_NUMBER_CONVERT_FROM_C_TYPE(int32_t, int32);
NAPI_NUMBER_CONVERT_FROM_C_TYPE(uint32_t, uint32);
NAPI_NUMBER_CONVERT_FROM_C_TYPE(int64_t, int64);
NAPI_NUMBER_CONVERT_FROM_C_TYPE(double, double);

#undef NAPI_NUMBER_CONVERT_FROM_C_TYPE

#define NAPI_NUMBER_CONVERT_FROM_NVALUE(type, name)                 \
  napi_status napi_get_value_##name(napi_env env, napi_value value, \
                                    type* result) {                 \
    jerry_value_t jval = AS_JERRY_VALUE(value);                     \
    double num_val = jerry_get_number_value(jval);                  \
    *result = num_val;                                              \
    return napi_ok;                                                 \
  }

NAPI_NUMBER_CONVERT_FROM_NVALUE(double, double);
NAPI_NUMBER_CONVERT_FROM_NVALUE(int32_t, int32);
NAPI_NUMBER_CONVERT_FROM_NVALUE(int64_t, int64);
NAPI_NUMBER_CONVERT_FROM_NVALUE(uint32_t, uint32);

#undef NAPI_NUMBER_CONVERT_FROM_NVALUE

napi_status napi_create_string_utf8(napi_env env, const char* str,
                                    size_t length, napi_value* result) {
  jerry_value_t jval =
      jerry_create_string_sz_from_utf8((jerry_char_t*)str, length);
  jerryx_create_handle(jval);
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

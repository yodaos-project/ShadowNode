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

napi_status napi_create_array(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_array(0));
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

napi_status napi_create_array_with_length(napi_env env, size_t length,
                                          napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_array(length));
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

napi_status napi_create_object(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_object());
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

#define DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(type, name)      \
  napi_status napi_create_##name(napi_env env, type value,   \
                                 napi_value* result) {       \
    jerry_value_t jval = jerry_create_number((double)value); \
    jerryx_create_handle(jval);                              \
    *result = AS_NAPI_VALUE(jval);                           \
    return napi_ok;                                          \
  }

DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(int32_t, int32);
DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(uint32_t, uint32);
DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(int64_t, int64);
DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(double, double);
#undef DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE

#define DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(type, name)             \
  napi_status napi_get_value_##name(napi_env env, napi_value value, \
                                    type* result) {                 \
    jerry_value_t jval = AS_JERRY_VALUE(value);                     \
    double num_val = jerry_get_number_value(jval);                  \
    *result = num_val;                                              \
    return napi_ok;                                                 \
  }

DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(double, double);
DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(int32_t, int32);
DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(int64_t, int64);
DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(uint32_t, uint32);
#undef DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE

napi_status napi_create_string_utf8(napi_env env, const char* str,
                                    size_t length, napi_value* result) {
  jerry_value_t jval =
      jerry_create_string_sz_from_utf8((jerry_char_t*)str, length);
  jerryx_create_handle(jval);
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

napi_status napi_get_array_length(napi_env env, napi_value value,
                                  uint32_t* result) {
  jerry_value_t jval = AS_JERRY_VALUE(value);
  *result = jerry_get_array_length(jval);
  return napi_ok;
}

napi_status napi_get_prototype(napi_env env, napi_value object,
                               napi_value* result) {
  jerry_value_t jval = AS_JERRY_VALUE(object);
  jerry_value_t jval_proto = jerry_get_prototype(jval);
  *result = AS_NAPI_VALUE(jval_proto);
  return napi_ok;
}

napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result) {
  jerry_value_t jval = AS_JERRY_VALUE(value);
  if (!jerry_value_is_boolean(jval))
    return napi_boolean_expected;
  *result = jerry_get_boolean_value(jval);
  return napi_ok;
}

napi_status napi_get_boolean(napi_env env, bool value, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_boolean(value));
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

napi_status napi_get_global(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_get_global_object());
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

napi_status napi_get_null(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_null());
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

napi_status napi_get_undefined(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_undefined());
  *result = AS_NAPI_VALUE(jval);
  return napi_ok;
}

#define DEF_NAPI_COERCE_TO(type, alias)                             \
  napi_status napi_coerce_to_##type(napi_env env, napi_value value, \
                                    napi_value* result) {           \
    jerry_value_t jval = AS_JERRY_VALUE(value);                     \
    jerry_value_t jval_result = jerry_value_to_##alias(jval);       \
    *result = AS_NAPI_VALUE(jval_result);                           \
    return napi_ok;                                                 \
  }

DEF_NAPI_COERCE_TO(bool, boolean);
DEF_NAPI_COERCE_TO(number, number);
DEF_NAPI_COERCE_TO(object, object);
DEF_NAPI_COERCE_TO(string, string);

napi_status napi_typeof(napi_env env, napi_value value,
                        napi_valuetype* result) {
  jerry_value_t jval = AS_JERRY_VALUE(value);
  jerry_type_t type = jerry_value_get_type(jval);

#define CM(jerry, napi) \
  case jerry:           \
    *result = napi;     \
    break;

  switch (type) {
    CM(JERRY_TYPE_UNDEFINED, napi_undefined);
    CM(JERRY_TYPE_NULL, napi_null);
    CM(JERRY_TYPE_BOOLEAN, napi_boolean);
    CM(JERRY_TYPE_NUMBER, napi_number);
    CM(JERRY_TYPE_STRING, napi_string);
    CM(JERRY_TYPE_OBJECT, napi_object);
    CM(JERRY_TYPE_FUNCTION, napi_function);
#undef CM
    default:
      return napi_invalid_arg;
  }

  return napi_ok;
}

#define DEF_NAPI_VALUE_IS(type)                                              \
  napi_status napi_is_##type(napi_env env, napi_value value, bool* result) { \
    jerry_value_t jval = AS_JERRY_VALUE(value);                              \
    *result = jerry_value_is_##type(jval);                                   \
    return napi_ok;                                                          \
  }

DEF_NAPI_VALUE_IS(array);
DEF_NAPI_VALUE_IS(arraybuffer);
DEF_NAPI_VALUE_IS(error);
DEF_NAPI_VALUE_IS(typedarray);

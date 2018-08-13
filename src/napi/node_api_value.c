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
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  return napi_ok;
}

napi_status napi_create_array_with_length(napi_env env, size_t length,
                                          napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_array(length));
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  return napi_ok;
}

napi_status napi_create_object(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_object());
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  return napi_ok;
}

#define DEF_NAPI_CREATE_ERROR(type, jerry_error_type)                         \
  napi_status napi_create_##type(napi_env env, napi_value code,               \
                                 napi_value msg, napi_value* result) {        \
    if (env != iotjs_get_current_napi_env())                                  \
      return napi_invalid_arg;                                                \
                                                                              \
    jerry_value_t jval_code = AS_JERRY_VALUE(code);                           \
    jerry_value_t jval_msg = AS_JERRY_VALUE(msg);                             \
                                                                              \
    NAPI_TRY_TYPE(string, jval_code);                                         \
    NAPI_TRY_TYPE(string, jval_msg);                                          \
                                                                              \
    jerry_size_t msg_size = jerry_get_utf8_string_size(jval_msg);             \
    jerry_char_t raw_msg[msg_size];                                           \
    jerry_size_t written_size =                                               \
        jerry_string_to_utf8_char_buffer(jval_msg, raw_msg, msg_size);        \
    NAPI_WEAK_ASSERT(napi_invalid_arg, written_size == msg_size);             \
                                                                              \
    JERRYX_CREATE(jval_error, jerry_create_error(jerry_error_type, raw_msg)); \
    jerry_value_clear_error_flag(&jval_error);                                \
                                                                              \
    iotjs_jval_set_property_jval(jval_error, "code", jval_code);              \
    NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_error));                           \
                                                                              \
    return napi_ok;                                                           \
  }

DEF_NAPI_CREATE_ERROR(error, JERRY_ERROR_COMMON);
DEF_NAPI_CREATE_ERROR(type_error, JERRY_ERROR_TYPE);
DEF_NAPI_CREATE_ERROR(range_error, JERRY_ERROR_RANGE);
#undef DEF_NAPI_CREATE_ERROR

#define DEF_NAPI_NUMBER_CONVERT_FROM_C_TYPE(type, name)      \
  napi_status napi_create_##name(napi_env env, type value,   \
                                 napi_value* result) {       \
    JERRYX_CREATE(jval, jerry_create_number((double)value)); \
    NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));                \
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
    NAPI_TRY_TYPE(number, jval);                                    \
    double num_val = jerry_get_number_value(jval);                  \
    NAPI_ASSIGN(result, num_val);                                   \
    return napi_ok;                                                 \
  }

DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(double, double);
DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(int32_t, int32);
DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(int64_t, int64);
DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE(uint32_t, uint32);
#undef DEF_NAPI_NUMBER_CONVERT_FROM_NVALUE

napi_status napi_create_string_utf8(napi_env env, const char* str,
                                    size_t length, napi_value* result) {
  if (length == NAPI_AUTO_LENGTH) {
    length = strlen(str);
  }
  JERRYX_CREATE(jval,
                jerry_create_string_sz_from_utf8((jerry_char_t*)str, length));
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  return napi_ok;
}

napi_status napi_get_array_length(napi_env env, napi_value value,
                                  uint32_t* result) {
  jerry_value_t jval = AS_JERRY_VALUE(value);
  NAPI_ASSIGN(result, jerry_get_array_length(jval));
  return napi_ok;
}

napi_status napi_get_prototype(napi_env env, napi_value object,
                               napi_value* result) {
  jerry_value_t jval = AS_JERRY_VALUE(object);
  JERRYX_CREATE(jval_proto, jerry_get_prototype(jval));
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_proto));
  return napi_ok;
}

napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result) {
  jerry_value_t jval = AS_JERRY_VALUE(value);
  NAPI_TRY_TYPE(boolean, jval);
  NAPI_ASSIGN(result, jerry_get_boolean_value(jval));
  return napi_ok;
}

napi_status napi_get_boolean(napi_env env, bool value, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_boolean(value));
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  return napi_ok;
}

napi_status napi_get_global(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_get_global_object());
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  return napi_ok;
}

napi_status napi_get_null(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_null());
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  return napi_ok;
}

napi_status napi_get_undefined(napi_env env, napi_value* result) {
  JERRYX_CREATE(jval, jerry_create_undefined());
  NAPI_ASSIGN(result, AS_NAPI_VALUE(jval));
  return napi_ok;
}

#define DEF_NAPI_COERCE_TO(type, alias)                             \
  napi_status napi_coerce_to_##type(napi_env env, napi_value value, \
                                    napi_value* result) {           \
    jerry_value_t jval = AS_JERRY_VALUE(value);                     \
    JERRYX_CREATE(jval_result, jerry_value_to_##alias(jval));       \
    NAPI_ASSIGN(result, AS_NAPI_VALUE(jval_result));                \
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

#define MAP(jerry, napi)       \
  case jerry:                  \
    NAPI_ASSIGN(result, napi); \
    break;

  switch (type) {
    MAP(JERRY_TYPE_UNDEFINED, napi_undefined);
    MAP(JERRY_TYPE_NULL, napi_null);
    MAP(JERRY_TYPE_BOOLEAN, napi_boolean);
    MAP(JERRY_TYPE_NUMBER, napi_number);
    MAP(JERRY_TYPE_STRING, napi_string);
    MAP(JERRY_TYPE_OBJECT, napi_object);
    MAP(JERRY_TYPE_FUNCTION, napi_function);
#undef MAP
    default:
      return napi_invalid_arg;
  }

  return napi_ok;
}

napi_status napi_instanceof(napi_env env, napi_value object,
                            napi_value constructor, bool* result) {
  jerry_value_t jval_object = AS_JERRY_VALUE(object);
  jerry_value_t jval_cons = AS_JERRY_VALUE(constructor);

  NAPI_ASSIGN(result, jerry_value_instanceof(jval_object, jval_cons));
  return napi_ok;
}

#define DEF_NAPI_VALUE_IS(type)                                              \
  napi_status napi_is_##type(napi_env env, napi_value value, bool* result) { \
    jerry_value_t jval = AS_JERRY_VALUE(value);                              \
    NAPI_ASSIGN(result, jerry_value_is_##type(jval));                        \
    return napi_ok;                                                          \
  }

DEF_NAPI_VALUE_IS(array);
DEF_NAPI_VALUE_IS(arraybuffer);
DEF_NAPI_VALUE_IS(typedarray);

napi_status napi_is_error(napi_env env, napi_value value, bool* result) {
  jerry_value_t jval = AS_JERRY_VALUE(value);
  /**
   * TODO: Pick jerrysciprt#ba2e49caaa6703dec7a83fb0b8586a91fac060eb to use
   * function `jerry_value_is_error`
   */
  NAPI_ASSIGN(result, jerry_value_has_error_flag(jval));
  return napi_ok;
}

napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs,
                               bool* result) {
  jerry_value_t jval_lhs = AS_JERRY_VALUE(lhs);
  jerry_value_t jval_rhs = AS_JERRY_VALUE(rhs);

  NAPI_ASSIGN(result, jerry_value_strict_equal(jval_lhs, jval_rhs));
  return napi_ok;
}

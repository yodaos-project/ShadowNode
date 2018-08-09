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

napi_status napi_define_class(napi_env env, const char* utf8name, size_t length,
                              napi_callback constructor, void* data,
                              size_t property_count,
                              const napi_property_descriptor* properties,
                              napi_value* result) {
  napi_value nval;
  NAPI_INTERNAL_CALL(
      napi_create_function(env, utf8name, length, constructor, data, &nval));
  napi_value nval_prototype;
  NAPI_INTERNAL_CALL(
      napi_get_named_property(env, nval, "prototype", &nval_prototype));

  for (size_t i = 0; i < property_count; ++i) {
    napi_property_descriptor prop = properties[i];
    if (prop.attributes & napi_static) {
      NAPI_INTERNAL_CALL(napi_define_properties(env, nval, 1, &prop));
    } else {
      NAPI_INTERNAL_CALL(napi_define_properties(env, nval_prototype, 1, &prop));
    }
  }

  return napi_ok;
}

napi_status napi_wrap(napi_env env, napi_value js_object, void* native_object,
                      napi_finalize finalize_cb, void* finalize_hint,
                      napi_ref* result) {
  jerry_value_t jval = AS_JERRY_VALUE(js_object);
  iotjs_object_info_t* object_info =
      iotjs_get_object_native_info(jval, sizeof(iotjs_object_info_t));

  NAPI_WEAK_ASSERT(napi_invalid_arg, (object_info->native_object == NULL));
  NAPI_WEAK_ASSERT(napi_invalid_arg, (object_info->finalize_cb == NULL));
  NAPI_WEAK_ASSERT(napi_invalid_arg, (object_info->finalize_hint == NULL));

  object_info->env = env;
  object_info->native_object = native_object;
  object_info->finalize_cb = finalize_cb;
  object_info->finalize_hint = finalize_hint;

  return napi_create_reference(env, js_object, 1, result);
}

napi_status napi_unwrap(napi_env env, napi_value js_object, void** result) {
  jerry_value_t jval = AS_JERRY_VALUE(js_object);
  iotjs_object_info_t* object_info =
      iotjs_get_object_native_info(jval, sizeof(iotjs_object_info_t));

  *result = object_info->native_object;
  return napi_ok;
}

napi_status napi_remove_wrap(napi_env env, napi_value js_object,
                             void** result) {
  jerry_value_t jval = AS_JERRY_VALUE(js_object);
  iotjs_object_info_t* object_info =
      iotjs_get_object_native_info(jval, sizeof(iotjs_object_info_t));

  object_info->native_object = NULL;
  object_info->finalize_cb = NULL;
  object_info->finalize_hint = NULL;

  return napi_ok;
}

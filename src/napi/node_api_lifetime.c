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

static void native_info_free(void* native_info) {
  iotjs_object_info_t* info = (iotjs_object_info_t*)native_info;
  if (info->ref != NULL) {
    info->ref->jval = AS_JERRY_VALUE(NULL);
  }

  if (info->finalize_cb != NULL) {
    info->finalize_cb(info->env, info->native_object, info->finalize_hint);
  }

  free(info);
}

static const jerry_object_native_info_t native_obj_type_info = {
  .free_cb = native_info_free
};

inline napi_status jerryx_status_to_napi_status(
    jerryx_handle_scope_status status) {
  switch (status) {
    case jerryx_handle_scope_mismatch:
      return napi_handle_scope_mismatch;
    case jerryx_escape_called_twice:
      return napi_escape_called_twice;
    default:
      return napi_ok;
  }
}

iotjs_object_info_t* iotjs_get_object_native_info(jerry_value_t jval,
                                                  size_t native_info_size) {
  iotjs_object_info_t* info;
  bool has_native_ptr =
      jerry_get_object_native_pointer(jval, (void**)&info, NULL);
  if (!has_native_ptr) {
    info = (iotjs_object_info_t*)iotjs_buffer_allocate(
        native_info_size < sizeof(iotjs_object_info_t)
            ? sizeof(iotjs_object_info_t)
            : native_info_size);
    jerry_set_object_native_pointer(jval, info, &native_obj_type_info);
  }

  return info;
}

napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result) {
  jerryx_handle_scope_status status;
  status = jerryx_open_handle_scope((jerryx_handle_scope*)result);

  return jerryx_status_to_napi_status(status);
}

napi_status napi_open_escapable_handle_scope(
    napi_env env, napi_escapable_handle_scope* result) {
  jerryx_handle_scope_status status;
  status = jerryx_open_escapable_handle_scope(
      (jerryx_escapable_handle_scope*)result);

  return jerryx_status_to_napi_status(status);
}

napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
  jerryx_handle_scope_status status;
  status = jerryx_close_handle_scope((jerryx_handle_scope)scope);

  return jerryx_status_to_napi_status(status);
}

napi_status napi_close_escapable_handle_scope(
    napi_env env, napi_escapable_handle_scope scope) {
  jerryx_handle_scope_status status;
  status =
      jerryx_close_escapable_handle_scope((jerryx_escapable_handle_scope)scope);

  return jerryx_status_to_napi_status(status);
}

napi_status napi_escape_handle(napi_env env, napi_escapable_handle_scope scope,
                               napi_value escapee, napi_value* result) {
  jerryx_handle_scope_status status;
  status =
      jerryx_escape_handle((jerryx_escapable_handle_scope)scope,
                           AS_JERRY_VALUE(escapee), (jerry_value_t*)result);

  return jerryx_status_to_napi_status(status);
}

napi_status napi_create_reference(napi_env env, napi_value value,
                                  uint32_t initial_refcount, napi_ref* result) {
  jerry_value_t jval = AS_JERRY_VALUE(value);
  iotjs_object_info_t* info;
  bool has_native_ptr =
      jerry_get_object_native_pointer(jval, (void**)&info, NULL);
  if (!has_native_ptr) {
    info = IOTJS_ALLOC(iotjs_object_info_t);
  } else {
    NAPI_WEAK_ASSERT(napi_invalid_arg, (info->ref != NULL));
  }

  iotjs_reference_t* ref = IOTJS_ALLOC(iotjs_reference_t);
  ref->refcount = initial_refcount;
  ref->jval = AS_JERRY_VALUE(value);
  info->ref = ref;

  *result = (napi_ref)ref;
  return napi_ok;
}

napi_status napi_delete_reference(napi_env env, napi_ref ref) {
  iotjs_reference_t* iot_ref = (iotjs_reference_t*)ref;
  if (iot_ref->jval != AS_JERRY_VALUE(NULL)) {
    jerry_value_t jval = iot_ref->jval;
    iotjs_object_info_t* info;
    bool has_native_ptr =
        jerry_get_object_native_pointer(jval, (void**)&info, NULL);
    NAPI_WEAK_ASSERT(napi_invalid_arg, has_native_ptr);
    NAPI_WEAK_ASSERT(napi_invalid_arg, (info->ref == iot_ref));
    info->ref = NULL;
  }
  for (uint32_t i = 0; i < iot_ref->refcount; ++i) {
    jerry_release_value(iot_ref->jval);
  }
  free(iot_ref);
  return napi_ok;
}

napi_status napi_reference_ref(napi_env env, napi_ref ref, uint32_t* result) {
  iotjs_reference_t* iot_ref = (iotjs_reference_t*)ref;
  NAPI_WEAK_ASSERT(napi_invalid_arg, (iot_ref->jval != AS_JERRY_VALUE(NULL)));

  jerry_acquire_value(iot_ref->jval);
  iot_ref->refcount += 1;

  *result = iot_ref->refcount;
  return napi_ok;
}

napi_status napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result) {
  iotjs_reference_t* iot_ref = (iotjs_reference_t*)ref;
  NAPI_WEAK_ASSERT(napi_invalid_arg, (iot_ref->refcount > 0));

  jerry_release_value(iot_ref->jval);
  iot_ref->refcount -= 1;

  *result = iot_ref->refcount;
  return napi_ok;
}

napi_status napi_get_reference_value(napi_env env, napi_ref ref,
                                     napi_value* result) {
  iotjs_reference_t* iot_ref = (iotjs_reference_t*)ref;
  *result = AS_NAPI_VALUE(iot_ref->jval);
  return napi_ok;
}

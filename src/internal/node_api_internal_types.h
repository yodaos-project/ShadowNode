
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

#ifndef IOTJS_NODE_API_TYPES_H
#define IOTJS_NODE_API_TYPES_H

#include "jerryscript.h"
#include "node_api.h"

typedef napi_value (*jerry_addon_register_func)(void* env,
                                                jerry_value_t exports);

typedef enum {
  napi_module_load_ok = 0,

  napi_module_no_pending,
  napi_module_no_nm_register_func,
} napi_module_load_status;

typedef struct {
  napi_value pending_exception;
  napi_value pending_fatal_exception;
  napi_extended_error_info extended_error_info;
} iotjs_napi_env_t;

typedef struct {
  jerry_value_t jval;
  uint32_t refcount;
} iotjs_reference_t;


#define IOTJS_OBJECT_INFO_FIELDS \
  napi_env env;                  \
  void* native_object;           \
  napi_finalize finalize_cb;     \
  void* finalize_hint;           \
  iotjs_reference_t* ref;

typedef struct {
  IOTJS_OBJECT_INFO_FIELDS;
} iotjs_object_info_t;

typedef struct {
  IOTJS_OBJECT_INFO_FIELDS;

  napi_callback cb;
  void* data;
} iotjs_function_info_t;

typedef struct {
  size_t argc;
  jerry_value_t* argv;
  jerry_value_t jval_this;

  iotjs_function_info_t* function_info;
} iotjs_callback_info_t;

#endif // IOTJS_NODE_API_TYPES_H

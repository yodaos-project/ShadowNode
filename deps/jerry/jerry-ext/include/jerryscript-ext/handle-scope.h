/*
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

#ifndef JERRYX_HANDLE_SCOPE_H
#define JERRYX_HANDLE_SCOPE_H

#include "jerryscript.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifndef JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT
#define JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT 20
#endif

#ifndef JERRY_X_HANDLE_SCOPE_PRELIST_SCOPE_COUNT
#define JERRY_X_HANDLE_SCOPE_PRELIST_SCOPE_COUNT 20
#endif

typedef struct jerryx_handle_t jerryx_handle_t;
struct jerryx_handle_t {
  jerry_value_t jval;
  jerryx_handle_t *sibling;
};

typedef struct jerryx_handle_scope_t jerryx_handle_scope_t;
typedef jerryx_handle_scope_t *jerryx_handle_scope;
typedef jerryx_handle_scope_t *jerryx_escapable_handle_scope;
struct jerryx_handle_scope_t {
  jerry_value_t handle_prelist[JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT];
  size_t handle_count;
  jerryx_handle_t *handle_ptr;
};


typedef struct jerryx_handle_scope_dynamic_t jerryx_handle_scope_dynamic_t;
struct jerryx_handle_scope_dynamic_t {
  jerry_value_t handle_prelist[JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT];
  size_t handle_count;
  jerryx_handle_t *handle_ptr;
  jerryx_handle_scope_dynamic_t *child;
  jerryx_handle_scope_dynamic_t *parent;
};

typedef enum {
  jerryx_handle_scope_ok = 0,

  jerryx_escape_called_twice,
  jerryx_handle_scope_mismatch,
} jerryx_handle_scope_status;

jerryx_handle_scope_status
jerryx_open_handle_scope (jerryx_handle_scope *result);

jerryx_handle_scope_status
jerryx_close_handle_scope (jerryx_handle_scope scope);

jerryx_handle_scope_status
jerryx_open_escapable_handle_scope (jerryx_handle_scope *result);

jerryx_handle_scope_status
jerryx_close_escapable_handle_scope (jerryx_handle_scope scope);

jerryx_handle_scope_status
jerryx_escape_handle (jerryx_escapable_handle_scope scope,
                      jerry_value_t escapee,
                      jerry_value_t *result);

jerry_value_t
jerryx_handle_add (jerry_value_t jval);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !JERRYX_HANDLE_SCOPE_H */

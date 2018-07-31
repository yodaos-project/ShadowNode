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

#ifndef JERRYX_HANDLE_SCOPE_INTERNAL_H
#define JERRYX_HANDLE_SCOPE_INTERNAL_H

#include "jerryscript.h"
#include "jerryscript-ext/handle-scope.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/** MARK: - handle-scope-allocator.c */
/**
 * A linear allocating memory pool for type jerryx_handle_scope_t,
 * in which allocated item shall be released in reversed order of allocation
 */
typedef struct jerryx_handle_scope_pool_t jerryx_handle_scope_pool_t;
struct jerryx_handle_scope_pool_t {
  jerryx_handle_scope_t prelist[JERRY_X_HANDLE_SCOPE_PRELIST_SCOPE_COUNT];
  size_t count;
  jerryx_handle_scope_dynamic_t *start;
  jerryx_handle_scope_dynamic_t *end;
};

jerryx_handle_scope_t *
jerryx_handle_scope_get_current (void);

jerryx_handle_scope_t *
jerryx_handle_scope_get_root (void);

bool
jerryx_handle_scope_is_in_prelist (jerryx_handle_scope_t *scope);

jerryx_handle_scope_t *
jerryx_handle_scope_get_parent (jerryx_handle_scope_t *scope);

jerryx_handle_scope_t *
jerryx_handle_scope_get_child (jerryx_handle_scope_t *scope);

jerryx_handle_scope_t *
jerryx_handle_scope_alloc (void);

void
jerryx_handle_scope_free (jerryx_handle_scope_t *scope);
/** MARK: - END handle-scope-allocator.c */

/** MARK: - handle-scope.c */
void
jerryx_handle_scope_release_handles (jerryx_handle_scope scope);

void
jerryx_hand_scope_escape_handle_from_prelist (jerryx_handle_scope scope, size_t idx);

jerry_value_t
jerryx_handle_scope_add_handle_to (jerryx_handle_t *handle, jerryx_handle_scope scope);

void
jerryx_handle_scope_add_to (jerry_value_t jval, jerryx_handle_scope scope);
/** MARK: - END handle-scope.c */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !JERRYX_HANDLE_SCOPE_INTERNAL_H */

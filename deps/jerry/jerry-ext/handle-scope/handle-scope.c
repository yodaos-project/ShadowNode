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

#include <stdlib.h>
#include "jerryscript-ext/handle-scope.h"

const jerryx_handle_scope_t jerryx_handle_scope_root;
jerryx_handle_scope jerryx_handle_scope_current = (jerryx_handle_scope) &jerryx_handle_scope_root;

/**
 * Opens a new handle scope and attach it to current global scope as a child scope.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_open_handle_scope(jerryx_handle_scope *result)
{
  jerryx_handle_scope scope = malloc(sizeof(jerryx_handle_scope_t));
  scope->handle_ptr = NULL;
  scope->child = NULL;
  jerryx_handle_scope_current->child = scope;
  scope->parent = jerryx_handle_scope_current;

  jerryx_handle_scope_current = scope;
  *result = scope;
  return jerryx_handle_scope_ok;
}


/**
 * Release all jerry values that the handle and its siblings holds.
 */
void
jerryx_handle_scope_release_handles(jerryx_handle_t *handle) {
  jerryx_handle_t *a_handle = handle;
  while (a_handle != NULL)
  {
    jerry_release_value(a_handle->jval);
    jerryx_handle_t *sibling = a_handle->sibling;
    free(a_handle);
    a_handle = sibling;
  }
}


/**
 * Close the scope and its child scopes and release all jerry values that
 * resides in the scopes.
 * Scopes must be closed in the reverse order from which they were created.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_close_handle_scope(jerryx_handle_scope scope)
{
  if (scope->parent != NULL)
  {
    scope->parent->child = NULL;
  }

  /**
   * Release all handles related to given scope and its child scopes
   */
  jerryx_handle_scope a_scope = scope;
  do
  {
    jerryx_handle_scope_release_handles(a_scope->handle_ptr);
    jerryx_handle_scope child = a_scope->child;
    free(a_scope);
    a_scope = child;
  } while(a_scope != NULL);

  return jerryx_handle_scope_ok;
}


/**
 * Opens a new handle scope from which one object can be promoted to the outer scope
 * and attach it to current global scope as a child scope.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_open_escapable_handle_scope(jerryx_handle_scope *result)
{
  return jerryx_open_handle_scope(result);
}


/**
 * Close the scope and its child scopes and release all jerry values that
 * resides in the scopes.
 * Scopes must be closed in the reverse order from which they were created.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_close_escapable_handle_scope(jerryx_handle_scope scope)
{
  return jerryx_close_handle_scope(scope);
}


/**
 * Promotes the handle to the JavaScript object so that it is valid for the lifetime of
 * the outer scope. It can only be called once per scope. If it is called more than
 * once an error will be returned.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_escape_handle(jerryx_escapable_handle_scope scope,
                     jerry_value_t escapee,
                     jerry_value_t *result)
{
  bool found = false;
  jerryx_handle_t *handle = scope->handle_ptr;
  jerryx_handle_t *memo_handle;
  jerryx_handle_t *found_handle;
  while (!found)
  {
    if (handle == NULL)
    {
      return jerryx_handle_scope_mismatch;
    }
    if (handle->jval != escapee)
    {
      memo_handle = handle;
      handle = handle->sibling;
      continue;
    }
    /**
     * Remove found handle from current scope's handle chain
     */
    found = true;
    found_handle = handle;
    if (memo_handle != NULL)
    {
      memo_handle->sibling = found_handle->sibling;
    }
  }

  if (scope->parent == NULL) {
    return jerryx_handle_scope_mismatch;
  }

  /**
   * Escape handle to parent scope
   */
  jerryx_handle_scope parent = scope->parent;
  found_handle->sibling = parent->handle_ptr;
  parent->handle_ptr = found_handle;
  *result = found_handle->jval;

  return jerryx_handle_scope_ok;
}


void
jerryx_handle_scope_add_to(jerry_value_t jval, jerryx_handle_scope scope)
{
  jerryx_handle_t *handle = malloc(sizeof(jerryx_handle_t));
  handle->jval = jval;

  handle->sibling = scope->handle_ptr;
  scope->handle_ptr = handle;
}


jerry_value_t
jerryx_handle_add(jerry_value_t jval)
{
  jerryx_handle_scope_add_to(jval, jerryx_handle_scope_current);
  return jval;
}

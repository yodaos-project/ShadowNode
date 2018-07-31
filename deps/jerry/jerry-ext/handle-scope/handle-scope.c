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
#include "handle-scope-internal.h"

static jerryx_handle_scope_t jerryx_handle_scope_root;
static jerryx_handle_scope jerryx_handle_scope_current = (jerryx_handle_scope) &jerryx_handle_scope_root;

/**
 * Opens a new handle scope and attach it to current global scope as a child scope.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_open_handle_scope (jerryx_handle_scope *result)
{
  jerryx_handle_scope scope = malloc (sizeof(jerryx_handle_scope_t));
  scope->handle_count = 0;
  scope->handle_ptr = NULL;
  scope->child = NULL;
  jerryx_handle_scope_current->child = scope;
  scope->parent = jerryx_handle_scope_current;

  jerryx_handle_scope_current = scope;
  *result = scope;
  return jerryx_handle_scope_ok;
}


/**
 * Release all jerry values attached to given scope
 */
void
jerryx_handle_scope_release_handles (jerryx_handle_scope scope)
{
  size_t handle_count = scope->handle_count;
  if (scope->handle_count > JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT)
  {
    jerryx_handle_t *a_handle = scope->handle_ptr;
    while (a_handle != NULL)
    {
      jerry_release_value (a_handle->jval);
      jerryx_handle_t *sibling = a_handle->sibling;
      free (a_handle);
      a_handle = sibling;
    }
    handle_count = JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT;
  }

  for (size_t idx = 0; idx < handle_count; idx++)
  {
    jerry_release_value(scope->handle_prelist[idx]);
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
jerryx_close_handle_scope (jerryx_handle_scope scope)
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
    jerryx_handle_scope_release_handles (a_scope);
    jerryx_handle_scope child = a_scope->child;
    free (a_scope);
    a_scope = child;
  } while (a_scope != NULL);

  return jerryx_handle_scope_ok;
}


/**
 * Opens a new handle scope from which one object can be promoted to the outer scope
 * and attach it to current global scope as a child scope.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_open_escapable_handle_scope (jerryx_handle_scope *result)
{
  return jerryx_open_handle_scope (result);
}


/**
 * Close the scope and its child scopes and release all jerry values that
 * resides in the scopes.
 * Scopes must be closed in the reverse order from which they were created.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_close_escapable_handle_scope (jerryx_handle_scope scope)
{
  return jerryx_close_handle_scope (scope);
}


/**
 * Escape a jerry value from the scope to its parent scope.
 * An assertion of if parent exists shall be made before invoking this function.
 *
 */
void
jerryx_hand_scope_escape_handle_from_prelist (jerryx_handle_scope scope, size_t idx)
{
  jerryx_handle_scope_add_to (scope->handle_prelist[idx], scope->parent);

  if (scope->handle_count > JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT)
  {
    jerryx_handle_t *handle = scope->handle_ptr;
    scope->handle_ptr = handle->sibling;
    scope->handle_prelist[idx] = handle->jval;
    scope->handle_count -= 1;
    return;
  }

  if (idx < JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT - 1)
  {
    scope->handle_prelist[idx] = scope->handle_prelist[JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT - 1];
  }
  scope->handle_count -= 1;
}


/**
 * Promotes the handle to the JavaScript object so that it is valid for the lifetime of
 * the outer scope. It can only be called once per scope. If it is called more than
 * once an error will be returned.
 *
 * @return status code, jerryx_handle_scope_ok if success.
 */
jerryx_handle_scope_status
jerryx_escape_handle (jerryx_escapable_handle_scope scope,
                      jerry_value_t escapee,
                      jerry_value_t *result)
{
  jerryx_handle_scope parent = scope->parent;
  if (parent == NULL)
  {
    return jerryx_handle_scope_mismatch;
  }

  bool found = false;
  do
  {
    size_t found_idx = 0;
    for (size_t idx = 0; idx < JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT; idx++)
    {
      if (escapee == scope->handle_prelist[idx])
      {
        found = true;
        found_idx = idx;
        break;
      }
    }

    if (found)
    {
      jerryx_hand_scope_escape_handle_from_prelist (scope, found_idx);
      return jerryx_handle_scope_ok;
    }
  }
  while (0);

  if (scope->handle_count <= JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT)
  {
    return jerryx_handle_scope_mismatch;
  }

  jerryx_handle_t *handle = scope->handle_ptr;
  jerryx_handle_t *memo_handle = NULL;
  jerryx_handle_t *found_handle = NULL;
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
    if (memo_handle == NULL)
    {
      // found handle is the first handle in heap
      scope->handle_ptr = found_handle->sibling;
    }
    else
    {
      memo_handle->sibling = found_handle->sibling;
    }
  }

  /**
   * Escape handle to parent scope
   */
  *result = jerryx_handle_scope_add_handle_to (found_handle, parent);
  scope->handle_count -= 1;

  return jerryx_handle_scope_ok;
}


/**
 * Try to reuse given handle if possible while adding to the scope.
 */
jerry_value_t
jerryx_handle_scope_add_handle_to (jerryx_handle_t *handle, jerryx_handle_scope scope)
{
  size_t handle_count = scope->handle_count;
  scope->handle_count += 1;
  if (handle_count < JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT)
  {
    jerry_value_t jval = handle->jval;
    free (handle);
    scope->handle_prelist[handle_count] = jval;
    return jval;
  }

  handle->sibling = scope->handle_ptr;
  scope->handle_ptr = handle;
  return handle->jval;
}


/**
 * Add given jerry value to the scope.
 */
void
jerryx_handle_scope_add_to (jerry_value_t jval, jerryx_handle_scope scope)
{
  size_t handle_count = scope->handle_count;
  scope->handle_count += 1;
  if (handle_count < JERRY_X_HANDLE_SCOPE_PRELIST_HANDLE_COUNT)
  {
    scope->handle_prelist[handle_count] = jval;
    scope->handle_count = handle_count + 1;
    return;
  }
  jerryx_handle_t *handle = malloc (sizeof(jerryx_handle_t));
  handle->jval = jval;

  handle->sibling = scope->handle_ptr;
  scope->handle_ptr = handle;
}


/**
 * Add given jerry value to current top scope.
 */
jerry_value_t
jerryx_handle_add (jerry_value_t jval)
{
  jerryx_handle_scope_add_to (jval, jerryx_handle_scope_current);
  return jval;
}

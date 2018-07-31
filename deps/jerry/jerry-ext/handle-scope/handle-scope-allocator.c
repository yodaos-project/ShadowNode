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

static jerryx_handle_scope_t kJerryXHandleScopeRoot = {
  .handle_count = 0,
  .handle_ptr = NULL,
};
static jerryx_handle_scope_t *kJerryXHandleScopeCurrent = &kJerryXHandleScopeRoot;
static jerryx_handle_scope_pool_t kJerryXHandleScopePool = {
  .count = 0,
  .start = NULL,
  .end = NULL,
};

#define kJerryXHandleScopePoolPrelistLast \
  kJerryXHandleScopePool.prelist + JERRY_X_HANDLE_SCOPE_PRELIST_SCOPE_COUNT - 1

#define JerryXHandleScopePrelistIdx(scope) (scope - kJerryXHandleScopePool.prelist)


jerryx_handle_scope_t *
jerryx_handle_scope_get_current (void)
{
  return kJerryXHandleScopeCurrent;
}


jerryx_handle_scope_t *
jerryx_handle_scope_get_root (void)
{
  return &kJerryXHandleScopeRoot;
}


bool
jerryx_handle_scope_is_in_prelist (jerryx_handle_scope_t *scope)
{
  return (kJerryXHandleScopePool.prelist <= scope) &&
    (scope <= (kJerryXHandleScopePool.prelist + JERRY_X_HANDLE_SCOPE_PRELIST_SCOPE_COUNT - 1));
}


jerryx_handle_scope_t *
jerryx_handle_scope_get_parent (jerryx_handle_scope_t *scope)
{
  if (!jerryx_handle_scope_is_in_prelist (scope))
  {
    jerryx_handle_scope_dynamic_t *dy_scope = (jerryx_handle_scope_dynamic_t *) scope;
    if (dy_scope == kJerryXHandleScopePool.start)
    {
      return kJerryXHandleScopePoolPrelistLast;
    }
    jerryx_handle_scope_dynamic_t *parent = dy_scope->parent;
    return (jerryx_handle_scope_t *) parent;
  }
  if (scope == kJerryXHandleScopePool.prelist)
  {
    return NULL;
  }
  return kJerryXHandleScopePool.prelist + JerryXHandleScopePrelistIdx (scope) - 1;
}


jerryx_handle_scope_t *
jerryx_handle_scope_get_child (jerryx_handle_scope_t *scope)
{
  if (!jerryx_handle_scope_is_in_prelist (scope))
  {
    jerryx_handle_scope_dynamic_t *child = ((jerryx_handle_scope_dynamic_t *) scope)->child;
    return (jerryx_handle_scope_t *) child;
  }
  if (scope == kJerryXHandleScopePoolPrelistLast)
  {
    return (jerryx_handle_scope_t *) kJerryXHandleScopePool.start;
  }
  long idx = JerryXHandleScopePrelistIdx (scope);
  if (idx < 0)
  {
    return NULL;
  }
  if ((unsigned long) idx >= kJerryXHandleScopePool.count - 1)
  {
    return NULL;
  }
  return kJerryXHandleScopePool.prelist + idx + 1;
}


jerryx_handle_scope_t *
jerryx_handle_scope_alloc (void)
{
  if (kJerryXHandleScopePool.count < JERRY_X_HANDLE_SCOPE_PRELIST_SCOPE_COUNT)
  {
    jerryx_handle_scope_t *scope = kJerryXHandleScopePool.prelist + kJerryXHandleScopePool.count;
    scope->handle_count = 0;
    scope->handle_ptr = NULL;

    kJerryXHandleScopeCurrent = scope;
    kJerryXHandleScopePool.count += 1;
    return scope;
  }

  jerryx_handle_scope_dynamic_t *scope = malloc (sizeof(jerryx_handle_scope_dynamic_t));
  scope->handle_count = 0;
  scope->handle_ptr = NULL;
  scope->child = NULL;

  if (kJerryXHandleScopePool.count != JERRY_X_HANDLE_SCOPE_PRELIST_SCOPE_COUNT)
  {
    jerryx_handle_scope_dynamic_t *dy_current = (jerryx_handle_scope_dynamic_t *) kJerryXHandleScopeCurrent;
    scope->parent = dy_current;
    dy_current->child = scope;
  }
  else
  {
    kJerryXHandleScopePool.start = scope;
  }

  kJerryXHandleScopeCurrent = (jerryx_handle_scope_t *) scope;

  kJerryXHandleScopePool.count += 1;
  return (jerryx_handle_scope_t *) scope;
}


void
jerryx_handle_scope_free (jerryx_handle_scope_t *scope)
{
  kJerryXHandleScopePool.count -= 1;

  if (!jerryx_handle_scope_is_in_prelist (scope))
  {
    jerryx_handle_scope_dynamic_t *dy_scope = (jerryx_handle_scope_dynamic_t *) scope;
    if (dy_scope == kJerryXHandleScopePool.start)
    {
      kJerryXHandleScopePool.start = dy_scope->child;
    }
    else if (dy_scope->parent != NULL)
    {
      dy_scope->parent->child = dy_scope->child;
    }
    free (dy_scope);
    return;
  }
  /**
   * Nothing to do with scopes in prelist
   */
}

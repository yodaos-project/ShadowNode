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

#include "iotjs.h"
#include "jerryscript-ext/handle-scope.h"
#include "internal/node_api_internal.h"
#include "node_api.h"

static napi_module *mod_pending;

void napi_module_register(napi_module *mod) {
  mod_pending = mod;
}

int napi_module_init_pending(jerry_value_t *exports) {
  if (mod_pending == NULL) {
    return napi_module_no_pending;
  }

  jerry_addon_register_func Init =
      (jerry_addon_register_func)mod_pending->nm_register_func;

  if (Init == NULL) {
    return napi_module_no_nm_register_func;
  }

  napi_env env = (napi_env)NULL;

  jerryx_escapable_handle_scope scope;
  jerryx_open_escapable_handle_scope(&scope);

  jerry_value_t jval_exports = jerry_create_object();
  jerryx_create_handle(jval_exports);
  napi_value nvalue_ret = (*Init)(env, jval_exports);

  if (nvalue_ret == NULL) {
    *exports = jerry_create_undefined();
  } else {
    jerry_value_t jval_ret = (jerry_value_t)(uintptr_t)nvalue_ret;
    if (jval_ret != jval_exports) {
      jerry_release_value(jval_exports);
    }
    jerryx_remove_handle(scope, jval_ret, exports);
  }

  jerryx_close_handle_scope(scope);

  mod_pending = NULL;
  return napi_module_load_ok;
}

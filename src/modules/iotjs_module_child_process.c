/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
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
#include <stdlib.h>

static void OnExit(uv_process_t* handle,
                   int64_t exit_status,
                   int term_signal) {
  printf("exit spawn %lld %d\n", exit_status, term_signal);
}

JS_FUNCTION(Spawn) {
  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());
  uv_process_t* process = (uv_process_t*)malloc(sizeof(uv_process_t));
  uv_process_options_t options;
  memset(&options, 0, sizeof(uv_process_options_t));

  options.file = "ls";
  // char args[1] = { "./" };
  // options.args = args;
  uv_stdio_container_t stdio[1];
  stdio[0].flags = UV_CREATE_PIPE | UV_READABLE_PIPE | UV_WRITABLE_PIPE;

  options.stdio = stdio;
  options.stdio_count = 1;
  options.exit_cb = OnExit;

  if (uv_spawn(loop, process, &options) != 0) {
    printf("spawn process error\n");
  }

  return jerry_create_undefined();
}

JS_FUNCTION(Kill) {
  return jerry_create_undefined();
}

jerry_value_t InitChildProcess() {
  jerry_value_t child_process = jerry_create_object();

  iotjs_jval_set_method(child_process, "spawn", Spawn);
  iotjs_jval_set_method(child_process, "kill", Kill);
  return child_process;
}

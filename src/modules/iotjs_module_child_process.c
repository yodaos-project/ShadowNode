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
#include "iotjs_objectwrap.h"
#include "iotjs_handlewrap.h"
#include "iotjs_module_pipe.h"
#include <stdlib.h>

typedef struct {
  iotjs_handlewrap_t handlewrap;
  uv_process_t handle;
  bool initialized;
} IOTJS_VALIDATED_STRUCT(iotjs_process_t);

IOTJS_DEFINE_NATIVE_HANDLE_INFO_THIS_MODULE(process);

static iotjs_process_t* iotjs_process_create(const jerry_value_t value) {
  iotjs_process_t* process = IOTJS_ALLOC(iotjs_process_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_process_t, process);
  iotjs_handlewrap_initialize(&_this->handlewrap, 
                              value,
                              (uv_handle_t*)(&_this->handle),
                              &this_module_native_info);
  return process;
}

static void iotjs_process_destroy(iotjs_process_t* process) {
  IOTJS_VALIDATED_STRUCT_DESTRUCTOR(iotjs_process_t, process);
  iotjs_handlewrap_destroy(&_this->handlewrap);
  IOTJS_RELEASE(process);
}

uv_process_t* iotjs_process_get_handle(iotjs_process_t* wrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_process_t, wrap);
  uv_handle_t* handle = iotjs_handlewrap_get_uv_handle(&_this->handlewrap);
  return (uv_process_t*)handle;
}

jerry_value_t iotjs_process_get_jobject(iotjs_process_t* wrap) {
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_process_t, wrap);
  return iotjs_handlewrap_jobject(&_this->handlewrap);
}

iotjs_process_t* iotjs_process_from_handle(uv_process_t* process) {
  uv_handle_t* handle = (uv_handle_t*)(process);
  iotjs_handlewrap_t* handlewrap = iotjs_handlewrap_from_handle(handle);
  iotjs_process_t* wrap = (iotjs_process_t*)handlewrap;
  IOTJS_ASSERT(iotjs_process_get_handle(wrap) == process);
  return wrap;
}

static void iotjs_process_onexit(uv_process_t* handle,
                                 int64_t exit_status,
                                 int term_signal) {
  iotjs_process_t* wrap = iotjs_process_from_handle(handle);
  jerry_value_t jthis = iotjs_process_get_jobject(wrap);
  IOTJS_ASSERT(wrap != NULL);

  iotjs_jargs_t jargs = iotjs_jargs_create(2);
  jerry_value_t status = jerry_create_number(exit_status);
  jerry_value_t signal = jerry_create_number(term_signal);
  iotjs_jargs_append_jval(&jargs, status);
  iotjs_jargs_append_jval(&jargs, signal);

  jerry_value_t fn = iotjs_jval_get_property(jthis, "onexit");
  if (jerry_value_is_function(fn)) {
    iotjs_make_callback(fn, jthis, &jargs);
  }
  jerry_release_value(status);
  jerry_release_value(signal);
  jerry_release_value(fn);
}

JS_FUNCTION(ProcessConstructor) {
  DJS_CHECK_THIS();
  const jerry_value_t self = JS_GET_THIS();
  iotjs_process_t* process = iotjs_process_create(self);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_process_t, process);
  _this->initialized = true;

  return jerry_create_undefined();
}

static void iotjs_process_parse_args_opts(jerry_value_t js_options,
                                   uv_process_options_t* options) {
  jerry_value_t jarr = iotjs_jval_get_property(js_options, "args");
  uint32_t len = jerry_get_array_length(jarr);
  if (len > 0) {
    options->args = (char**)malloc(sizeof(char*) * len + 1);
    for (uint32_t i = 0; i < len; i++) {
      jerry_value_t jval = iotjs_jval_get_property_by_index(jarr, i);
      iotjs_string_t str = iotjs_jval_as_string(jval);
      options->args[i] = (char*)iotjs_string_data(&str);
    }
    options->args[len] = NULL;
  }
  jerry_release_value(jarr);
}

static void iotjs_process_parse_envs_opts(jerry_value_t js_options,
                                   uv_process_options_t* options) {
  jerry_value_t jarr = iotjs_jval_get_property(js_options, "envPairs");
  uint32_t len = jerry_get_array_length(jarr);
  if (len > 0) {
    options->env = (char**)malloc(sizeof(char*) * len + 1);
    for (uint32_t i = 0; i < len; i++) {
      jerry_value_t jval = iotjs_jval_get_property_by_index(jarr, i);
      iotjs_string_t str = iotjs_jval_as_string(jval);
      options->env[i] = (char*)iotjs_string_data(&str);
    }
    options->env[len] = NULL;
  }
  jerry_release_value(jarr);
}

static void iotjs_process_parse_stdio_opts(jerry_value_t js_options,
                                           uv_process_options_t* options) {
  jerry_value_t stdios = iotjs_jval_get_property(js_options, "stdio");
  uint32_t len = jerry_get_array_length(stdios);

  options->stdio = (uv_stdio_container_t*)malloc(len);
  options->stdio_count = (int)len;

  for (uint32_t i = 0; i < len; i++) {
    jerry_value_t stdio = iotjs_jval_get_property_by_index(stdios, i);
    jerry_value_t jtype = iotjs_jval_get_property(stdio, "type");
    iotjs_string_t jtype_str = iotjs_jval_as_string(jtype);
    const char* type = (const char*)iotjs_string_data(&jtype_str);

    if (!strcmp(type, "ignore")) {
      options->stdio[i].flags = UV_IGNORE;
    } else if (!strcmp(type, "pipe")) {
      jerry_value_t jhandle = iotjs_jval_get_property(stdio, "handle");
      iotjs_pipewrap_t* wrap = iotjs_pipewrap_from_jobject(jhandle);
      options->stdio[i].data.stream = (uv_stream_t*)iotjs_pipewrap_get_handle(wrap);
      options->stdio[i].flags = (uv_stdio_flags)(
        UV_CREATE_PIPE | UV_READABLE_PIPE | UV_WRITABLE_PIPE);
      jerry_release_value(jhandle);
    } else if (!strcmp(type, "wrap")) {
      // TODO(Yorkie): to be done
      options->stdio[i].flags = UV_IGNORE;
    } else {
      jerry_value_t fd = iotjs_jval_get_property(stdio, "fd");
      options->stdio[i].flags = UV_INHERIT_FD;
      options->stdio[i].data.fd = jerry_get_number_value(fd);
      jerry_release_value(fd);
    }
    jerry_release_value(jtype);
    jerry_release_value(stdio);
  }
  jerry_release_value(stdios);
}

JS_FUNCTION(ProcessSpawn) {
  JS_DECLARE_THIS_PTR(process, process);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_process_t, process);

  jerry_value_t js_options = jerry_value_to_object(jargv[0]);
  uv_process_options_t options;
  memset(&options, 0, sizeof(uv_process_options_t));
  options.exit_cb = iotjs_process_onexit;

#define IOTJS_PROCESS_SET_XID(name, type) do {                                \
  jerry_value_t val = iotjs_jval_get_property(js_options, #name);             \
  if (!jerry_value_is_undefined(val) && !jerry_value_is_null(val)) {          \
    options.flags |= type;                                                    \
    options.name = (uv_##name##_t)jerry_get_number_value(val);                \
  }                                                                           \
  jerry_release_value(val);                                                   \
} while (0)

#define IOTJS_PROCESS_SET_STRING(name) do {                                   \
  jerry_value_t val = iotjs_jval_get_property(js_options, #name);             \
  if (!jerry_value_is_undefined(val) && !jerry_value_is_null(val)) {          \
    iotjs_string_t str = iotjs_jval_as_string(val);                           \
    options.name = iotjs_string_data(&str);                                   \
  }                                                                           \
  jerry_release_value(val);                                                   \
} while (0)

  IOTJS_PROCESS_SET_XID(uid, UV_PROCESS_SETUID);
  IOTJS_PROCESS_SET_XID(gid, UV_PROCESS_SETGID);
  IOTJS_PROCESS_SET_STRING(file);
  IOTJS_PROCESS_SET_STRING(cwd);
#undef IOTJS_PROCESS_SET_XID
#undef IOTJS_PROCESS_SET_STRING

  iotjs_process_parse_args_opts(js_options, &options);
  iotjs_process_parse_envs_opts(js_options, &options);
  iotjs_process_parse_stdio_opts(js_options, &options);
  jerry_release_value(js_options);

  _this->initialized = true;
  uv_loop_t* loop = iotjs_environment_loop(iotjs_environment_get());

  printf("before: %s\n", options.env[0]);
  int err = uv_spawn(loop, &_this->handle, &options);
  if (err == 0) {
    jerry_value_t pid = jerry_create_number(_this->handle.pid);
    iotjs_jval_set_property_jval(jthis, "pid", pid);
    jerry_release_value(pid);
  }

  if (options.args != NULL) {
    free(options.args);
  }

  if (options.env != NULL) {
    free(options.env);
  }

  return jerry_create_number(0);
}

JS_FUNCTION(ProcessKill) {
  return jerry_create_undefined();
}

// Socket close result handler.
void iotjs_process_after_close(uv_handle_t* handle) {
  iotjs_handlewrap_t* wrap = iotjs_handlewrap_from_handle(handle);
  jerry_value_t jthis = iotjs_handlewrap_jobject(wrap);

  // callback function.
  jerry_value_t jcallback =
      iotjs_jval_get_property(jthis, "onclose");
  if (jerry_value_is_function(jcallback)) {
    iotjs_make_callback(jcallback, 
                        jerry_create_undefined(),
                        iotjs_jargs_get_empty());
  }
  jerry_release_value(jcallback);
}

JS_FUNCTION(ProcessClose) {
  JS_DECLARE_THIS_PTR(handlewrap, wrap);

  // close uv handle, `AfterClose` will be called after socket closed.
  iotjs_handlewrap_close(wrap, iotjs_process_after_close);
  return jerry_create_undefined();
}

jerry_value_t InitChildProcess() {
  jerry_value_t process = jerry_create_object();
  jerry_value_t processConstructor =
      jerry_create_external_function(ProcessConstructor);
  iotjs_jval_set_property_jval(process, "Process", processConstructor);

  jerry_value_t proto = jerry_create_object();
  iotjs_jval_set_method(proto, "spawn", ProcessSpawn);
  iotjs_jval_set_method(proto, "kill", ProcessKill);
  iotjs_jval_set_method(proto, "close", ProcessClose);
  iotjs_jval_set_property_jval(processConstructor, "prototype", proto);

  jerry_release_value(proto);
  jerry_release_value(processConstructor);
  return process;
}

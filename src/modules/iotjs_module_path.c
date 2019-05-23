#include "iotjs_def.h"
#include <libgen.h>

JS_FUNCTION(Basename) {
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t jfilename = JS_GET_ARG(0, string);
  const char* filename = iotjs_string_data(&jfilename);
  const char* _basename = basename((char*)filename);
  iotjs_string_destroy(&jfilename);

  return jerry_create_string_from_utf8((jerry_char_t*)_basename);
}

JS_FUNCTION(Dirname) {
  DJS_CHECK_ARGS(1, string);

  iotjs_string_t jfilename = JS_GET_ARG(0, string);
  const char* filename = iotjs_string_data(&jfilename);
  const char* _dirname = dirname((char*)filename);
  iotjs_string_destroy(&jfilename);

  return jerry_create_string_from_utf8((jerry_char_t*)_dirname);
}

jerry_value_t InitPath() {
  jerry_value_t path = jerry_create_object();
  iotjs_jval_set_method(path, "basename", Basename);
  iotjs_jval_set_method(path, "dirname", Dirname);
  return path;
}

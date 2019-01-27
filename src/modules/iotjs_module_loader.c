#include "iotjs_def.h"

static resolve_file_path(char** filepath, const char* id, char* root) {
  char* mod_path = (char*)id;
  if (root != NULL) {

  }
}

JS_FUNCTION(Resolve) {
  iotjs_string_t id = JS_GET_ARG(0, string);
  jerry_value_t jparent = jargv[1];

  char filepath[255];
  char* id_str = (char*)iotjs_string_data(&id);

  if (id_str[0] == '/') {
    printf("absolute resolving...\n");
    resolve_file_path(&filepath, id_str, NULL);
  } else if (jerry_value_is_null(jparent)) {
    printf("parent is undefined...\n");
  } else if (id_str[0] == '.') {
    printf("relative resolving...\n");
  } else {
    printf("other branches\n");
  }

  return jerry_create_undefined();
}

JS_FUNCTION(SetSearchPaths) {
  return jerry_create_undefined();
}

jerry_value_t InitLoader() {
  jerry_value_t loader = jerry_create_object();
  iotjs_jval_set_method(loader, "resolve", Resolve);
  iotjs_jval_set_method(loader, "setSearchPaths", SetSearchPaths);
  return loader;
}

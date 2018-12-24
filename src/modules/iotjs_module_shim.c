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
#include "jerryscript.h"
#include "math.h"

static double PRECISION = 1e-15;
static double MAX_INTEGER_NUM = 9007199254740991;

JS_FUNCTION(NumberIsInteger) {
  if (jargc == 0) {
    return jerry_create_boolean(FALSE);
  }

  if (!jerry_value_is_number(jargv[0])) {
    return jerry_create_boolean(FALSE);
  }

  double x = iotjs_jval_as_number(jargv[0]);
  if (isnan(x)) {
    return jerry_create_boolean(FALSE);
  }

  if (!(fabs(x - floor(x)) <= PRECISION)) {
    return jerry_create_boolean(FALSE);
  }

  return jerry_create_boolean(TRUE);
}

JS_FUNCTION(NumberIsSafeInteger) {
  jerry_value_t isnum = NumberIsInteger(jfunc, jthis, jargv, jargc);
  if (!iotjs_jval_as_boolean(isnum)) {
    return jerry_create_boolean(FALSE);
  }

  double x = iotjs_jval_as_number(jargv[0]);
  return jerry_create_boolean(fabs(x) <= MAX_INTEGER_NUM);
}

JS_FUNCTION(NumberIsFinite) {
  if (jargc == 0) {
    return jerry_create_boolean(FALSE);
  }

  if (!jerry_value_is_number(jargv[0])) {
    return jerry_create_boolean(FALSE);
  }

  double x = iotjs_jval_as_number(jargv[0]);
  return jerry_create_boolean(isfinite(x));
}

JS_FUNCTION(NumberIsNaN) {
  if (jargc == 0) {
    return jerry_create_boolean(FALSE);
  }

  if (!jerry_value_is_number(jargv[0])) {
    return jerry_create_boolean(FALSE);
  }

  double x = iotjs_jval_as_number(jargv[0]);
  return jerry_create_boolean(isnan(x));
}

jerry_value_t InitShim() {
  jerry_value_t shim = jerry_create_object();
  iotjs_jval_set_method(shim, "NumberIsInteger", NumberIsInteger);
  iotjs_jval_set_method(shim, "NumberIsSafeInteger", NumberIsSafeInteger);
  iotjs_jval_set_method(shim, "NumberIsFinite", NumberIsFinite);
  iotjs_jval_set_method(shim, "NumberIsNaN", NumberIsNaN);
  return shim;
}

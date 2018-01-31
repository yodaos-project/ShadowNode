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


var util = require('util');


function Console() {
  // Empty
}

function stdout(text) {
  return native.stdout(text);
}

function stderr(text) {
  return native.stderr(text);
}

Console.prototype.log =
Console.prototype.debug =
Console.prototype.info = function() {
  stdout(util.format.apply(this, arguments) + '\n');
};

Console.prototype.warn =
Console.prototype.error = function() {
  stderr(util.format.apply(this, arguments) + '\n');
};

var console = new Console();

module.exports = {
  log: console.log.bind(console),
  debug: console.debug.bind(console),
  info: console.info.bind(console),
  warn: console.warn.bind(console),
  error: console.error.bind(console),
  Console: Console,
  _stdout: stdout,
  _stderr: stderr,
};

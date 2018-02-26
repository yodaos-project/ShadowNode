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
  var prop = {
    writable: true,
    enumerable: false,
    configurable: true
  };
  prop.value = stdout;
  Object.defineProperty(this, '_stdout', prop);
  prop.value = stderr;
  Object.defineProperty(this, '_stderr', prop);
  prop.value = {};
  Object.defineProperty(this, '_times', prop);

  var keys = Object.keys(Console.prototype);
  for (var v = 0; v < keys.length; v++) {
    var k = keys[v];
    this[k] = this[k].bind(this);
  }
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

Console.prototype.time = function(label) {
  this._times[String(label)] = process.hrtime();
};

Console.prototype.timeEnd = function(label) {
  label = String(label);

  var time = this._times[label];
  if (time == null) {
    process.emitWarning('No such label ' + label + ' for console.timeEnd()');
    return;
  }

  var duration = process.hrtime(time);
  var ms = duration[0] * 1000 + duration[1] / 1e6;
  this._times[label] = undefined;
};

var console = new Console();

module.exports = console;
module.exports.Console = Console;

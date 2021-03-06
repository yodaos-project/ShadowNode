/* Copyright 2017-present Samsung Electronics Co., Ltd. and other contributors
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
'use strict';

var assert = require('assert');
var fs = require('fs');

var filePath = process.cwd() + '/resources';

try {
  process.readSource(filePath);
} catch (e) {
  assert.strictEqual(fs.existsSync(filePath), true);
  assert.strictEqual(e.name, 'Error');
  assert.strictEqual(e.message, 'ReadSource error, not a regular file');
}

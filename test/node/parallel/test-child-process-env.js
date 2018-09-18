// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

'use strict';
var common = require('../common');
var assert = require('assert');
var os = require('os');

var spawn = require('child_process').spawn;
var env = {
  'HELLO': 'WORLD',
  'UNDEFINED': undefined,
  'NULL': null,
  'EMPTY': ''
};
Object.setPrototypeOf(env, {
  'FOO': 'BAR'
});

var child = spawn('/usr/bin/env', [], Object.assign({}, process.env, env));

var response = '';
child.stdout.setEncoding('utf8');
child.stdout.on('data', function(chunk) {
  console.log(`stdout: ${chunk}`);
  response += chunk;
});

process.on('exit', function() {
  assert.ok(response.search('HELLO=WORLD') > -1);
  assert.ok(response.search('FOO=BAR') > -1);
  assert.ok(response.search('UNDEFINED=undefined') > -1);
  assert.ok(response.search('NULL=null') > -1);
  assert.ok(response.search(`EMPTY=${os.EOL}`) > -1);
  console.log('done');
});

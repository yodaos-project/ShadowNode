'use strict';

var assert = require('assert');
var addon = require(`./build/Release/binding`);

var fn = addon();
assert.strictEqual(fn(), 'hello world'); // 'hello world'

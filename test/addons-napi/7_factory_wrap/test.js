'use strict';

var assert = require('assert');
var createObject = require(`./build/Release/binding`);

var obj = createObject(10);
assert.strictEqual(obj.plusOne(), 11);
assert.strictEqual(obj.plusOne(), 12);
assert.strictEqual(obj.plusOne(), 13);

var obj2 = createObject(20);
assert.strictEqual(obj2.plusOne(), 21);
assert.strictEqual(obj2.plusOne(), 22);
assert.strictEqual(obj2.plusOne(), 23);

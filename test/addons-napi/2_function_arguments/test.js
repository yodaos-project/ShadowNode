'use strict';

var assert = require('assert');
var addon = require(`./build/Release/binding`);

assert.strictEqual(addon.add(3, 5), 8);

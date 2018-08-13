'use strict';

var assert = require('assert');
var addon = require(`./build/Release/binding.node`);

assert.strictEqual(addon.add(3, 5), 8);

'use strict';

var assert = require('assert');

var test_globals = require(`./build/Release/test_general`);

assert.strictEqual(test_globals.getUndefined(), undefined);
assert.strictEqual(test_globals.getNull(), null);

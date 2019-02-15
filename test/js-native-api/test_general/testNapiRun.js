'use strict';

var common = require('../../common');
var assert = require('assert');

// `addon` is referenced through the eval expression in testFile
var addon = require(`./build/Release/test_general.node`);

var testCase = '(41.92 + 0.08);';
var expected = 42;
var actual = addon.testNapiRun(testCase);

assert.strictEqual(actual, expected);
assert.throws(() => addon.testNapiRun({ abc: 'def' }), /string was expected/);

'use strict';
// Flags: --expose-gc

var common = require('../../common');
var assert = require('assert');
var test_exception = require(`./build/Release/test_exception.node`);

// Make sure that exceptions that occur during finalization are propagated.
function testFinalize(binding) {
  var x = test_exception[binding]();
  x = null;
  assert.throws(() => { global.gc(); }, /Error during Finalize/);

  // To assuage the linter's concerns.
  (function() {})(x);
}
testFinalize('createExternalBuffer');

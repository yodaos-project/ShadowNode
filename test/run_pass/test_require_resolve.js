'use strict';

var assert = require('assert');
var fixtures = require('../common/fixtures');

assert.strictEqual(
  require.resolve('test_util').toLowerCase(),
  fixtures.path('./test/run_pass/test_util.js').toLowerCase());

assert.strictEqual(
  require.resolve('os').toLowerCase(), 'os');

[1, false, null, undefined, {}].forEach((value) => {
  assert.throws(function() {
    require.resolve(value);
  }, TypeError);
});

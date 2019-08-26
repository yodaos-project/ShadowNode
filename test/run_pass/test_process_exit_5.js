'use strict';
var assert = require('assert');
var common = require('../common');
var code = 233;

var fn = (c) => {
  assert.strictEqual(code, c);
};

process.on('exit', function(c) {
  fn(c);
});

fn = common.mustCall(fn);

process.exit(code);

assert.fail();

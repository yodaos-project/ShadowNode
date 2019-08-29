'use strict';
var assert = require('assert');
var code = 0;
var count = 0;

process.on('exit', function(c) {
  assert.strictEqual(++count, 1);
  process.exit(c);
});

process.exit(code);

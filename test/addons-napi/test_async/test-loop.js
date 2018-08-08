'use strict';

var assert = require('assert');
var test_async = require(`./build/Release/test_async`);
var iterations = 500;

let x = 0;
var workDone = common.mustCall((status) => {
  assert.strictEqual(status, 0, 'Work completed successfully');
  if (++x < iterations) {
    setImmediate(() => test_async.DoRepeatedWork(workDone));
  }
}, iterations);
test_async.DoRepeatedWork(workDone);

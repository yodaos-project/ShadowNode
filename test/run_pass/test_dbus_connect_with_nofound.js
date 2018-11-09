'use strict';

var assert = require('assert');
var dbus = require('dbus');

var start = Date.now();
try {
  dbus.getBus();
} catch (err) {
  console.log(err.message);
  assert.equal(err.message,
    'Failed to connect to socket /invalid: No such file or directory');
}

var diff = Date.now() - start;
console.log(`consumes ${diff}ms`);
assert.equal(diff > 1000, true);

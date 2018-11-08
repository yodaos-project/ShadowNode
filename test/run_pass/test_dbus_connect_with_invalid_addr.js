'use strict';

var assert = require('assert');
var dbus = require('dbus');
var start = Date.now();

try {
  dbus.getBus();
} catch (err) {
  console.log(err.message);
  assert.equal(err.message, 'Address does not contain a colon');
}
assert.equal(Date.now() - start < 100, true);

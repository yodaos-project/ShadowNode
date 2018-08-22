'use strict';

var assert = require('assert');
var dbus = require('dbus');

assert.equal(dbus.Define(String), 's');
assert.equal(dbus.Define(Number), 'd');
assert.equal(dbus.Define(Boolean), 'b');
assert.equal(dbus.Define(Array), 'av');
assert.equal(dbus.Define(Object), 'a{sv}');


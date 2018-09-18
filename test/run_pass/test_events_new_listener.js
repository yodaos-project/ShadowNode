var EventEmitter = require('events').EventEmitter;
var assert = require('assert');
var common = require('../common');

var bus = new EventEmitter();

var listener = function onEvent () {};

bus.on('newListener', common.mustCall((event, fn) => {
  assert.strictEqual(event, 'foobar');
  assert.strictEqual(fn, listener);
}));

bus.on('foobar', listener);

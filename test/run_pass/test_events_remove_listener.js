var EventEmitter = require('events').EventEmitter;
var assert = require('assert');
var common = require('../common');

var bus = new EventEmitter();

function noop() {}

bus.on('foobar', noop);

bus.on('removeListener', common.mustCall((event, fn) => {
  assert.strictEqual(event, 'foobar');
  assert.strictEqual(fn, noop);
}));

bus.removeListener('foobar', noop);

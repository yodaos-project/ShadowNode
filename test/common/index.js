'use strict';
var assert = require('assert');
var os = require('os');
var mustCallChecks = [];

var isWindows = process.platform === 'win32';
var isAIX = process.platform === 'aix';
var isLinuxPPCBE = (process.platform === 'linux') &&
                     (process.arch === 'ppc64') &&
                     (os.endianness() === 'BE');
var isSunOS = process.platform === 'sunos';
var isFreeBSD = process.platform === 'freebsd';
var isOpenBSD = process.platform === 'openbsd';
var isLinux = process.platform === 'linux';
var isOSX = process.platform === 'darwin';

var is = {
  number: function(value, key) {
    // TODO: assert(!Number.isNaN(value), `${key} should not be NaN`);
    // Number.isNaN() should be supported.
    assert.strictEqual(typeof value, 'number');
  },
  string: function(value) { assert.strictEqual(typeof value, 'string'); },
  array: function(value) { assert.ok(Array.isArray(value)); },
  object: function(value) {
    assert.strictEqual(typeof value, 'object');
    assert.notStrictEqual(value, null);
  }
};

function mustCall(fn, criteria) {
  if (typeof fn === 'number') {
    criteria = fn;
    fn = noop;
  } else if (fn === undefined) {
    fn = noop;
  }
  if (criteria === undefined) {
    criteria = 1;
  }

  if (typeof criteria !== 'number')
    throw new TypeError(`Invalid value: ${criteria}`);

  var context = {
    expect: criteria,
    actual: 0,
    stack: (new Error()).stack,
    name: fn.name || '<anonymous>'
  };

  if (mustCallChecks.length === 0) process.on('exit', runCallChecks);

  mustCallChecks.push(context);

  return function() {
    ++context.actual;
    return fn.apply(this, arguments);
  };
}

function noop() {}

function runCallChecks() {
  mustCallChecks.forEach((it) => {
    assert.strictEqual(
      it.actual,
      it.expect,
      `Expect function ${it.name} been called ${it.expect} times, \
got ${it.actual}
${it.stack}`);
  });
}

function expectsError(fn, exact) {
  // TODO:rigorous assert need!
  function innerFn(error) {
    return error instanceof Error;
  }

  if (fn) {
    assert.throws(fn, innerFn, exact);
    return;
  }

  return mustCall(innerFn, exact);
}

module.exports = {
  isWindows: isWindows,
  isAIX: isAIX,
  isLinuxPPCBE: isLinuxPPCBE,
  isSunOS: isSunOS,
  isFreeBSD: isFreeBSD,
  isOpenBSD: isOpenBSD,
  isLinux: isLinux,
  isOSX: isOSX,

  is: is,

  mustCall: mustCall,
  expectsError: expectsError
};

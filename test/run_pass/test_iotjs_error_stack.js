var assert = require('assert');

var error = getError();
assert(typeof error.stack === 'string');
assert(error.stack.match(/^    at getError/));

var plain = getPlainObject();
assert(typeof plain.stack === 'string');
assert(error.stack.match(/^    at getPlainObject/));

function getError() {
  return new Error('foobar');
}

function getPlainObject() {
  var e = {};
  Error.captureStackTrace(e);
  return e;
}

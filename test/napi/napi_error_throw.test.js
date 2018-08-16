var assert = require('assert');
var test = require('./build/Release/napi_error.node');

try {
  test.ThrowError();
  assert.fail('fail path');
} catch (err) {
  assert(err != null);
  assert.strictEqual(err.message, 'foobar')
}

try {
  test.ThrowCreatedError();
  assert.fail('fail path');
} catch (err) {
  assert(err != null);
  assert.strictEqual(err.message, 'foobar')
}

var err = test.GetError();
assert(err != null);
assert.strictEqual(err.message, 'foobar')

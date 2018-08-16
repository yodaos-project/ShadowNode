var assert = require('assert');
var test = require('./build/Release/napi_error.node');

try {
  test.RethrowError();
  assert.fail('fail path');
} catch (err) {
  assert(err != null);
  assert.strictEqual(err.message, 'foobar')
}

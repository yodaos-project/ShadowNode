var assert = require('assert')
var napi_test = require('./napi_test')

assert(napi_test !== null)
assert.strictEqual(typeof napi_test, 'object')
assert.strictEqual(napi_test.id, 321)

assert.strictEqual(typeof napi_test.sayHello, 'function')
assert.strictEqual(napi_test.sayHello(), 'Hello')

assert.strictEqual(typeof napi_test.sayError, 'function')

var error
try {
  napi_test.sayError()
} catch (err) {
  error = err
}
assert(error instanceof Error)
assert.strictEqual(error.code, 'foo')
assert.strictEqual(error.message, 'bar')

var lhs = {}
assert.strictEqual(napi_test.strictEquals(lhs, lhs), lhs === lhs);
assert.strictEqual(napi_test.instanceof(lhs, Object), lhs instanceof Object);

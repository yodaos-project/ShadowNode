var assert = require('assert')
var napi_test = require('./napi_test.node')

assert(napi_test !== null)
assert.strictEqual(typeof napi_test, 'object')
assert.strictEqual(napi_test.id, 321)

assert.strictEqual(typeof napi_test.sayHello, 'function')
assert.strictEqual(napi_test.sayHello(), 'Hello')

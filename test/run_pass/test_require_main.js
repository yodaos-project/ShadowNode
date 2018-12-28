'use strict';

var assert = require('assert');

assert.strictEqual(require.main === module, true);
require('./test_require_main_foo');

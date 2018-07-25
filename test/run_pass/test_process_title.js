'use strict';

var assert = require('assert');
assert.equal(/(\/)?iotjs$/.test(process.title), true);

process.title = 'foobar';
assert.equal(process.title, 'foobar');

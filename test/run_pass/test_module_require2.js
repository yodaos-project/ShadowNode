'use strict';

var assert = require('assert');
var dir = process.cwd() + '/run_pass/require2';

var foobar = require('foobar');
assert.equal(foobar, 'foobar');

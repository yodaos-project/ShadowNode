'use strict';

console.log('', process.env)
var assert = require('assert');
var foobar = require('foobar');
assert.equal(foobar, 'foobar');

var test = require('foobar/test');
assert.equal(test, 'test');
'use strict';

var assert = require('assert');

// Testing api calls for a varructor that defines properties
var TestConstructor =
    require(`./build/Release/test_varructor_name.node`);
assert.strictEqual(TestConstructor.name, 'MyObject');

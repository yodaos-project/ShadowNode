'use strict';


var storeEnv = require(`./build/Release/store_env`);
var compareEnv = require(`./build/Release/compare_env`);
var assert = require('assert');

assert.strictEqual(compareEnv(storeEnv), true,
                   'N-API environment pointers in two different modules have ' +
                   'the same value');

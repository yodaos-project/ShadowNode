'use strict';


var addon = require(`./build/Release/test_general`);
var assert = require('assert');

addon.createNapiError();
assert(addon.testNapiErrorCleanup(), 'napi_status cleaned up for second call');

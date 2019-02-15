'use strict';

var common = require('../../common');
var test_general = require(`./build/Release/test_general.node`);
var assert = require('assert');

var [ major, minor, patch, release ] = test_general.testGetNodeVersion();
assert.strictEqual(process.version.split('-')[0],
                   `v${major}.${minor}.${patch}`);
assert.strictEqual(release, process.release.name);

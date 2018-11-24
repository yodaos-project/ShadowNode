'use strict';

var assert = require('assert');
var common = require('../common');
var os = require('os');

var hostname = os.hostname();
common.is.string(hostname);
assert.ok(hostname.length > 0);

var uptime = os.uptime();
common.is.number(uptime);
assert.ok(uptime > 0);

var release = os.release();
common.is.string(release);
assert.ok(release.length > 0);

var platform = os.platform();
common.is.string(platform);
assert.ok(platform.length > 0);

var EOL = os.EOL;
if (common.isWindows) {
  assert.strictEqual(EOL, '\r\n');
} else {
  assert.strictEqual(EOL, '\n');
}

var interfaces = os.networkInterfaces();
if (os.platform() === 'darwin') {
    var actual = interfaces.lo0.filter(function(e){
      return e.address === '127.0.0.1' &&
              e.netmask === '255.0.0.0' &&
               e.family === 'IPv4';
    });
    
    var expected = [{
      address: '127.0.0.1',
      netmask: '255.0.0.0',
      family: 'IPv4',
      broadcast: "127.0.0.1",
      mac: '00:00:00:00:00:00'
    }];

    assert.deepStrictEqual(actual, expected);
}

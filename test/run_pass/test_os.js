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

var family = {
  IPv4: 1,
  IPv6: 1
};

var net_info = os.networkInterfaces();

for (var name in net_info) {
  assert(net_info[name].length);
  net_info[name].forEach(net => {
    assert.equal(typeof net.address, 'string');
    assert.equal(typeof net.netmask, 'string');
    assert.equal(family[net.family], 1);
    assert.equal(typeof net.broadcast, 'string');
    assert.equal(typeof net.mac, 'string');
  });
}

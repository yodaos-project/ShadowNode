'use strict';

var assert = require('assert');
var os = require('os');

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

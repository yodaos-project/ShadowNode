'use strict';

exports.hostname = function() {
  return native.getHostname();
};

exports.uptime = function() {
  return native.getUptime();
};

exports.totalmem = function() {
  return native.getTotalMem();
};

exports.freemem = function() {
  return native.getFreeMem();
};

exports.platform = function() {
  return process.platform;
};

exports.release = function() {
  return native._getOSRelease();
};

exports.networkInterfaces = function() {
  var list = native.getInterfaceAddresses();
  var interfaces = {};
  for (var i = 0; i < list.length; i++) {
    var item = list[i];
    var name = item.name;
    if (!interfaces[name]) {
      interfaces[name] = [];
    }
    interfaces[name].push({
      address: item.address,
      netmask: item.netmask,
      family: item.family,
      broadcast: item.broadcast,
      mac: item.mac,
    });
  }
  return interfaces;
};

Object.defineProperty(exports, 'EOL', {
  get: function() {
    return '\n';
  }
});

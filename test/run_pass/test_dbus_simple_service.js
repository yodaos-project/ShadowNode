'use strict';

var assert = require('assert');
var dbus = require('dbus');

var myservice = dbus.registerService('session', 'org.myservice');
var myobject = myservice.createObject('/org/myobject');
var myiface = myobject.createInterface('test.dbus.myservice.Interface1');

myiface.addMethod('test', { out: ['s'] }, (callback) => {
  callback(null, 'result');
});
myiface.update();
// myiface

var client = dbus.getBus('session');
client.getInterface('org.myservice', '/org/myobject', 'test.dbus.myservice.Interface1', (err, iface) => {
  assert.equal(typeof iface.test, 'function');
  iface.test((err, result) => {
    assert.equal(result, 'result');
    client.destroy();
  });
});
'use strict';

var assert = require('assert');
var dbus = require('dbus');

var myservice = dbus.registerService('session', 'org.myservice');
var myobject = myservice.createObject('/org/myobject');
var myiface = myobject.createInterface('test.dbus.myservice.Interface1');

myiface.addMethod('test', {
  out: [dbus.Define(String)] }, (cb) => {
  cb(null, 'simple call');
});
myiface.addMethod('testWithArgs', {
  in: [dbus.Define(String)],
  out: [dbus.Define(String)] }, (num, cb) => {
  cb(null, num + '!!!');
});
myiface.update();

var bus = dbus.getBus();
var plan = 0;
// start to call
bus.callMethod(
  'org.myservice', 
  '/org/myobject', 
  'test.dbus.myservice.Interface1', 'test', '', [], (err, res) => {
  assert.equal(err, null);
  assert.equal(res, 'simple call');
  plan += 1;
});

bus.callMethod(
  'org.myservice', 
  '/org/myobject', 
  'test.dbus.myservice.Interface1', 'testWithArgs', 's', ['foobar'], (err, res) => {
  assert.equal(err, null);
  assert.equal(res, 'foobar!!!');
  plan += 1;
});

setTimeout(function() {
  assert.equal(plan, 2);
  bus.destroy();
}, 500);
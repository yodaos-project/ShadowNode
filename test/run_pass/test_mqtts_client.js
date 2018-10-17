'use strict';

var testHost = 'mqtts://test.mosquitto.org:8883';
var mqtt = require('mqtt');
var assert = require('assert');
var common = require('../common')

var disconnected = false;
var client = mqtt.connect(testHost, {
  reconnectPeriod: -1
});
client.once('connect', common.mustCall(function() {
  client.disconnect();
  assert.equal(client._keepAliveTimer, null);
  assert.equal(client._keepAliveTimeout, null);
  disconnected = true;
}));
client.once('offline', common.mustCall(function() {
  assert.ok(disconnected);
}));

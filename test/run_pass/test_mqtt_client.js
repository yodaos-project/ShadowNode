'use strict';

var testHost = 'mqtt://test.mosquitto.org:1883';
var mqtt = require('mqtt');
var assert = require('assert');

var disconnected = false;
var client = mqtt.connect(testHost, {
  reconnectPeriod: -1
});
client.once('connect', function() {
  client.disconnect();
  assert.equal(client._keepAliveTimer, null);
  assert.equal(client._keepAliveTimeout, null);
  disconnected = true;
});
client.once('offline', function() {
  assert.ok(disconnected);
});

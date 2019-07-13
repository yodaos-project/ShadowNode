'use strict';
var mqtt = require('mqtt');
var assert = require('assert');
/*
  To reproduce connect timeout need the peer drops TCP SYN packets,
  so connect to an existing host but to a port that is blocked by the firewall.
*/
var timeoutHost = 'whatever://google.com:81';
var mqttHost = 'mqtts://localhost:9088';
var common = require('../common');

var timeoutClient = mqtt.connect(timeoutHost, {
  connectTimeout: 1000,
  reconnectPeriod: -1
});
timeoutClient.once('connect', function() {
  assert(0, 'should not call this callback');
});
timeoutClient.once('error', common.mustCall(function(err) {
  assert.strictEqual(err.message, 'connect timeout');
  timeoutClient.disconnect();
}));
timeoutClient.once('close', common.mustCall(function() {
  console.log(`connection ${timeoutHost} closed`);
}));

var client = mqtt.connect(mqttHost, {
  connectTimeout: 1000,
  reconnectPeriod: -1
});
client.once('connect', common.mustCall(function() {
  assert.strictEqual(client._connectTimer, null);
  client.disconnect();
}));

client.once('close', common.mustCall(function() {
  console.log(`connection ${mqttHost} closed`);
}));

'use strict';

var mqttHost = 'mqtt://test.mosquitto.org:1883';
var mqttsHost = 'mqtts://test.mosquitto.org:8883';
var mqtt = require('mqtt');
var assert = require('assert');
var common = require('../common');

function test(testHost) {
  var client = mqtt.connect(testHost, {
    reconnectPeriod: -1
  });
  // testHost may not be able to connect, so don't use mustCall here
  client.once('connect', common.mustCall(function() {
    // trigger the close event
    client.once('offline', common.mustCall(function() {
      assert.ok(!client.connected);
      assert.ok(!client._isTCPConnected);
    }));
    client.disconnect();
    assert.equal(client._keepAliveTimer, null);
    assert.equal(client._keepAliveTimeout, null);
  }));
  //the close event will be triggered whether testHost is connected or not
  client.once('close', common.mustCall(function() {
    assert.ok(!client.connected);
    assert.ok(!client._isTCPConnected);
  }));
}

test(mqttHost);
test(mqttsHost);

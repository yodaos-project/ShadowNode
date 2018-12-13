'use strict';
var mqtt = require('mqtt');
var assert = require('assert');
var mqttHost = 'mqtt://localhost:9080';
var mqttsHost = 'mqtts://localhost:9088';
var common = require('../common');

function test(testHost) {
  var client = mqtt.connect(testHost, {
    reconnectPeriod: -1
  });
  assert.equal(client.reconnecting, false);
  // FIXME
  // testHost may not be able to connect, so don't use mustCall here
  client.once('connect', function() {
    assert.equal(client.reconnecting, false);
    assert.equal(client.connected, true);
    assert.equal(client._isSocketConnected, true);
    // trigger the close event
    client.once('offline', common.mustCall(function() {
      assert.equal(client.reconnecting, false);
      assert.equal(client.connected, false);
      assert.equal(client._isSocketConnected, false);
    }));
    client.disconnect();
    assert.equal(client._keepAliveTimer, null);
    assert.equal(client._keepAliveTimeout, null);
  });
  //the close event will be triggered whether testHost is connected or not
  client.once('close', common.mustCall(function() {
    assert.equal(client.reconnecting, false);
    assert.equal(client.connected, false);
    assert.equal(client._isSocketConnected, false);
  }));
}

test(mqttHost);
test(mqttsHost);

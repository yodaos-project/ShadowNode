'use strict';

var util = require('util');
var URL = require('url');
var net = require('net');
var EventEmitter = require('events').EventEmitter;

var MQTT_CONNECT      = 1;
var MQTT_CONNACK      = 2;
var MQTT_PUBLISH      = 3;
var MQTT_PUBACK       = 4;
var MQTT_PUBREC       = 5;
var MQTT_PUBREL       = 6;
var MQTT_PUBCOMP      = 7;
var MQTT_SUBSCRIBE    = 8;
var MQTT_SUBACK       = 9;
var MQTT_UNSUBSCRIBE  = 10;
var MQTT_UNSUBACK     = 11;
var MQTT_PINGREQ      = 12;
var MQTT_PINGRESP     = 13;
var MQTT_DISCONNECT   = 14;

/**
 * @class MqttClient
 * @param {String} endpoint
 * @param {Object} options
 */
function MqttClient(endpoint, options) {
  EventEmitter.call(this);
  var obj = URL.parse(endpoint);
  this._host = obj.hostname;
  this._port = Number(obj.port) || 8883;
  this._protocol = obj.protocol;
  this._options = Object.assign({
    username: null,
    password: null,
    clientId: 'mqttjs_' + Math.random().toString(16).substr(2, 8),
    keepalive: 60 * 1000,
    reconnectPeriod: 1000,
    connectTimeout: 30 * 1000,
    resubscribe: true,
    protocolId: 'MQTT',
    protocolVersion: 4,
  }, options);
  this._reconnecting = false;
  this._reconnectingTimer = null;
  this._msgId = 0;
  this._ttl = null;
  this._handle = new native.MqttHandle(this._options);
}
util.inherits(MqttClient, EventEmitter);

/**
 * @method connect
 */
MqttClient.prototype.connect = function() {
  var tls;
  var opts = Object.assign({
    port: this._port,
    host: this._host,
  }, this._options);
  if (this._protocol === 'mqtts:') {
    tls = require('tls');
    this._socket = tls.connect(opts, this._onconnect.bind(this));
  } else {
    this._socket = net.connect(opts, this._onconnect.bind(this));
  }
  this._socket.on('data', (chunk) => {
    var res = this._handle._readPacket(chunk);
    this.emit('packetreceive');

    if (res.type === MQTT_CONNACK) {
      if (this._reconnecting) {
        this._reconnecting = false;
        this.emit('reconnect');
      }
      this._keepAlive();
      this.emit('connect');
    } else if (res.type === MQTT_PUBLISH) {
      var msg = this._handle._deserialize(res.buffer);
      var ret = Object.assign(msg, {
        toString: function(enc) {
          return msg.payload.toString(enc || 'utf8');
        }
      });
      this.emit('message', msg.topic, ret);
      if (msg.qos > 0) {
        // send publish ack
        var ack = this._handle._getAck(msg.id, msg.qos);
        this._write(ack, function(err) {
          console.log('sent ack buffer with', err);
        });
      }
    }
  });
  this._socket.on('error', (err) => {
    if (this._socket && this._socket.end) {
      this._socket.end();
    }
    if (this._reconnecting) {
      if (this._reconnectingTimer)
        clearTimeout(this._reconnectingTimer);
      this._reconnectingTimer = setTimeout(
        this.reconnect.bind(this), this._options.reconnectPeriod);
    } else {
      this.emit('error', err);
    }
  });
  this._socket.on('end', (err) => {
    this.emit('offline');
    this.reconnect();
  });
  return this;
};

/**
 * @method _onconnect
 */
MqttClient.prototype._onconnect = function() {
  var buf = this._handle._getConnect();
  return this._write(buf);
};

/**
 * @method _write
 * @param {Buffer} buffer
 * @param {Function} callback
 */
MqttClient.prototype._write = function(buffer, callback) {
  this._socket.write(buffer, (err, data) => {
    this.emit('packetsend');
    if (typeof callback === 'function') 
      callback(err, data);
  });
};

/**
 * @method _keepAlive
 */
MqttClient.prototype._keepAlive = function() {
  var buf = this._handle._getPingReq();
  this._write(buf);
  this._ttl = setTimeout(this._keepAlive.bind(this), this._options.keepalive);
};

/**
 * @method publish
 * @param {String} topic
 * @param {String} payload
 * @param {Object} options
 * @param {Function} callback
 */
MqttClient.prototype.publish = function(topic, payload, options, callback) {
  var buf = this._handle._getPublish(topic, {
    id: this._msgId++,
    qos: (options && options.qos) || 0,
    dup: (options && options.dup) || false,
    retain: (options && options.retain) || false,
    payload: payload || '',
  });
  this._write(buf);
};

/**
 * @method subscribe
 * @param {String} topic
 * @param {Object} options
 * @param {Function} callback
 */
MqttClient.prototype.subscribe = function(topic, options, callback) {
  if (!Array.isArray(topic))
    topic = [ topic ];
  if (typeof options === 'function') {
    callback = options;
    options = { qos: 0 };
  }
  var buf = this._handle._getSubscribe(topic, {
    id: this._msgId++,
    qos: (options && options.qos) || 0,
  });
  this._write(buf, callback);
};

/**
 * @method unsubscribe
 * @param {String} topic
 * @param {Function} callback
 */
MqttClient.prototype.unsubscribe = function(topic, callback) {
  var buf = this._handle._getUnsubscribe(topic, {
    id: this._msgId++,
  });
  this._write(buf, callback);
};

/**
 * @method unsubscribe
 * @param {Boolean} isForce
 * @param {Function} callback
 */
MqttClient.prototype.end = function(isForce, callback) {
  clearTimeout(this._ttl);

  var buf = this._handle._getDisconnect();
  if (isForce) {
    this._write(buf);
    this._socket.end();
  } else {
    this._write(buf, () => {
      this._socket.end();
    });
  }
};

/**
 * @method reconnect
 */
MqttClient.prototype.reconnect = function() {
  // this._reconnecting = true;
  // this.connect();
};

/**
 * @method getLastMessageId
 */
MqttClient.prototype.getLastMessageId = function() {
  return this._msgId;
};

/**
 * @property {Boolean} connected
 */
Object.defineProperty(MqttClient, 'connected', {
  get: function() {
    return this._isConnected();
  }
});

/**
 * @property {Boolean} reconnecting
 */
Object.defineProperty(MqttClient, 'reconnecting', {
  get: function() {
    return this._reconnecting;
  }
});

function connect(endpoint, options) {
  var client = new MqttClient(endpoint, options);
  return client.connect();
}

exports.connect = connect;
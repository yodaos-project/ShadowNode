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

var noop = function() {};

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
    reconnectPeriod: 5000,
    connectTimeout: 30 * 1000,
    resubscribe: true,
    protocolId: 'MQTT',
    protocolVersion: 4,
  }, options);
  this._isConnected = false;
  this._reconnecting = false;
  this._reconnectingTimer = null;
  this._lastConnectTime = 0;
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
  this._socket.on('data', this._ondata.bind(this));
  this._socket.once('error', this._ondisconnect.bind(this));
  this._socket.once('end', this._ondisconnect.bind(this));
  this._lastConnectTime = Date.now();
  return this;
};

/**
 * @method _onconnect
 */
MqttClient.prototype._onconnect = function() {
  this._isConnected = true;
  var buf;
  try {
    buf = this._handle._getConnect();
  } catch (err) {
    this.disconnect(err);
    return;
  }
  this._write(buf);
};

MqttClient.prototype._onerror = function(err) {
  this._ondisconnect(err);
};

MqttClient.prototype._onend = function() {
  this._ondisconnect();
};

MqttClient.prototype._ondisconnect = function(err) {
  if (err) {
    this.emit('error', err);
  }
  if (this._isConnected) {
    this._isConnected = false;
    this.emit('offline');
  }
  this.reconnect();
};

MqttClient.prototype._ondata = function(chunk) {
  var res;
  try {
    res = this._handle._readPacket(chunk);
  } catch (err) {
    this.disconnect(err);
    return;
  }
  this.emit('packetreceive');

  if (res.type === MQTT_CONNACK) {
    if (this._reconnecting) {
      clearTimeout(this._reconnectingTimer);
      this._reconnecting = false;
      this.emit('reconnect');
    } else {
      this.emit('connect');
    }
    this._keepAlive();
  } else if (res.type === MQTT_PUBLISH) {
    var msg;
    try {
      msg = this._handle._deserialize(res.buffer);
    } catch (err) {
      this.disconnect(err);
      return;
    }
    this.emit('message', msg.topic, msg.payload);
    if (msg.qos > 0) {
      // send publish ack
      try {
        var ack = this._handle._getAck(msg.id, msg.qos);
        this._write(ack);
      } catch (err) {
        this.disconnect(err);
      }
    }
  } else {
    // TODO handle other message type
  }
};

/**
 * @method _write
 * @param {Buffer} buffer
 * @param {Function} callback
 */
MqttClient.prototype._write = function(buffer, callback) {
  var self = this;
  callback = callback || noop;
  if (!self._isConnected) {
    callback(new Error('mqtt is disconnected'));
    return;
  }
  self._socket.write(buffer, function() {
    self.emit('packetsend');
    callback();
  });
};

/**
 * @method _keepAlive
 */
MqttClient.prototype._keepAlive = function() {
  try {
    var buf = this._handle._getPingReq();
    this._write(buf);
  } catch (err) {
    this.emit('error', err);
    return;
  }
  this._ttl = setTimeout(this._keepAlive.bind(this), this._options.keepalive);
};

MqttClient.prototype.disconnect = function(err) {
  if (err) {
    this.emit('error', err);
  }
  if (!this._isConnected) {
    return;
  }
  clearTimeout(this._ttl);
  clearTimeout(this._reconnectingTimer);
  try {
    var buf = this._handle._getDisconnect();
    this._write(buf);
  } catch (err) {
    this.emit('error', err);
  }
  this._socket.end();
};

/**
 * @method publish
 * @param {String} topic
 * @param {String} payload
 * @param {Object} options
 * @param {Function} callback
 */
MqttClient.prototype.publish = function(topic, payload, options, callback) {
  callback = callback || noop;
  if (!Buffer.isBuffer(payload)) {
    payload = new Buffer(payload);
  }
  try {
    var buf = this._handle._getPublish(topic, {
      id: this._msgId++,
      qos: (options && options.qos) || 0,
      dup: (options && options.dup) || false,
      retain: (options && options.retain) || false,
      payload: payload,
    });
    this._write(buf, callback);
  } catch (err) {
    callback(err);
  }
};

/**
 * @method subscribe
 * @param {String} topic
 * @param {Object} options
 * @param {Function} callback
 */
MqttClient.prototype.subscribe = function(topic, options, callback) {
  if (!Array.isArray(topic))
    topic = [topic];
  if (typeof options === 'function') {
    callback = options;
    options = { qos: 0 };
  } else {
    callback = callback || noop;
  }
  try {
    var buf = this._handle._getSubscribe(topic, {
      id: this._msgId++,
      qos: (options && options.qos) || 0,
    });
    this._write(buf, callback);
  } catch (err) {
    callback(err);
  }
};

/**
 * @method unsubscribe
 * @param {String} topic
 * @param {Function} callback
 */
MqttClient.prototype.unsubscribe = function(topic, callback) {
  callback = callback || noop;
  if (!Array.isArray(topic)) {
    topic = [topic];
  }
  try {
    var buf = this._handle._getUnsubscribe(topic, {
      id: this._msgId++,
    });
    this._write(buf, callback);
  } catch (err) {
    callback(err);
  }
};

/**
 * @method reconnect
 */
MqttClient.prototype.reconnect = function() {
  if (this._reconnecting) {
    return;
  }
  var reconnectPeriod = this._options.reconnectPeriod;
  if (reconnectPeriod < 0) {
    return;
  }
  this.disconnect();
  var t = this._lastConnectTime + reconnectPeriod - Date.now();
  if (t < 1) {
    this.connect();
  } else {
    setTimeout(this.connect.bind(this), t);
  }
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
    return this._isConnected;
  },
});

/**
 * @property {Boolean} reconnecting
 */
Object.defineProperty(MqttClient, 'reconnecting', {
  get: function() {
    return this._reconnecting;
  },
});

function connect(endpoint, options) {
  var client = new MqttClient(endpoint, options);
  return client.connect();
}

exports.connect = connect;

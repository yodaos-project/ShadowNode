'use strict';

require('crypto');

var net = require('net');
var util = require('util');
var EventEmitter = require('events').EventEmitter;
var TlsWrap = native.TlsWrap;
var TLS_CHUNK_MAX_SIZE = require('constants').TLS_CHUNK_MAX_SIZE;

function TLSSocket(socket, opts) {
  if (!(this instanceof TLSSocket))
    return new TLSSocket(socket, opts);
  EventEmitter.call(this);

  var tlsOptions = Object.assign({
    servername: socket.host || socket.hostname,
    // rejectUnauthorized: false,
  }, opts);

  // TODO: currently certification is disabled
  tlsOptions.rejectUnauthorized = false;

  // handle the [ca1,ca2,...]
  if (Array.isArray(tlsOptions.ca)) {
    tlsOptions.ca = tlsOptions.ca.join('\n');
  }

  this.servername = tlsOptions.servername;
  this.authorized = false;
  this.authorizationError = null;
  this._socket = new net.Socket(socket);
  this._writev = [];
  this._ended = false;

  // Just a documented property to make secure sockets
  // distinguishable from regular ones.
  this.encrypted = false;
  this._socket.on('error', this._tlsError.bind(this));
  this._socket.on('connect', this.onsocket.bind(this));
  this._socket.on('data', this.onsocketdata.bind(this));

  function onclose() {
    this.emit('close');
  }

  function onfinish() {
    this.emit('finish');
  }

  function onend() {
    this.emit('end');
  }

  // bypass event emits
  this._socket.on('close', onclose.bind(this));
  this._socket.on('finish', onfinish.bind(this));
  this._socket.on('end', onend.bind(this));

  // init the handle
  this._tls = new TlsWrap(tlsOptions);
  this._tls.jsref = this;
  this._tls.onread = this.onread;
  this._tls.onwrite = this.onwrite;
  this._tls.onclose = this.onclose;
  this._tls.onhandshakedone = this.onhandshakedone;
  this._tls.ondata = this.ondata;
}
util.inherits(TLSSocket, EventEmitter);

TLSSocket.prototype.connect = function(opts, callback) {
  if (typeof callback === 'function') {
    this.once('connect', callback);
  }
  this._socket.connect(opts);
  return this;
};

TLSSocket.prototype.write = function(data, cb) {
  if (!Buffer.isBuffer(data))
    data = new Buffer(data);

  if (!this.encrypted) {
    this._writev.push([data, cb]);
    return;
  }

  var chunks = [];
  var chunkByteLength = data.byteLength;
  var sourceStart = 0;
  var sourceLength;
  while (sourceStart < chunkByteLength) {
    var chunkLeft = chunkByteLength - sourceStart;
    if (chunkLeft > TLS_CHUNK_MAX_SIZE) {
      sourceLength = TLS_CHUNK_MAX_SIZE;
    } else {
      sourceLength = chunkLeft
    }
    var chunk = data.slice(sourceStart, sourceStart + sourceLength);
    sourceStart += sourceLength;
    try {
      /* XXX: tls.write may throw error if iotjs_tlswrap_encode_data failure */
      var encodedChunk = this._tls.write(chunk);
      chunks.push(encodedChunk);
    } catch (err) {
      cb(err);
      return false;
    }
  }
  var encodedData = Buffer.concat(chunks);
  return this._socket.write(encodedData, cb);
};

TLSSocket.prototype.onsocket = function() {
  this.emit('socket', this._socket);
  if (this._ended) return;
  this._tls.handshake();
};

TLSSocket.prototype.onsocketdata = function(chunk) {
  if (this._ended) return;
  this._tls.read(chunk);
};

TLSSocket.prototype.onwrite = function(chunk) {
  var self = this.jsref;
  var bytes = self._socket.write(chunk);
  return bytes;
};

TLSSocket.prototype.onread = function(chunk) {
  var self = this.jsref;
  self.emit('data', chunk);
};

TLSSocket.prototype.onclose = function() {
  var self = this.jsref;
  self.emit('close');
};

TLSSocket.prototype.onhandshakedone = function(status) {
  var self = this.jsref;
  self.authorized = true;
  self.encrypted = true;

  for (var i = 0; i < self._writev.length; i++) {
    self.write.apply(self, self._writev[i]);
  }
  self._writev.length = [];
  self.emit('connect', this);
};

TLSSocket.prototype._tlsError = function(err) {
  this.emit('error', err);
};

TLSSocket.prototype.pause = function() {
  this._socket.pause();
};

TLSSocket.prototype.resume = function() {
  this._socket.resume();
};

TLSSocket.prototype.end = function() {
  this._ended = true;
  this._socket.end(null, () => {
    this._tls.end();
  });
};

TLSSocket.prototype.destroy = function() {
  this._socket.destroy();
};

function connect(options, callback) {
  return TLSSocket({
    port: options.port,
    host: options.host,
  }, options).connect(options, callback);
}

exports.TLSSocket = TLSSocket;
exports.connect = connect;
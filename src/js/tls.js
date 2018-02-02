'use strict';

require('crypto');

var net = require('net');
var util = require('util');
var EventEmitter = require('events').EventEmitter;
var TlsWrap = native.TlsWrap;

function TLSSocket(socket, opts) {
  if (!(this instanceof TLSSocket))
    return new TLSSocket(socket, opts);
  EventEmitter.call(this);

  var tlsOptions = Object.assign({
    servername: socket.host || socket.hostname,
    rejectUnauthorized: false,
  }, opts);
  this.servername = tlsOptions.servername;
  this.authorized = false;
  this.authorizationError = null;
  this._socket = new net.Socket(socket);
  this._pendingRead = false;
  this._chunkSize = 8 * 1024;

  // Just a documented property to make secure sockets
  // distinguishable from regular ones.
  this.encrypted = true;
  this._socket.on('error', this._tlsError.bind(this));
  this._socket.on('connect', this.onsocket.bind(this));
  this._socket.on('end', this.onsocketend.bind(this));

  // init the handle
  this._tls = new TlsWrap(tlsOptions);
  this._tls.jsref = this;
  this._tls.onread = this.onread;
  this._tls.onwrite = this.onwrite;
  this._tls.onhandshakedone = this.onhandshakedone;
  this._tls.ondata = this.ondata;
}
util.inherits(TLSSocket, EventEmitter);

TLSSocket.prototype.connect = function(opts, callback) {
  this.once('connect', callback);
  return this._socket.connect(opts);
};

TLSSocket.prototype.write = function(data) {
  if (!Buffer.isBuffer(data))
    data = new Buffer(data);
  return this._tls.write(data);
};

TLSSocket.prototype.read = function() {
  if (this._pendingRead) {
    throw new Error('read process is pending');
  }
  this._pendingRead = true;
  this._tls.read(this._chunkSize);
};

TLSSocket.prototype.ondata = function(bytes, data) {
  this._pendingRead = false;
  this.emit('data', data);
  if (bytes > 0) {
    this.read();
  }
};

TLSSocket.prototype.onsocket = function() {
  this.emit('socket', this._socket);
  this._tls.handshake();
};

TLSSocket.prototype.onsocketend = function() {
  console.log('disconnected');
};

TLSSocket.prototype.onread = function(size) {
  var buf = this.jsref._socket.read(size);
  return Buffer.isBuffer(buf) ? buf : null;
};

TLSSocket.prototype.onwrite = function(chunk) {
  var self = this.jsref;
  return self._socket.write(chunk);
};

TLSSocket.prototype.onhandshakedone = function(status) {
  var self = this.jsref;
  self.authorized = true;
  self.emit('connect');
};

TLSSocket.prototype._tlsError = function(err) {
  console.error(err);
};


function connect(options, callback) {
  return TLSSocket({
    port: options.port,
    host: options.host,
  }).connect(options, callback);
}

exports.TLSSocket = TLSSocket;
exports.connect = connect;
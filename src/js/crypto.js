'use strict';

var Writable = require('stream').Writable;
var _randomBytesSync = native.randomBytesSync;
var _Hash = require('crypto_hash').Hash;

function Hash(algorithm) {
  if (!Hash._hashes[algorithm]) {
    throw new Error('Unknown hash algorithm ' + algorithm);
  }
  this._handle = new _Hash(Hash._hashes[algorithm]);
}

Hash.prototype.update = function(buf, inputEncoding) {
  if (typeof buf !== 'string' && !Buffer.isBuffer(buf)) {
    throw new TypeError(
      'Expect buffer or string on first argument of cipher.update.');
  }
  if (typeof buf === 'string') {
    buf = Buffer.from(buf, inputEncoding);
  }
  this._handle.update(buf);
  return this;
};

Hash.prototype.digest = function(encoding) {
  var buf = this._handle.digest();
  if (typeof encoding === 'string') {
    return buf.toString(encoding);
  } else {
    return buf;
  }
};

Hash._hashes = {
  'none': 0,
  'md2': 1,
  'md4': 2,
  'md5': 3,
  'sha1': 4,
  'sha224': 5,
  'sha256': 6,
  'sha384': 7,
  'sha512': 8,
  'ripemd160': 9
};

exports.createHash = function(algorithm) {
  return new Hash(algorithm);
};

exports.getHashes = function() {
  return Object.keys(Hash._hashes);
};

exports.randomBytes = function(size, callback) {
  var bytes = _randomBytesSync(size);
  var buf = new Buffer(bytes);
  if (typeof callback === 'function') {
    callback(null, buf);
  }
  return buf;
};


// Sign & Verify
var _Sign = require('crypto_sign').Sign;
var _Verify = require('crypto_verify').Verify;

function Sign(algorithm, options) {
  if (!(this instanceof Sign))
    return new Sign(algorithm, options);
  if (typeof algorithm !== 'string') {
    throw new TypeError('algorithm must one string type');
  }

  if (!Hash._hashes[algorithm]) {
    throw new Error('Unknown hash algorithm ' + algorithm);
  }

  this._signHandle = new _Sign(Hash._hashes[algorithm]);
  Writable.call(this, options);
}

Sign.prototype.update = function(buf, inputEncoding) {
  if (typeof buf !== 'string' && !Buffer.isBuffer(buf)) {
    throw new TypeError(
      'Expect buffer or string on first argument of update.');
  }

  if (typeof buf === 'string') {
    buf = Buffer.from(buf, inputEncoding);
  }

  this._signHandle.update(buf);
  return this;
};

Sign.prototype.sign = function(privateKey, encoding) {
  if (typeof privateKey !== 'string' && !Buffer.isBuffer(privateKey)) {
    throw new TypeError(
      'Expect buffer or string on first argument of update.');
  }

  if (typeof privateKey === 'string') {
    privateKey = Buffer.from(privateKey);
  }

  var buf = this._signHandle.sign(privateKey);
  if (typeof encoding === 'string') {
    return buf.toString(encoding);
  }
  return buf;
};

exports.createSign = function(algorithm, options) {
  return new Sign(algorithm, options);
};


function Verify(algorithm, options) {
  if (!(this instanceof Verify))
    return new Verify(algorithm, options);
  if (typeof algorithm !== 'string') {
    throw new TypeError('algorithm must be a string value');
  }

  if (!Hash._hashes[algorithm]) {
    throw new Error('Unknown hash algorithm ' + algorithm);
  }

  this._verify_handler = new _Verify(Hash._hashes[algorithm]);

  Writable.call(this, options);
}

Verify.prototype.update = function(buf, inputEncoding) {
  if (typeof buf !== 'string' && !Buffer.isBuffer(buf)) {
    throw new TypeError(
      'Expect buffer or string on first argument of update.');
  }

  if (typeof buf === 'string') {
    buf = Buffer.from(buf, inputEncoding);
  }

  this._verify_handle.update(buf);
  return this;
};

Verify.prototype.verify = function(object, signature, signatureEncoding) {

  return true;
};

exports.createVerify = function(algorithm, options) {
  return new Verify(algorithm, options);
};

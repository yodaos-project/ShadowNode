'use strict';

var Transform = require('stream').Transform;
var util = require('util');
var constants = native;
var zlib_modes = {
  none: 0,
  deflate: 1,
  inflate: 2,
  gzip: 3,
  gunzip: 4,
  deflateraw: 5,
  inflateraw: 6,
  unzip: 7,
};

// translation table for return codes.
var codes = {
  Z_OK: constants.Z_OK,
  Z_STREAM_END: constants.Z_STREAM_END,
  Z_NEED_DICT: constants.Z_NEED_DICT,
  Z_ERRNO: constants.Z_ERRNO,
  Z_STREAM_ERROR: constants.Z_STREAM_ERROR,
  Z_DATA_ERROR: constants.Z_DATA_ERROR,
  Z_MEM_ERROR: constants.Z_MEM_ERROR,
  Z_BUF_ERROR: constants.Z_BUF_ERROR,
  Z_VERSION_ERROR: constants.Z_VERSION_ERROR,
};

function zlibOnError(message, errno) {
  var self = this.jsref;
  // there is no way to cleanly recover.
  // continuing only obscures problems.
  _close(self);
  self._hadError = true;

  var error = new Error(message);
  error.errno = errno;
  error.code = codes[errno];
  self.emit('error', error);
}

function Zlib(opts, mode) {
  var chunkSize = constants.Z_DEFAULT_CHUNK;
  var flush = constants.Z_NO_FLUSH;
  var finishFlush = constants.Z_FINISH;
  var windowBits = constants.Z_DEFAULT_WINDOWBITS;
  var level = constants.Z_DEFAULT_COMPRESSION;
  var memLevel = constants.Z_DEFAULT_MEMLEVEL;
  var strategy = constants.Z_DEFAULT_STRATEGY;
  var dictionary;

  if (typeof mode !== 'number')
    throw new TypeError('ERR_INVALID_ARG_TYPE');
  if (mode < constants.DEFLATE || mode > constants.UNZIP)
    throw new RangeError('ERR_OUT_OF_RANGE');

  Transform.call(this, opts);
  this.bytesRead = 0;
  this._handle = new native.Zlib(zlib_modes[mode]);
  this._handle.jsref = this; // Used by processCallback() and zlibOnError()
  this._handle.onerror = zlibOnError;
  this._hadError = false;
  this._writeState = new Array(2);

  if (!this._handle.init(windowBits,
                         level,
                         memLevel,
                         strategy,
                         this._writeState,
                         processCallback,
                         dictionary)) {
    throw new Error('ERR_ZLIB_INITIALIZATION_FAILED');
  }
}
util.inherits(Zlib, Transform);

Object.defineProperty(Zlib.prototype, '_closed', {
  configurable: true,
  enumerable: true,
  get: function() {
    return !this._handle;
  }
});

function processCallback() {
  console.log('process');
}

Zlib.prototype._flush = function _flush(callback) {
  this._transform(Buffer.alloc(0), '', callback);
};

Zlib.prototype._transform = function _transform(chunk, encoding, cb) {
  var flushFlag;
  var ws = this._writableState;
  if ((ws.ending || ws.ended) && ws.length === chunk.byteLength) {
    flushFlag = this._finishFlushFlag;
  } else {
    flushFlag = this._flushFlag;
    // once we've flushed the last of the queue, stop flushing and
    // go back to the normal behavior.
    if (chunk.byteLength >= ws.length)
      this._flushFlag = this._origFlushFlag;
  }
  processChunk(this, chunk, flushFlag, cb);
};

function _close(engine, callback) {
  if (callback)
    process.nextTick(callback);

  // Caller may invoke .close after a zlib error (which will null _handle).
  if (!engine._handle)
    return;

  engine._handle.close();
  engine._handle = null;
}

module.exports = {
  Zlib: Zlib,
};

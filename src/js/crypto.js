// var _randomBytes = native.randomBytes;
var _randomBytesSync = native.randomBytesSync;

exports.randomBytes = function(size, callback) {
  var bytes = _randomBytesSync(size);
  var buf = new Buffer(bytes);
  if (typeof callback === 'function') {
    callback(null, buf);
  }
  return buf;
};

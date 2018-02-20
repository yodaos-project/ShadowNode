var assert = require('assert');
var tls = require('tls');

var socket = tls.connect({
  port: 443,
  host: 'www.bing.com',
}, function() {
  socket.end();
});

var closed = false;
socket.on('close', function() {
  closed = true;
});

var finished = false;
socket.on('finish', function() {
  finished = true;
});

var thrown = null;
socket.on('error', function(err) {
  thrown = err;
});

process.on('exit', function() {
  assert.equal(closed, true);
  assert.equal(finished, true);
  assert.equal(thrown.code, 'ECONNRESET');
});

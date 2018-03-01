var net = require('net');
var assert = require('assert');

var PORT = 54321

var server = net.createServer(socket => {
  socket.end('goodbye\n');
});

server.on('error', () => {
  assert.fail('first server encountered unexpected error');
});

var done = false;
var ret = server.listen(PORT, () => {
  var dupServer = net.createServer(socket => {
    socket.end('goodbye\n');
  }).on('error', err => {
    assert(err !== null);
    assert(typeof err === 'object')
    assert(err.code === 'EADDRINUSE')
    done = true;
  });

  var dupret = dupServer.listen(PORT, () => {
    assert.fail('server listened on same port');
  });

  assert(typeof dupret === 'object');
});

assert(typeof ret === 'object')

setTimeout(() => {
  assert(done, 'timed out');
  server.close();
}, 100)

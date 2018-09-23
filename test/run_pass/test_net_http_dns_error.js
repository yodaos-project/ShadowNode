var assert = require('assert');
var http = require('http');

var host = 'www.unable-to-resolved-host.com'
var req = http.request({
  host: host,
})
req.socket.on('finish', () => {
  process.nextTick(() => {
    var destroyed = req.socket._socketState.destroyed;
    console.log(`socket is destroyed ${host}:`, destroyed)
    assert.equal(destroyed, true)
  })
})
// handle error event to prevent the process throw an error
req.on('error', (err) => {
  console.log('request error', err.message)
})
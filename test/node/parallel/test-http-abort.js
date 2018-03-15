var http = require('http');
var https = require('https');
var Url = require('url');
var assert = require('assert');


function getHandle(protocol) {
  switch (protocol) {
    case 'http:':
      return http;
    case 'https:':
      return https;
    default:
      throw new Error('unsupported protocol ' + protocol);
  }
}

// test http_client
test('http://www.baidu.com');
// test https_client
test('https://www.baidu.com');

function test(url) {
  url = Url.parse(url);
  var isAborted = false;
  var eventTriggered = false;
  var chunks = [];
  var handle = getHandle(url.protocol);
  var req = handle.get(url.href, function(res) {
    res.on('data', function(chunk) {
      assert.strictEqual(isAborted, false, 'should not aborted');
      isAborted = true;
      req.abort();
      process.nextTick(function() {
        assert.strictEqual(eventTriggered, true, 'should trigger event abort');
      });
      chunks.push(chunk);
    });

    res.on('end', function() {
      var body = Buffer.concat(chunks).toString('utf8');
      console.log(url.href, 'end', body.length);
    });

  });

  req.on('abort', function() {
    eventTriggered = true;
  });

  req.end();
}

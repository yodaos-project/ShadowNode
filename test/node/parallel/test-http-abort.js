var http = require('http');
var https = require('https');
var assert = require('assert');

function getHandle(url) {
  if (/^http:\/\//.test(url)) {
    return http;
  } else if (/^https:\/\//.test(url)) {
    return https;
  }
  throw new Error('unsupported url ' + url);
}

// test http_client
test('http://www.baidu.com');
// test https_client
test('https://www.baidu.com');

function test(url) {
  var isAborted = false;
  var eventTriggered = false;
  var chunks = [];
  var handle = getHandle(url);
  var req = handle.get(url, function(res) {
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
      console.log(url, 'end', body.length);
    });

  });

  req.on('abort', function() {
    eventTriggered = true;
  });

  req.end();
}

'use strict';
var assert = require('assert');
var data = require('./test_process_send.json');
if (process.send) {
  // send with callback
  process.send(data, err => {
    assert.equal(err, undefined);
  });
  // send without callback
  process.send(data);
} else {
  var fork = require('child_process').fork;
  var sendTimes = 0;
  var child = fork(module.filename, [], {});
  child.on('message', msg => {
    var msgLength = JSON.stringify(msg).length;
    var dataLength = JSON.stringify(data).length;
    console.log(`received ${msgLength}bytes data ${++sendTimes} times`);
    assert.equal(dataLength, msgLength);
  });
}

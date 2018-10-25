'use strict';
var assert = require('assert');
// json length is close to 19KB
var data = require('./test_process_send.json');
var dataStr = JSON.stringify(data)
var name = process.send ? 'child' : 'parent'

var equalTimes = 0;
function equalData(msg) {
  if (typeof msg === 'object') {
    msg = JSON.stringify(msg);
  }
  ++equalTimes;
  console.log(name, 'equal data', equalTimes)
  assert.equal(dataStr.length, msg.length);
}

var obj = null;
if (process.send) {
  obj = process
} else {
  var fork = require('child_process').fork;
  obj = fork(module.filename, [], {});
}
obj.on('message', equalData);

var times = 10
var timer = setInterval(() => {
  console.log(name, 'sent', times)
  --times;
  obj.send(data);
  obj.send(dataStr);
  if (times <= 0) {
    clearInterval(timer);
  }
}, 500)
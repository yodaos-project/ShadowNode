var assert = require('assert');
var path = require('path');
var fork = require('child_process').fork;
var grep = fork(
  path.join(__dirname, 'test_child_process_kill2.js'),
  {
    execArgv: [],
  }
);

var closed;
var exited

grep.on('close', function(code, signal) {
  assert.equal(signal, 'SIGTERM');
  closed = true;
});

grep.on('exit', function(code, signal) {
  assert.equal(signal, 'SIGTERM');
  exited = true;
});

// Send SIGHUP to process
var killed = grep.kill();
assert.equal(killed, true);
assert.equal(killed, grep.killed);

process.on('exit', function() {
  console.log('exit');
  assert.equal(closed, true);
  assert.equal(exited, true);
});
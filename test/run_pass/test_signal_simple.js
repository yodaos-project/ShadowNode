'use strict';

var assert = require('assert');
var signals = require('constants').os.signals;

function testSignal(type) {
  process.once(type, function(signal) {
    console.log(signal);
    assert.equal(signal, type);
  });
  process.kill(signals[type]);
}

testSignal('SIGHUP');
testSignal('SIGINT');
testSignal('SIGQUIT');
testSignal('SIGILL');
testSignal('SIGTRAP');
testSignal('SIGABRT');
testSignal('SIGBUS');
testSignal('SIGFPE');
testSignal('SIGUSR1');
testSignal('SIGSEGV');
testSignal('SIGUSR2');
testSignal('SIGPIPE');
testSignal('SIGALRM');
testSignal('SIGTERM');
testSignal('SIGSTKFLT');
testSignal('SIGSTOP');

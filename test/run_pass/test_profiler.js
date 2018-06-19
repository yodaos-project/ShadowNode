'use strict';

var profiler = require('profiler');
profiler.startProfiling();
console.log('cpu profiling starts');

function test() {}
for (var i = 0; i < 10; i++) {
  test();
}

setTimeout(function foobar() {
  var profile = profiler.stopProfiling();
  console.log(profile);
}, 1000);

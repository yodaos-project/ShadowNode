'use strict';

var profiler = require('profiler');
profiler.startProfiling();
console.log('cpu profiling starts');

function test() {}
console.log('test name', test.name);
test();


setTimeout(function foobar() {
  console.log('foobar test');
  var profile = profiler.stopProfiling();
  console.log(profile);
}, 1000);

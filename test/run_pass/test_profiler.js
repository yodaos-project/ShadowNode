'use strict';

var profiler = require('profiler');
profiler.startProfiling();
console.log('cpu profiling starts');

setTimeout(function foobar() {
  console.log('foobar test');
  var profile = profiler.stopProfiling();
  console.log(profile);
}, 3000);

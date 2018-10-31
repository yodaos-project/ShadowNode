'use strict';

var common = require('../common.js');
var bench = common.createBenchmark(main, {
  n: [1024],
});

function main(opts) {
  var n = opts.n;
  var tasks = [];
  function runTasks(i) {
    process.nextTick(function() {
      tasks[i]();
      if (++i <= n) {
        process.nextTick(runTasks, i);
      }
    });
  }
  for (var i = 0; i < n; ++i) {
    tasks.push(function() {});
  }
  tasks.push(function() {
    bench.end(n);
  })
  bench.start();
  runTasks(0);
}

'use strict';

var common = require('../common.js');

var bench = common.createBenchmark(main, {
  n: [512]
});

function main(opts) {
  var n = opts.n;
  bench.start();
  for (var i = 0; i < n; ++i) {
    require('os');
    require('../common.js');
  }
  bench.end(n);
}

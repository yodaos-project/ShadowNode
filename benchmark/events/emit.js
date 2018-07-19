'use strict';

var EventEmitter = require('events').EventEmitter;
var common = require('../common.js');
var bench = common.createBenchmark(main, {
  n: [256],
});

var emitter = new EventEmitter();

function main(opts) {
  var n = opts.n;
  bench.start();
  for (var i = 0; i < n; i += 1)
    emitter.on('data', () => false);
  for (var i = 0; i < n; i += 1)
    emitter.emit('data', 'test');
  bench.end(n);
}

'use strict';
var common = require('../common.js');
var posix = require('path');

var bench = common.createBenchmark(main, {
  path: [
    '',
    '/',
    '/foo',
    '/foo/bar',
    'foo',
    'foo/bar',
    '/foo/bar/baz/asdf/quux',
  ],
  n: [1e5]
});

function main(opts) {
  var n = opts.n;
  var path = opts.path;

  bench.start();
  for (var i = 0; i < n; i++) {
    posix.dirname(i % 3 === 0 ? `${path}${i}` : path);
  }
  bench.end(n);
}

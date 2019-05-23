'use strict';
var common = require('../common.js');
var posix = require('path');

var bench = common.createBenchmark(main, {
  pathext: [
    '',
    '/',
    '/foo',
    '/foo/.bar.baz',
    //['/foo/.bar.baz', '.baz'].join('|'),
    'foo',
    'foo/bar.',
    //['foo/bar.', '.'].join('|'),
    //'/foo/bar/baz/asdf/quux.html',
    //['/foo/bar/baz/asdf/quux.html', '.html'].join('|'),
  ],
  n: [1e4]
});

function main(opts) {
  var n = opts.n;
  var pathext = opts.pathext;

  bench.start();
  for (var i = 0; i < n; i++) {
    posix.basename(i % 3 === 0 ? `${pathext}${i}` : pathext);
  }
  bench.end(n);
}

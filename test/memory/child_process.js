'use strict';

var exec = require('child_process').exec;
var startProfiling = false;
var matrix = {
  startRss: 0,
  endRss: 0,
  samples: 0,
};

var duration = process.argv[2] || 5000;
var cycle = process.argv[3] || 0;

(function main() {
  var rss = process.memoryUsage().rss;
  exec('ls', (err) => err && console.error('error', err));
  setTimeout(() => {
    var curr = process.memoryUsage().rss;
    if (startProfiling) {
      if (!matrix.startRss)
        matrix.startRss = curr;
      matrix.endRss = rss;
      matrix.samples += 1;
    }
    console.log(`> total: ${curr}, increase: ${curr - rss}`);
    main();
  }, cycle);
})();

function summary() {
  console.log(`Memory Leak Summary(${matrix.samples} samples):`);
  console.log(` Rss(From): ${matrix.startRss}`);
  console.log(` Rss(End ): ${matrix.endRss}`);
  console.log(` Detects ${matrix.endRss - matrix.startRss} bytes`);
  process.exit();
}

setTimeout(() => {
  startProfiling = true;
  setTimeout(summary, duration);
}, 500);

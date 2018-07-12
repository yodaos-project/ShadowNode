'use strict';

var exec = require('child_process').exec;
var startProfiling = false;
var profilingHandle = null;
var matrix = {
  startRss: 0,
  endRss: 0,
  samples: 0,
  increasingCount: 0,
  decreasingCount: 0,
};

var duration = process.argv[2] || 7000;
var cycle = process.argv[3] || 0;

(function main() {
  var rss = process.memoryUsage().rss;
  exec('ls', (err) => err && console.error('error', err));
  profilingHandle = setTimeout(() => {
    var curr = process.memoryUsage().rss;
    if (startProfiling) {
      if (!matrix.startRss)
        matrix.startRss = curr;
      matrix.endRss = rss;
      matrix.samples += 1;
      if (curr - rss > 0) {
        matrix.increasingCount += 1;
      } else if (curr - rss < 0) {
        matrix.decreasingCount += 1;
      }
    }
    if (curr - rss !== 0) {
      console.log(`:: ${curr} + ${curr - rss} bytes`);
    }
    main();
  }, cycle);
})();

function summary() {
  clearTimeout(profilingHandle);
  console.log(`\nMemory Summary(${matrix.samples} samples):`);
  console.log(` Duration ${duration/1000}s`);
  console.log(` Rss(From): ${matrix.startRss}`);
  console.log(` Rss(End ): ${matrix.endRss}`);
  console.log(` Detects ${matrix.endRss - matrix.startRss} bytes`);
  console.log('====================================================');
  console.log(` Increasing Count  : ${matrix.increasingCount} / ${matrix.samples}`);
  console.log(` Decreasing Count  : ${matrix.decreasingCount} / ${matrix.samples}`);
  console.log(` Increasing Rate   : ${parseInt((matrix.increasingCount / matrix.samples) * 100)}%`);
  console.log(` Decreasing Rate   : ${parseInt((matrix.decreasingCount / matrix.samples) * 100)}%`);
  setTimeout(process.exit, 500);
}

setTimeout(() => {
  startProfiling = true;
  setTimeout(summary, duration);
}, 500);

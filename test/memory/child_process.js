'use strict';

var exec = require('child_process').exec;

(function main() {
  var rss = process.memoryUsage().rss;
  exec('ls', (err) => err && console.error(err));
  setTimeout(() => {
    var curr = process.memoryUsage().rss;
    console.log(`> total: ${curr}, increase: ${curr - rss}`);
    main();
  }, 100)
})();

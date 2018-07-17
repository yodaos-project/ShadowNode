'use strict';
var run = require('./').run;

function report(pathname) {
  var tests = require(`./${pathname}.json`);
  var result = run(tests);
  console.log(`Test ES compatible(${pathname})`);
  console.log(JSON.stringify(result, null, 2));
}

report('builtins/es2015');
report('builtins/es2016');
report('syntax/es2015');
report('syntax/es2016');


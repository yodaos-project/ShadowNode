'use strict';

// from https://github.com/Tokimon/es-feature-detection
// author: https://github.com/Tokimon

function check(script) {
  try {
    return (new Function('"use strict";\n' + script))() !== false; // eslint-disable-line no-new-func
  } catch(ex) {
    return false;
  }
}

function run(tests) {
  var results = {
    __all: true
  };
  for (var key in tests) {
    var test = tests[key];
    if (!test)
      continue;

    if (typeof test === 'object') {
      results[key] = run(test);
      if (!results[key].__all)
        results.__all = false;
    } else {
      results[key] = check(test);
      if (!results[key])
        results.__all = false;
    }
  }
  return results;
}

exports.run = run;


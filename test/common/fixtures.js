'use strict';

var path = require('path');

var fixturesDir = path.join(__dirname, '../../');

function fixturesPath() {
  return path.join.apply(
    this,
    [fixturesDir].concat(Array.prototype.slice.apply(arguments)));
}

module.exports = {
  fixturesDir: fixturesDir,
  path: fixturesPath
};

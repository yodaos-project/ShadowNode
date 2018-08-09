'use strict';

var NODE_DEBUG = process.env.NODE_DEBUG;

module.exports = function(tag) {
  function debug(str) {
    console.info(`:${tag}: ${str}`);
  }
  if (NODE_DEBUG === '*') {
    return debug;
  }
  if (NODE_DEBUG === tag) {
    return debug;
  }
};

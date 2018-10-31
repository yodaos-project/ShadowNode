'use strict';

var nextTickQueue = [];

module.exports.nextTick = function nextTick(callback) {
  var args;
  switch (arguments.length) {
    case 1: break;
    case 2: args = [arguments[1]]; break;
    case 3: args = [arguments[1], arguments[2]]; break;
    case 4: args = [arguments[1], arguments[2], arguments[3]]; break;
    default:
      args = Array.prototype.slice.call(arguments, 1);
      break;
  }
  nextTickQueue.push({ callback: callback, args: args });
};

module.exports._onNextTick = function _onNextTick() {
  var i = 0;
  while (i < nextTickQueue.length) {
    var tickObject = nextTickQueue[i];
    var callback = tickObject.callback;
    var args = tickObject.args;
    try {
      if (args === undefined) {
        callback();
      } else {
        switch (args.length) {
          case 1: callback(args[0]); break;
          case 2: callback(args[0], args[1]); break;
          case 3: callback(args[0], args[1], args[2]); break;
          default: callback.apply(undefined, args); break;
        }
      }
    } catch (e) {
      process._onUncaughtException(e);
    }
    ++i;
  }
  nextTickQueue = [];
  return false;
};

'use strict';

function TickObject(callback, args) {
  this.callback = callback;
  this.args = args;
}

function NextTickQueue() {
  this.head = null;
  this.tail = null;
  this.length = 0;
}

NextTickQueue.prototype.push = function push(v) {
  var entry = { data: v, next: null };
  if (this.length > 0) {
    this.tail.next = entry;
  } else {
    this.head = entry;
  }
  this.tail = entry;
  ++this.length;
};

NextTickQueue.prototype.shift = function shift() {
  if (this.length === 0) {
    return;
  }
  var ret = this.head.data;
  if (this.length === 1) {
    this.head = this.tail = null;
  } else {
    this.head = this.head.next;
  }
  --this.length;
  return ret;
};

NextTickQueue.prototype.clear = function clear() {
  this.head = null;
  this.tail = null;
  this.length = 0;
};

var nextTickQueue = new NextTickQueue();

module.exports.nextTick = function nextTick(callback) {
  var args;
  switch (arguments.length) {
    case 1: break;
    case 2: args = [arguments[1]]; break;
    case 3: args = [arguments[1], arguments[2]]; break;
    case 4: args = [arguments[1], arguments[2], arguments[3]]; break;
    default:
      args = new Array(arguments.length - 1);
      for (var i = 1; i < arguments.length; i++) {
        args[i - 1] = arguments[i];
      }
      break;
  }
  var tickObject = new TickObject(callback, args);
  nextTickQueue.push(tickObject);
};

module.exports._onNextTick = function _onNextTick() {
  while (nextTickQueue.length > 0) {
    var tickObject = nextTickQueue.shift();
    var callback = tickObject.callback;
    var args = tickObject.args;
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
  }
  return false;
};

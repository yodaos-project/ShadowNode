/* Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

(function() {
  this.global = this;

  function Module(id) {
    this.id = id;
    this.exports = {};
  }


  Module.cache = {};

  Module.require = function(id) {
    if (id == 'native') {
      return Module;
    }

    if (Module.cache[id]) {
      return Module.cache[id].exports;
    }

    var module = new Module(id);

    Module.cache[id] = module;
    module.compile();

    return module.exports;
  };


  Module.prototype.compile = function() {
    process.compileModule(this, Module.require);
  };


  /**
   * Polyfills Start
   */

  if (typeof Object.assign != 'function') {
    // Must be writable: true, enumerable: false, configurable: true
    Object.defineProperty(Object, "assign", {
      value: function assign(target, varArgs) { // .length of function is 2
        'use strict';
        if (target == null) { // TypeError if undefined or null
          throw new TypeError('Cannot convert undefined or null to object');
        }

        var to = Object(target);

        for (var index = 1; index < arguments.length; index++) {
          var nextSource = arguments[index];
          if (nextSource != null) { // Skip over if undefined or null
            for (var nextKey in nextSource) {
              // Avoid bugs when hasOwnProperty is shadowed
              if (Object.prototype.hasOwnProperty.call(nextSource, nextKey)) {
                to[nextKey] = nextSource[nextKey];
              }
            }
          }
        }
        return to;
      },
      writable: true,
      configurable: true
    });
  }

  function CallSite(name, opts) {
    this._name = name;
    this._filename = opts[0];
    this._line = opts[1];
    this._column = opts[2];
    this._origin = false;
  }

  CallSite.create = function(name, opts) {
    return new CallSite(name, opts);
  };

  CallSite.prototype.getFileName = function() {
    return this._filename || '<anonymous>';
  };

  CallSite.prototype.getLineNumber = function() {
    return this._line || 1;
  };

  CallSite.prototype.getColumnNumber = function() {
    return this._column || 7;
  };

  CallSite.prototype.getFunctionName = function() {
    return this._name || 'undefined';
  };

  CallSite.prototype.isEval = function() {
    if (this._origin) {
      return true;
    } else {
      this._origin =
        this.getFunctionName() + ' ' +
        '(' +
          this.getFileName() + ':' +
          this.getLineNumber() + ':' +
          this.getColumnNumber() +
        ')';
      return true;
    }
  };

  CallSite.prototype.getEvalOrigin = function() {
    return this._origin;
  };

  function prepareStackTrace(throwable) {
    return [
      CallSite.create('main', ['<anonymous>', 1, 7]),
      CallSite.create('main', ['<anonymous>', 2, 7]),
      CallSite.create('main', ['<anonymous>', 3, 7]),
      CallSite.create('main', ['<anonymous>', 4, 7]),
    ];
  }

  Error.stackTraceLimit = 1;
  Error.prepareStackTrace = prepareStackTrace;
  Error.captureStackTrace = function(throwable, terminator) {
    Object.defineProperties(throwable, {
      stack: {
        configurable: true,
        get: function () {
          return prepareStackTrace(throwable);
        }
      },
      cachedStack: {
        configurable: true,
        writable: true,
        enumerable: false,
        value: true
      }
    });
  };

  Error.getStackTrace = function(throwable) {
    if (throwable.cachedStack)
      return throwable.stack;
    var stack = prepareStackTrace(throwable);
    try {
      Object.defineProperties(throwable, {
        stack: {
          configurable: true,
          writable: true,
          enumerable: false,
          value: stack
        },
        cachedStack: {
          configurable: true,
          writable: true,
          enumerable: false,
          value: true
        }
      });
    } catch (nonConfigurableError) {
      // SKIP
    }
    return stack;
  };

  // Symbol
  global.Symbol = function(description) {
    return description;
  };

  // Proxy
  (function(scope) {
    if (scope['Proxy']) {
      return;
    }
    var lastRevokeFn = null;

    /**
     * @param {*} o
     * @return {boolean} whether this is probably a (non-null) Object
     */
    function isObject(o) {
      return o ? (typeof o === 'object' || typeof o === 'function') : false;
    }

    /**
     * @constructor
     * @param {!Object} target
     * @param {{apply, construct, get, set}} handler
     */
    scope.Proxy = function(target, handler) {
      if (!isObject(target) || !isObject(handler)) {
        throw new TypeError('Cannot create proxy with a non-object as target or handler');
      }

      // Construct revoke function, and set lastRevokeFn so that Proxy.revocable can steal it.
      // The caller might get the wrong revoke function if a user replaces or wraps scope.Proxy
      // to call itself, but that seems unlikely especially when using the polyfill.
      var throwRevoked = function() {};
      lastRevokeFn = function() {
        throwRevoked = function(trap) {
          throw new TypeError(`Cannot perform '${trap}' on a proxy that has been revoked`);
        };
      };

      // Fail on unsupported traps: Chrome doesn't do this, but ensure that users of the polyfill
      // are a bit more careful. Copy the internal parts of handler to prevent user changes.
      var unsafeHandler = handler;
      handler = {'get': null, 'set': null, 'apply': null, 'construct': null};
      for (var k in unsafeHandler) {
        if (!(k in handler)) {
          throw new TypeError(`Proxy polyfill does not support trap '${k}'`);
        }
        handler[k] = unsafeHandler[k];
      }
      if (typeof unsafeHandler === 'function') {
        // Allow handler to be a function (which has an 'apply' method). This matches what is
        // probably a bug in native versions. It treats the apply call as a trap to be configured.
        handler.apply = unsafeHandler.apply.bind(unsafeHandler);
      }

      // Define proxy as this, or a Function (if either it's callable, or apply is set).
      // TODO(samthor): Closure compiler doesn't know about 'construct', attempts to rename it.
      var proxy = this;
      var isMethod = false;
      if (typeof target === 'function') {
        proxy = function Proxy() {
          var usingNew = (this && this.constructor === proxy);
          var args = Array.prototype.slice.call(arguments);
          throwRevoked(usingNew ? 'construct' : 'apply');

          if (usingNew && handler['construct']) {
            return handler['construct'].call(this, target, args);
          } else if (!usingNew && handler.apply) {
            return handler.apply(target, this, args);
          }

          // since the target was a function, fallback to calling it directly.
          if (usingNew) {
            // inspired by answers to https://stackoverflow.com/q/1606797
            args.unshift(target);  // pass class as first arg to constructor, although irrelevant
            // nb. cast to convince Closure compiler that this is a constructor
            var f = /** @type {!Function} */ (target.bind.apply(target, args));
            return new f();
          }
          return target.apply(this, args);
        };
        isMethod = true;
      }

      // Create default getters/setters. Create different code paths as handler.get/handler.set can't
      // change after creation.
      var getter = handler.get ? function(prop) {
        throwRevoked('get');
        return handler.get(this, prop, proxy);
      } : function(prop) {
        throwRevoked('get');
        return this[prop];
      };
      var setter = handler.set ? function(prop, value) {
        throwRevoked('set');
        var status = handler.set(this, prop, value, proxy);
        if (!status) {
          // TODO(samthor): If the calling code is in strict mode, throw TypeError.
          // It's (sometimes) possible to work this out, if this code isn't strict- try to load the
          // callee, and if it's available, that code is non-strict. However, this isn't exhaustive.
        }
      } : function(prop, value) {
        throwRevoked('set');
        this[prop] = value;
      };

      // Clone direct properties (i.e., not part of a prototype).
      var propertyNames = Object.getOwnPropertyNames(target);
      var propertyMap = {};
      propertyNames.forEach(function(prop) {
        if (isMethod && prop in proxy) {
          return;  // ignore properties already here, e.g. 'bind', 'prototype' etc
        }
        var real = Object.getOwnPropertyDescriptor(target, prop);
        var desc = {
          enumerable: !!real.enumerable,
          get: getter.bind(target, prop),
          set: setter.bind(target, prop),
        };
        Object.defineProperty(proxy, prop, desc);
        propertyMap[prop] = true;
      });

      // Set the prototype, or clone all prototype methods (always required if a getter is provided).
      // TODO(samthor): We don't allow prototype methods to be set. It's (even more) awkward.
      // An alternative here would be to _just_ clone methods to keep behavior consistent.
      var prototypeOk = true;
      if (Object.setPrototypeOf) {
        Object.setPrototypeOf(proxy, Object.getPrototypeOf(target));
      } else {
        prototypeOk = false;
      }
      if (handler.get || !prototypeOk) {
        for (var k in target) {
          if (propertyMap[k]) {
            continue;
          }
          Object.defineProperty(proxy, k, {get: getter.bind(target, k)});
        }
      }

      // The Proxy polyfill cannot handle adding new properties. Seal the target and proxy.
      Object.seal(target);
      Object.seal(proxy);

      return proxy;  // nb. if isMethod is true, proxy != this
    };

    scope.Proxy.revocable = function(target, handler) {
      var p = new scope.Proxy(target, handler);
      return {'proxy': p, 'revoke': lastRevokeFn};
    };

    scope.Proxy['revocable'] = scope.Proxy.revocable;
    scope['Proxy'] = scope.Proxy;
  })(global);

  /**
   * Polyfills End
   */

  global.console = Module.require('console');
  global.Buffer = Module.require('buffer');

  var timers = undefined;

  var _timeoutHandler = function(mode) {
    if (timers == undefined) {
      timers = Module.require('timers');
    }
    return timers[mode].apply(this, Array.prototype.slice.call(arguments, 1));
  };

  global.setTimeout = _timeoutHandler.bind(this, 'setTimeout');
  global.setInterval = _timeoutHandler.bind(this, 'setInterval');
  global.clearTimeout = _timeoutHandler.bind(this, 'clearTimeout');
  global.clearInterval = _timeoutHandler.bind(this, 'clearInterval');

  var EventEmitter = Module.require('events').EventEmitter;

  EventEmitter.call(process);

  var keys = Object.keys(EventEmitter.prototype);
  var keysLength = keys.length;
  for (var i = 0; i < keysLength; ++i) {
    var key = keys[i];
    if (!process[key]) {
      process[key] = EventEmitter.prototype[key];
    }
  }

  var nextTickQueue = [];

  process.nextTick = nextTick;
  process._onNextTick = _onNextTick;
  global.setImmediate = setImmediate;

  function _onNextTick() {
    // clone nextTickQueue to new array object, and calls function
    // iterating the cloned array. This is because,
    // during processing nextTick
    // a callback could add another next tick callback using
    // `process.nextTick()`, if we calls back iterating original
    // `nextTickQueue` that could turn into infinite loop.

    var callbacks = nextTickQueue.slice(0);
    nextTickQueue = [];

    var len = callbacks.length;
    for (var i = 0; i < len; ++i) {
      try {
        callbacks[i]();
      } catch (e) {
        process._onUncaughtException(e);
      }
    }

    return nextTickQueue.length > 0;
  }


  function nextTick(callback) {
    var args = Array.prototype.slice.call(arguments);
    args[0] = null;
    nextTickQueue.push(Function.prototype.bind.apply(callback, args));
  }


  function setImmediate(callback) {
    // TODO(Yorkie): use nextTick for now...
    nextTick(callback);
  }

  var module = Module.require('module');

  process._onUncaughtException = _onUncaughtException;
  function _onUncaughtException(error) {
    var event = 'uncaughtException';
    if (error instanceof SyntaxError) {
      error.message = `${error.message} at\n\t ${module.curr}`;
    }

    if (process._events[event] && process._events[event].length > 0) {
      try {
        // Emit uncaughtException event.
        process.emit('uncaughtException', error);
      } catch (e) {
        // Even uncaughtException handler thrown, that could not be handled.
        console.error('uncaughtException handler throws: ' + e);
        process.exit(1);
      }
    } else {
      // Exit if there are no handler for uncaught exception.
      console.error('uncaughtException: ' + error);
      process.exit(1);
    }
  }

  var os = Module.require('os');

  process.uptime = function() {
    return os.uptime();
  };

  process.exitCode = 0;
  process._exiting = false;
  process.emitExit = function(code) {
    if (!process._exiting) {
      process._exiting = true;
      if (code || code == 0) {
        process.exitCode = code;
      }
      process.emit('exit', process.exitCode || 0);
    }
  };

  function updateEnviron() {
    var envs = process._getEnvironArray();
    envs.forEach(function(env, idx) {
      var item = env.split('=');
      var key = item[0];
      var val = item[1];
      process.env[key] = val;
    });
  }
  updateEnviron();

  // compatible with stdout
  process.stdout = {
    isTTY: false,
    write: console._stdout,
  };

  // compatible with stdout
  process.stderr = {
    isTTY: false,
    write: console._stderr,
  };

  // FIXME(Yorkie): the NamedPropertyHandlerConfiguration is not implemented at IoT.js
  process.set = function(key, val) {
    if (key === 'env' || key === 'environ') {
      for (var key in val) {
        process._setEnviron(key, val[key]);
      }
      updateEnviron();
    } else {
      throw new Error('Not supported property');
    }
  };

  process.exit = function(code) {
    try {
      process.emitExit(code);
    } catch (e) {
      process.exitCode = 1;
      process._onUncaughtException(e);
    } finally {
      process.doExit(process.exitCode || 0);
    }
  };

  // TODO(Yorkie): compatible with Node.js
  process.emitWarning = function(warning, type, code, ctor) {
    process.emit('warning', warning, type, code, ctor);
  };

  // TODO(Yorkie): compatible with Node.js
  process.memoryUsage = function() {
    return {
      rss: 0,
      heapTotal: 0,
      heapUsed: 0,
      external: 0,
    };
  };

  var _hrtime = process.hrtime;
  var NANOSECOND_PER_SECONDS = 1e9;
  process.hrtime = function hrtime (time) {
    var curr = _hrtime();
    if (time === null || time === undefined) {
      return curr;
    }
    if (!Array.isArray(time)) {
      var error = new TypeError('[ERR_INVALID_ARG_TYPE]: ' +
        'The "time" argument must be of type Array. Received type ' +
        typeof time);
      error.code = 'ERR_INVALID_ARG_TYPE';
      throw error;
    }
    if (time.length !== 2) {
      var error = new TypeError('[ERR_INVALID_ARRAY_LENGTH]: ' +
        'The array "time" (length ' + String(time.length) + ') ' +
        'must be of length 2.');
      error.code = 'ERR_INVALID_ARRAY_LENGTH';
      throw error;
    }
    var left = curr[0] - time[0];
    var right = curr[1] - time[1];
    if (right < 0) {
      left -= 1;
      right += NANOSECOND_PER_SECONDS;
    }
    return [left, right];
  }

  function setupChannel() {
    // If we were spawned with env NODE_CHANNEL_FD then load that up and
    // start parsing data from that stream.
    if (process.env.NODE_CHANNEL_FD) {
      var fd = parseInt(process.env.NODE_CHANNEL_FD, 10);

      // Make sure it's not accidentally inherited by child processes.
      delete process.env.NODE_CHANNEL_FD;
      var cp = Module.require('child_process');
      cp._forkChild(fd);
    }
  }
  setupChannel();

  var module = Module.require('module');
  module.runMain();
})();

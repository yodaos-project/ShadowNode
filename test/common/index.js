'use strict';
var assert = require('assert');
var mustCallChecks = [];

function mustCall(fn, criteria) {
  if (typeof fn === 'number') {
    criteria = fn;
    fn = noop;
  } else if (fn === undefined) {
    fn = noop;
  }
  if (criteria === undefined) {
    criteria = 1;
  }

  if (typeof criteria !== 'number')
    throw new TypeError(`Invalid value: ${criteria}`);

  var context = {
    expect: criteria,
    actual: 0,
    stack: (new Error()).stack,
    name: fn.name || '<anonymous>'
  };

  if (mustCallChecks.length === 0) process.on('exit', runCallChecks);

  mustCallChecks.push(context);

  return function() {
    ++context.actual;
    return fn.apply(this, arguments);
  };
}

function noop() {}

function runCallChecks() {
  mustCallChecks.forEach((it) => {
    assert.strictEqual(
      it.actual,
      it.expect,
      `Expect function ${it.name} been called ${it.expect} times, \
got ${it.actual}
${it.stack}`);
  });
}

// function Comparison(obj, settings) {
//   for (var key in settings) {
//     if (key in obj)
//       this[key] = obj[key];
//   }
// }

function expectsError(fn, settings, exact) {
  if (typeof fn !== 'function') {
    exact = settings;
    settings = fn;
    fn = undefined;
  }

  function innerFn(error) {
    if (arguments.length !== 1) {
      assert.fail(`Expected one argument, got ${arguments}`);
    }
    var descriptor = Object.getOwnPropertyDescriptor(error, 'message');
    assert.strictEqual(descriptor.enumerable, false);

    var innerSettings = settings;
    if ('type' in settings) {
      var type = settings.type;
      if (type !== Error && !Error.isPrototypeOf(type)) {
        throw new TypeError('`settings.type` must inherit from `Error`');
      }
      var constructor = error.constructor;
      if (constructor.name === 'NodeError' && type.name !== 'NodeError') {
        constructor = Object.getPrototypeOf(error.constructor);
      }
      if (!('type' in error)) {
        error.type = constructor;
      }
    }

    if ('message' in settings &&
        typeof settings.message === 'object' &&
        settings.message.test(error.message)) {
      innerSettings = Object.create(
        settings, Object.getOwnPropertyDescriptors(settings));
      innerSettings.message = error.message;
    }

    // console.log(error);
    // console.log(settings);

    // for (var key in settings) {
    //   console.log(key, error[key], innerSettings[key]);
    //   if (error[key] === innerSettings[key]) {
    //     var a = new Comparison(error, settings);
    //     var b = new Comparison(innerSettings, settings);

    //     var tmpLimit = Error.stackTraceLimit;
    //     Error.stackTraceLimit = 0;
    //     var err = new assert.AssertionError({
    //       actual: a,
    //       expected: b,
    //       operator: 'strictEqual',
    //       stackStartFn: assert.throws
    //     });

    //     Error.stackTraceLimit = tmpLimit;

    //     throw new assert.AssertionError({
    //       actual: error,
    //       expected: settings,
    //       operator: 'common.expectsError',
    //       message: err.message
    //     });
    //   }
    // }

    return true;
  }

  if (fn) {
    assert.throws(fn, innerFn, 'nimabi');
    return;
  }

  return mustCall(innerFn, exact);
}

module.exports = {
  mustCall: mustCall,
  expectsError: expectsError
};

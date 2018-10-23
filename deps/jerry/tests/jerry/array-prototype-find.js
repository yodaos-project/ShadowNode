var tests = [
  function array_altered_during_loop() {
    // Copyright (C) 2015 the V8 project authors. All rights reserved.
    // This code is governed by the BSD license found in the LICENSE file.
    /*---
    esid: sec-array.prototype.find
    es6id: 22.1.3.8
    description: >
      The range of elements processed is set before the first call to `predicate`.
    info: |
      22.1.3.8 Array.prototype.find ( predicate[ , thisArg ] )
    
      ...
      6. If thisArg was supplied, let T be thisArg; else let T be undefined.
      7. Let k be 0.
      8. Repeat, while k < len
        ...
        d. Let testResult be ToBoolean(Call(predicate, T, «kValue, k, O»)).
      ...
    ---*/

    var arr = ['Shoes', 'Car', 'Bike'];
    var results = [];

    arr.find(function (kValue) {
      if (results.length === 0) {
        arr.splice(1, 1);
      }
      results.push(kValue);
    });

    assert.sameValue(results.length, 3, 'predicate called three times');
    assert.sameValue(results[0], 'Shoes');
    assert.sameValue(results[1], 'Bike');
    assert.sameValue(results[2], undefined);

    results = [];
    arr = ['Skateboard', 'Barefoot'];
    arr.find(function (kValue) {
      if (results.length === 0) {
        arr.push('Motorcycle');
        arr[1] = 'Magic Carpet';
      }

      results.push(kValue);
    });

    assert.sameValue(results.length, 2, 'predicate called twice');
    assert.sameValue(results[0], 'Skateboard');
    assert.sameValue(results[1], 'Magic Carpet');
  },
  function predicate_call_parameters() {
    // Copyright (C) 2015 the V8 project authors. All rights reserved.
    // This code is governed by the BSD license found in the LICENSE file.
    /*---
    esid: sec-array.prototype.find
    es6id: 22.1.3.8
    description: >
      Predicate called as F.call( thisArg, kValue, k, O ) for each array entry.
    info: |
      22.1.3.8 Array.prototype.find ( predicate[ , thisArg ] )
    
      ...
      6. If thisArg was supplied, let T be thisArg; else let T be undefined.
      7. Let k be 0.
      8. Repeat, while k < len
        ...
        d. Let testResult be ToBoolean(Call(predicate, T, «kValue, k, O»)).
        e. ReturnIfAbrupt(testResult).
      ...
    ---*/

    var arr = ['Mike', 'Rick', 'Leo'];

    var results = [];

    arr.find(function (kValue, k, O) {
      results.push(arguments);
    });

    assert.sameValue(results.length, 3);

    var result = results[0];
    assert.sameValue(result[0], 'Mike');
    assert.sameValue(result[1], 0);
    assert.sameValue(result[2], arr);
    assert.sameValue(result.length, 3);

    result = results[1];
    assert.sameValue(result[0], 'Rick');
    assert.sameValue(result[1], 1);
    assert.sameValue(result[2], arr);
    assert.sameValue(result.length, 3);

    result = results[2];
    assert.sameValue(result[0], 'Leo');
    assert.sameValue(result[1], 2);
    assert.sameValue(result[2], arr);
    assert.sameValue(result.length, 3);
  },
  function predicate_call_this_non_strict() {
    // Copyright (C) 2015 the V8 project authors. All rights reserved.
    // This code is governed by the BSD license found in the LICENSE file.
    /*---
    esid: sec-array.prototype.find
    es6id: 22.1.3.8
    description: >
      Predicate thisArg as F.call( thisArg, kValue, k, O ) for each array entry.
    info: |
      22.1.3.8 Array.prototype.find ( predicate[ , thisArg ] )
    
      ...
      8. Repeat, while k < len
        ...
        d. Let testResult be ToBoolean(Call(predicate, T, «kValue, k, O»)).
        e. ReturnIfAbrupt(testResult).
      ...
    flags: [noStrict]
    ---*/

    var result;

    [1].find(function (kValue, k, O) {
      result = this;
    });

    assert.sameValue(result, this);

    var o = {};
    [1].find(function () {
      result = this;
    }, o);

    assert.sameValue(result, o);

  },
  function predicate_call_this_strict() {
    // Copyright (C) 2015 the V8 project authors. All rights reserved.
    // This code is governed by the BSD license found in the LICENSE file.
    /*---
    esid: sec-array.prototype.find
    es6id: 22.1.3.8
    description: >
      Predicate thisArg as F.call( thisArg, kValue, k, O ) for each array entry.
    info: |
      22.1.3.8 Array.prototype.find ( predicate[ , thisArg ] )
    
      ...
      8. Repeat, while k < len
        ...
        d. Let testResult be ToBoolean(Call(predicate, T, «kValue, k, O»)).
        e. ReturnIfAbrupt(testResult).
      ...
    flags: [onlyStrict]
    ---*/

    var result;

    [1].find(function (kValue, k, O) {
      result = this;
    });

    assert.sameValue(result, undefined);

    var o = {};
    [1].find(function () {
      result = this;
    }, o);

    assert.sameValue(result, o);

  },
  function predicate_called_for_each_array_property() {
    // Copyright (C) 2015 the V8 project authors. All rights reserved.
    // This code is governed by the BSD license found in the LICENSE file.
    /*---
    esid: sec-array.prototype.find
    es6id: 22.1.3.8
    description: >
      Predicate is called for each array property.
    info: |
      22.1.3.8 Array.prototype.find ( predicate[ , thisArg ] )
    
      ...
      6. If thisArg was supplied, let T be thisArg; else let T be undefined.
      7. Let k be 0.
      8. Repeat, while k < len
        ...
        d. Let testResult be ToBoolean(Call(predicate, T, «kValue, k, O»)).
      ...
    ---*/

    var arr = [undefined, , , 'foo'];
    var called = 0;

    arr.find(function () {
      called++;
    });

    assert.sameValue(called, 4);

  },
  function predicate_is_not_callable_throws() {
    // Copyright (C) 2015 the V8 project authors. All rights reserved.
    // This code is governed by the BSD license found in the LICENSE file.
    /*---
    esid: sec-array.prototype.find
    es6id: 22.1.3.8
    description: >
      Throws a TypeError exception if predicate is not callable.
    info: |
      22.1.3.8 Array.prototype.find ( predicate[ , thisArg ] )
    
      ...
      5. If IsCallable(predicate) is false, throw a TypeError exception.
      ...
    ---*/

    assert.throws(TypeError, function () {
      [].find({});
    });

    assert.throws(TypeError, function () {
      [].find(null);
    });

    assert.throws(TypeError, function () {
      [].find(undefined);
    });

    assert.throws(TypeError, function () {
      [].find(true);
    });

    assert.throws(TypeError, function () {
      [].find(1);
    });

    assert.throws(TypeError, function () {
      [].find('');
    });

    assert.throws(TypeError, function () {
      [].find(1);
    });

    assert.throws(TypeError, function () {
      [].find([]);
    });

    assert.throws(TypeError, function () {
      [].find(/./);
    });

  }
]
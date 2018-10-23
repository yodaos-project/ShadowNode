'use strict';
var assert = require('assert');
var tests = [
  {
    name: 'array-altered-during-loop.js',
    exec: function() {
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

      arr.find(function(kValue) {
        if (results.length === 0) {
          arr.splice(1, 1);
        }
        results.push(kValue);
      });

      assert(results.length, 3 === 'predicate called three times');
      assert(results[0] === 'Shoes');
      assert(results[1] === 'Bike');
      assert(results[2] === undefined);

      results = [];
      arr = ['Skateboard', 'Barefoot'];
      arr.find(function(kValue) {
        if (results.length === 0) {
          arr.push('Motorcycle');
          arr[1] = 'Magic Carpet';
        }

        results.push(kValue);
      });

      assert(results.length, 2 === 'predicate called twice');
      assert(results[0] === 'Skateboard');
      assert(results[1] === 'Magic Carpet');

    }
  },
  {
    name: 'predicate-call-parameters.js',
    exec: function() {
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

      arr.find(function(kValue, k, O) {
        results.push(arguments);
      });

      assert(results.length === 3);

      var result = results[0];
      assert(result[0] === 'Mike');
      assert(result[1] === 0);
      assert(result[2] === arr);
      assert(result.length === 3);

      result = results[1];
      assert(result[0] === 'Rick');
      assert(result[1] === 1);
      assert(result[2] === arr);
      assert(result.length === 3);

      result = results[2];
      assert(result[0] === 'Leo');
      assert(result[1] === 2);
      assert(result[2] === arr);
      assert(result.length === 3);

    }
  },
  {
    name: 'predicate-call-this-non-strict.js',
    exec: function() {
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

      [1].find(function(kValue, k, O) {
        result = this;
      });

      assert(result === this);

      var o = {};
      [1].find(function() {
        result = this;
      }, o);

      assert(result === o);

    }
  },
  {
    name: 'predicate-call-this-strict.js',
    exec: function() {
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

      [1].find(function(kValue, k, O) {
        result = this;
      });

      assert(result === undefined);

      var o = {};
      [1].find(function() {
        result = this;
      }, o);

      assert(result === o);

    }
  },
  {
    name: 'predicate-called-for-each-array-property.js',
    exec: function() {
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

      arr.find(function() {
        called++;
      });

      assert(called === 4);

    }
  },
  {
    name: 'predicate-not-called-on-empty-array.js',
    exec: function() {
      // Copyright (C) 2015 the V8 project authors. All rights reserved.
      // This code is governed by the BSD license found in the LICENSE file.
      /*---
esid: sec-array.prototype.find
es6id: 22.1.3.8
description: >
  Predicate is only called if this.length is > 0.
info: |
  22.1.3.8 Array.prototype.find ( predicate[ , thisArg ] )

  ...
  7. Let k be 0.
  8. Repeat, while k < len
    ...
    d. Let testResult be ToBoolean(Call(predicate, T, «kValue, k, O»)).
  ...
  9. Return undefined.
---*/

      var called = false;

      var predicate = function() {
        called = true;
        return true;
      };

      var result = [].find(predicate);

      assert(called, false === '[].find(predicate) does not call predicate');
      assert(result, undefined === '[].find(predicate) returned undefined');

    }
  },
];

for (var i = 0; i < tests.length; ++i) {
  console.log(`testing ${tests[i].name}`);
  tests[i].exec();
}

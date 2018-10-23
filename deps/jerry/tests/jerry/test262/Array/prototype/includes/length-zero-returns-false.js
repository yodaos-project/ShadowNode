// Copyright (C) 2016 the V8 project authors. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-array.prototype.includes
description: Returns false if length is 0
info: |
  22.1.3.11 Array.prototype.includes ( searchElement [ , fromIndex ] )

  ...
  2. Let len be ? ToLength(? Get(O, "length")).
  3. If len is 0, return false.
  ...
---*/

var calls = 0;
var fromIndex = {
  valueOf: function() {
    calls++;
  }
};

var sample = [];
assert(sample.includes(0) === false);
assert(sample.includes() === false);
assert(sample.includes(0, fromIndex) === false);
assert.sameValue(calls, 0, "length is checked before ToInteger(fromIndex)");

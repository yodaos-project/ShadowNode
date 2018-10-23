// Copyright (C) 2016 the V8 project authors. All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/*---
esid: sec-array.prototype.includes
description: Searches using fromIndex
info: |
  22.1.3.11 Array.prototype.includes ( searchElement [ , fromIndex ] )

  ...
  5. If n ≥ 0, then
    a. Let k be n.
  6. Else n < 0,
    a. Let k be len + n.
    b. If k < 0, let k be 0.
  7. Repeat, while k < len
    a. Let elementK be the result of ? Get(O, ! ToString(k)).
    b. If SameValueZero(searchElement, elementK) is true, return true.
    c. Increase k by 1.
  ...
---*/

var sample = ["a", "b", "c"];
assert(sample.includes("a", 0) === true);
assert(sample.includes("a", 1) === false);
assert(sample.includes("a", 2) === false);

assert(sample.includes("b", 0) === true);
assert(sample.includes("b", 1) === true);
assert(sample.includes("b", 2) === false);

assert(sample.includes("c", 0) === true);
assert(sample.includes("c", 1) === true);
assert(sample.includes("c", 2) === true);

assert(sample.includes("a", -1) === false);
assert(sample.includes("a", -2) === false);
assert(sample.includes("a", -3) === true);
assert(sample.includes("a", -4) === true);

assert(sample.includes("b", -1) === false);
assert(sample.includes("b", -2) === true);
assert(sample.includes("b", -3) === true);
assert(sample.includes("b", -4) === true);

assert(sample.includes("c", -1) === true);
assert(sample.includes("c", -2) === true);
assert(sample.includes("c", -3) === true);
assert(sample.includes("c", -4) === true);

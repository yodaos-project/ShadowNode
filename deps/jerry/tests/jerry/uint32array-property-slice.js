'use strict';

var null_u32array = new Uint32Array();
var u32array = new Uint32Array([1, 2, 3, 4]);

var u32arr1 = u32array.slice();
var u32arr2 = u32array.slice("a", "3");
var u32arr3 = u32array.slice(-2);
var u32arr4 = u32array.slice(Infinity, NaN);
var u32arr5 = u32array.slice(undefined, -3);

assert (null_u32array.toString() === '[object Uint32Array]');

assert (u32arr1.length === 4);
assert (u32arr1[0] === 1);
assert (u32arr1[1] === 2);
assert (u32arr1[2] === 3);
assert (u32arr1[3] === 4);

assert (u32arr2[0] === 1);
assert (u32arr2[1] === 2);
assert (u32arr2[2] === 3);

assert (u32arr3[0] === 3);
assert (u32arr3[1] === 4);

assert (u32arr4.length === 0);

assert (u32arr5[0] === 1);

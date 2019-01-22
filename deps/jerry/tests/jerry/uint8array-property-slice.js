'use strict';

var null_u8array = new Uint8Array();
var u8array = new Uint8Array([1, 2, 3, 4]);

var u8arr1 = u8array.slice();
var u8arr2 = u8array.slice("a", "3");
var u8arr3 = u8array.slice(-2);
var u8arr4 = u8array.slice(Infinity, NaN);
var u8arr5 = u8array.slice(undefined, -3);

assert (null_u8array.toString() === '[object Uint8Array]');

assert (u8arr1.length === 4);
assert (u8arr1[0] === 1);
assert (u8arr1[1] === 2);
assert (u8arr1[2] === 3);
assert (u8arr1[3] === 4);

assert (u8arr2[0] === 1);
assert (u8arr2[1] === 2);
assert (u8arr2[2] === 3);

assert (u8arr3[0] === 3);
assert (u8arr3[1] === 4);

assert (u8arr4.length === 0);

assert (u8arr5[0] === 1);

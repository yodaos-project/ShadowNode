'use strict';

// for Uint8Array
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


// for Uint16Array
var null_u16array = new Uint16Array();
var u16array = new Uint16Array([1, 2, 3, 4]);

var u16arr1 = u16array.slice();
var u16arr2 = u16array.slice("a", "3");
var u16arr3 = u16array.slice(-2);
var u16arr4 = u16array.slice(Infinity, NaN);
var u16arr5 = u16array.slice(undefined, -3);

assert (null_u16array.toString() === '[object Uint16Array]');

assert (u16arr1.length === 4);
assert (u16arr1[0] === 1);
assert (u16arr1[1] === 2);
assert (u16arr1[2] === 3);
assert (u16arr1[3] === 4);

assert (u16arr2[0] === 1);
assert (u16arr2[1] === 2);
assert (u16arr2[2] === 3);

assert (u16arr3[0] === 3);
assert (u16arr3[1] === 4);

assert (u16arr4.length === 0);

assert (u16arr5[0] === 1);


// for Uint32Array
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

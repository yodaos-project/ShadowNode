'use strict';

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

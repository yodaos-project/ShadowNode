"use strict";

var assert = require("assert");

(function NumberIsInteger() {
  assert(!Number.isInteger());
  assert(!Number.isInteger("hello"));
  assert(Number.isInteger(4));
  assert(Number.isInteger(4.0));
  assert(!Number.isInteger(4.1));
  assert(!Number.isInteger(4.000000001));
  assert(!Number.isInteger(NaN));
  assert(!Number.isSafeInteger(Infinity));
})();

(function NumberIsSafeInteger() {
  assert(!Number.isSafeInteger());
  assert(!Number.isSafeInteger("hello"));
  assert(Number.isSafeInteger(4.0));
  assert(!Number.isSafeInteger(4.1));
  assert(!Number.isSafeInteger(4.000000001));
  assert(!Number.isSafeInteger(NaN));
  assert(!Number.isSafeInteger(9007199254740992));
  assert(!Number.isSafeInteger(Infinity));
})();

(function NumberIsFinite() {
  assert(!Number.isFinite());
  assert(!Number.isFinite("hello"));
  assert(Number.isFinite(4.0));
  assert(!Number.isFinite(Infinity));
})();

(function NumberIsFinite() {
  assert(!Number.isNaN());
  assert(!Number.isNaN("hello"));
  assert(!Number.isNaN(4.0));
  assert(Number.isNaN(NaN));
})();

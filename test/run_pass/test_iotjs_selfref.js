var assert = require('assert');

var foo = function bar() {
  assert(foo === bar);
};

foo();

var globalfunc;

function foo(func) {
  globalfunc = func;
}

foo(function bar() {
  assert(globalfunc === bar);
});

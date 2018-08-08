'use strict';


var assert = require('assert');
var binding = require(`./build/Release/binding`);

class Class extends binding.BaseClass {
  varructor() {
    super();
    this.method();
  }
  method() {
    this.ok = true;
  }
}

assert.ok(new Class() instanceof binding.BaseClass);
assert.ok(new Class().ok);
assert.ok(binding.OrdinaryFunction());
assert.ok(
  new binding.Constructor(binding.Constructor) instanceof binding.Constructor);

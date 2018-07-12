var tty = require('tty');
var assert = require('assert');

assert.equal(true, tty.isatty(0));
assert.equal(true, tty.isatty(1));
assert.equal(true, tty.isatty(2));
assert.equal(false, tty.isatty(3));
assert.equal(false, tty.isatty(4));
assert.equal(false, tty.isatty(5));

assert.throws(() => {
  tty.isatty(100000);
}, Error);

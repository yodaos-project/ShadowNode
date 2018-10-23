var assert = require('assert')

var sample = [7, 7, 7, 7];

assert.sameValue(sample.includes(7, 4), false, "length");
assert.sameValue(sample.includes(7, 5), false, "length + 1");
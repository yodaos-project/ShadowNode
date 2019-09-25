'use strict';

var fs = require('fs');
var path = require('path');
var assert = require('assert');
var common = require('../common');

var dstFilePath = path.join(__dirname, '../resources/test2.txt');
var originalContents = fs.readFileSync(dstFilePath);
var buff1 = Buffer.from('IoT');
var buff2 = Buffer.from('.js');

function nop() {}

var stream = fs.createWriteStream(dstFilePath);
stream.write(buff1, nop);
stream.write(buff2, nop);
stream.on('finish', common.mustCall(function onend() {
  var outputs = fs.readFileSync(dstFilePath, 'utf8');
  assert.strictEqual(outputs, 'IoT.js');
  fs.writeFileSync(dstFilePath, originalContents);
}));
stream.end();

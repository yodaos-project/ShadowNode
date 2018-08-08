'use strict';

var { SetImmediate } = require(`./build/Release/test_uv_loop`);

SetImmediate(common.mustCall());

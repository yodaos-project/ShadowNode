'use strict';

var { SetImmediate } = require(`./build/Release/test_uv_loop.node`);

SetImmediate(common.mustCall());

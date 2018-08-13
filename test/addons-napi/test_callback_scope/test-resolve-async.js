'use strict';


var { testResolveAsync } = require(`./build/Release/binding.node`);

testResolveAsync().then(common.mustCall());

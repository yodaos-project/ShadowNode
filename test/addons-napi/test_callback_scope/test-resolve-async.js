'use strict';


var { testResolveAsync } = require(`./build/Release/binding`);

testResolveAsync().then(common.mustCall());

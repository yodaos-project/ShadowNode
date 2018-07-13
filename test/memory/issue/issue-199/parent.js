'use strict';

var child = require('child_process').fork(__dirname + '/child.js', {
  env: {
    isSubprocess: 'true',
  },
});

child.on('message', (data) => {
  // console.log(data.toString());
});


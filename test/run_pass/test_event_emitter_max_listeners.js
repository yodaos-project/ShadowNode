'use strict';

var EventEmitter = require('events');
var events = new EventEmitter();
var common = require('../common');

events.on('maxListeners', common.mustCall());

var throwsObjs = [NaN, -1, 'and even this'];

var obj;
for (var i = 0; i < throwsObjs.length; i++) {
  obj = throwsObjs[i];
  
  common.expectsError(
    function() {
      events.setMaxListeners(obj)
    },
    {
      type: Error,
      message: '1'
      // message: 'The value of "n" is out of range. ' +
      //          `It must be a non-negative number. Received ${obj}`
    }
  );

  // common.expectsError(
  //   function() {
  //     events.defaultMaxListeners = obj
  //   },
  //   {
  //     type: Error,
  //     message: 'The value of "defaultMaxListeners" is out of range. ' +
  //              `It must be a non-negative number. Received ${obj}`
  //   }
  // );
}

events.emit('maxListeners');

'use strict';

var EventEmitter = require('events');
var e = new EventEmitter();
var common = require('../common');

e.on('maxListeners', common.mustCall());

var throwsObjs = [NaN, -1, 'and even this'];

var i = 0, obj;
for (i = 0; i < throwsObjs.length; i++) {
  obj = throwsObjs[i];
  
  common.expectsError(
    () => e.setMaxListeners(obj),
    {
      code: 'ERR_OUT_OF_RANGE',
      type: RangeError,
      message: 'The value of "n" is out of range. ' +
               `It must be a non-negative number. Received ${obj}`
    }
  );

  common.expectsError(
    () => events.defaultMaxListeners = obj,
    {
      code: 'ERR_OUT_OF_RANGE',
      type: RangeError,
      message: 'The value of "defaultMaxListeners" is out of range. ' +
               `It must be a non-negative number. Received ${obj}`
    }
  );
}

e.emit('maxListeners');

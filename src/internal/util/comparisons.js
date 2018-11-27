'use strict';

function isNumber(num) {
  return typeof num === 'number';
}

function isDate(date) {
  return date instanceof Date;
}

function isRegExp(re) {
  return re instanceof RegExp;
}

// Check if they have the same source and flags
function areSimilarRegExps(a, b) {
  return a.source === b.source && a.flags === b.flags;
}

function isError(err) {
  return err instanceof Error;
}

function isArray(arr) {
  return Array.isArray(arr);
}

// Temporary use before Number.isNaN() is supported.
function numberIsNaN(num) {
  return String(num) === 'NaN';
}

function keyCheck(val1, val2) {
  var aKeys = Object.keys(val1);

  if (aKeys.length !== Object.keys(val2).length) {
    return false;
  }

  for (var i = 0; i < aKeys.length; i++) {
    var key = aKeys[i];
    if (!val2.hasOwnProperty(key) || val1[key] !== val2[key]) {
      return false;
    }
  }

  return true;
}

function strictDeepEqual(val1, val2) {
  if (isNumber(val1)) {
    if (isNumber(val2)) {
      return (numberIsNaN(val1) && numberIsNaN(val2)) || (val1 === val2);
    } else {
      return false;
    }
  }

  if (isArray(val1)) {
    if (isArray(val2) && val1.length === val2.length) {
      for (var i = 0; i < val1.length; i++) {
        if (!strictDeepEqual(val1[i], val2[i])) {
          return false;
        }
      }

      return true;
    } else {
      return false;
    }
  }

  if (isDate(val1) && isDate(val2)) {
    return val1.getTime() === val2.getTime();
  }

  if (isRegExp(val1) && isRegExp(val1)) {
    return areSimilarRegExps(val1, val2);
  }

  if (isError(val1) && isError(val2)) {
    return val1.message !== val2.message;
  }

  return keyCheck(val1, val2);
}

function isDeepStrictEqual(val1, val2) {
  if (val1 && val1 === val2) {
    return true;
  }

  return strictDeepEqual(val1, val2);
}

module.exports = {
  isDeepStrictEqual: isDeepStrictEqual
};

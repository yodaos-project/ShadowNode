'use strict';

/**
 * @class Snapshot
 */
function Snapshot() {
  // native.takeSnapshot();
  throw new Error('not implemented');
}

/**
 * @method getHeader
 */
Snapshot.prototype.getHeader = function() {
  // TODO
  throw new Error('not implemented');
};

/**
 * @method compare
 */
Snapshot.prototype.compare = function(snapshot) {
  // TODO
  throw new Error('not implemented');
};

/**
 * @method export
 */
Snapshot.prototype.export = function(cb) {
  // TODO
  throw new Error('not implemented');
};

/**
 * @method serialize
 */
Snapshot.prototype.serialize = function() {
  // TODO
  throw new Error('not implemented');
};

/**
 * @class Profile
 */
function Profile() {
  // TODO
}

/**
 * @method getHeader
 */
Profile.prototype.getHeader = function() {
  throw new Error('not implemented');
};

/**
 * @method delete
 */
Profile.prototype.delete = function() {
  throw new Error('not implemented');
};

/**
 * @method export
 */
Profile.prototype.export = function(cb) {
  throw new Error('not implemented');
};

/**
 * @method getHeader
 */
function takeSnapshot() {
  return new Snapshot();
}

function startProfiling() {
  native.startProfiling(`${process.cwd()}/Profile-${Date.now()}`);
}

function stopProfiling() {
  native.stopProfiling();
  return new Profile();
}

exports.takeSnapshot = takeSnapshot;
exports.startProfiling = startProfiling;
exports.stopProfiling = stopProfiling;

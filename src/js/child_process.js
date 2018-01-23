'use strict';

var Process = native.Process;
var Pipe = require('pipe_wrap').Pipe;
var EventEmitter = require('events').EventEmitter;
var util = require('util');
var net = require('net');

function ChildProcess() {
  EventEmitter.call(this);
  this._closesNeeded = 1;
  this._closesGot = 0;
  this.connected = false;

  this.signalCode = null;
  this.exitCode = null;
  this.killed = false;
  this.spawnfile = null;

  this._handle = new native.Process();
  this._handle.owner = this;
  this._handle.onexit = function(exitCode, signalCode) {
    if (signalCode) {
      this.signalCode = signalCode;
    } else {
      this.exitCode = exitCode;
    }

    if (this.stdin) {
      this.stdin.destroy();
    }

    this._handle.close();
    this._handle = null;

    if (exitCode < 0) {
      var syscall = this.spawnfile ? 'spawn ' + this.spawnfile : 'spawn';
      var err = new Error(syscall + ' with exit code: ' + exitCode);

      if (this.spawnfile)
        err.path = this.spawnfile;

      err.spawnargs = this.spawnargs.slice(1);
      this.emit('error', err);
    } else {
      this.emit('exit', this.exitCode, this.signalCode);
    }
    process.nextTick(flushStdio, this);
    maybeClose(this);
  }.bind(this);
}
util.inherits(ChildProcess, EventEmitter);

function flushStdio(subprocess) {
  var stdio = subprocess.stdio;
  if (stdio == null) return;

  for (var i = 0; i < stdio.length; i++) {
    var stream = stdio[i];
    if (!stream || !stream.readable || stream._readableState.readableListening)
      continue;
    stream.resume();
  }
}

function createSocket(pipe, readable) {
  var s = new net.Socket({ handle: pipe });
  if (readable) {
    s.writable = false;
    s.readable = true;
  } else {
    s.writable = true;
    s.readable = false;
  }
  return s;
}

ChildProcess.prototype.spawn = function(options) {
  var ipc;
  var ipcFd;
  var i;

  if (options === null || typeof options !== 'object') {
    throw new TypeError('ERR_INVALID_ARG_TYPE');
  }

  // If no `stdio` option was given - use default
  var stdio = options.stdio || 'pipe';
  stdio = _validateStdio(stdio, false);

  ipc = stdio.ipc;
  ipcFd = stdio.ipcFd;
  stdio = options.stdio = stdio.stdio;

  if (ipc !== undefined) {
    // Let child process know about opened IPC channel
    if (options.envPairs === undefined)
      options.envPairs = [];
    else if (!Array.isArray(options.envPairs)) {
      throw new TypeError('ERR_INVALID_ARG_TYPE', 'options.envPairs');
    }

    options.envPairs.push('NODE_CHANNEL_FD=' + ipcFd);
  }

  if (typeof options.file !== 'string') {
    throw new TypeError('ERR_INVALID_ARG_TYPE');
  }
  this.spawnfile = options.file;

  if (Array.isArray(options.args))
    this.spawnargs = options.args;
  else if (options.args === undefined)
    this.spawnargs = [];
  else
    throw new TypeError('ERR_INVALID_ARG_TYPE');

  var err = this._handle.spawn(options);
  // TODO: handle error

  this.pid = this._handle.pid;
  for (i = 0; i < stdio.length; i++) {
    var stream = stdio[i];
    if (stream.type === 'ignore') 
      continue;
    
    if (stream.ipc) {
      this._closesNeeded++;
      continue;
    }

    if (stream.handle) {
      // when i === 0 - we're dealing with stdin
      // (which is the only one writable pipe)
      stream.socket = createSocket(this.pid !== 0 ?
        stream.handle : null, i > 0);

      if (i > 0 && this.pid !== 0) {
        this._closesNeeded++;
        stream.socket.on('close', function() {
          maybeClose(this);
        }.bind(this));
      }
    }
  }

  this.stdin = stdio.length >= 1 && stdio[0].socket !== undefined ?
    stdio[0].socket : null;
  this.stdout = stdio.length >= 2 && stdio[1].socket !== undefined ?
    stdio[1].socket : null;
  this.stderr = stdio.length >= 3 && stdio[2].socket !== undefined ?
    stdio[2].socket : null;

  this.stdio = [];

  for (i = 0; i < stdio.length; i++)
    this.stdio.push(stdio[i].socket === undefined ? null : stdio[i].socket);

  return err;
};

function getHandleWrapType(stream) {
  if (stream instanceof Pipe) return 'pipe';
  return false;
}

function _validateStdio(stdio, sync) {
  var ipc;
  var ipcFd;

  // Replace shortcut with an array
  if (typeof stdio === 'string') {
    switch (stdio) {
      case 'ignore': stdio = ['ignore', 'ignore', 'ignore']; break;
      case 'pipe': stdio = ['pipe', 'pipe', 'pipe']; break;
      case 'inherit': stdio = [0, 1, 2]; break;
      default:
        throw new TypeError('ERR_INVALID_OPT_VALUE with stdio=' + stdio);
    }
  } else if (!Array.isArray(stdio)) {
    throw new TypeError('ERR_INVALID_OPT_VALUE');
  }

  // At least 3 stdio will be created
  // Don't concat() a new Array() because it would be sparse, and
  // stdio.reduce() would skip the sparse elements of stdio.
  // See http://stackoverflow.com/a/5501711/3561
  while (stdio.length < 3) stdio.push(undefined);

  // Translate stdio into C++-readable form
  // (i.e. PipeWraps or fds)
  stdio = stdio.reduce(function(acc, stdio, i) {
    function cleanup() {
      for (var i = 0; i < acc.length; i++) {
        if ((acc[i].type === 'pipe' || acc[i].type === 'ipc') && acc[i].handle)
          acc[i].handle.close();
      }
    }

    // Defaults
    if (stdio == null) {
      stdio = i < 3 ? 'pipe' : 'ignore';
    }

    if (stdio === 'ignore') {
      acc.push({ type: 'ignore' });
    } else if (stdio === 'pipe' || typeof stdio === 'number' && stdio < 0) {
      var a = {
        type: 'pipe',
        readable: i === 0,
        writable: i !== 0
      };

      if (!sync) {
        a.handle = new Pipe();
      }

      acc.push(a);
    } else if (stdio === 'ipc') {
      if (sync || ipc !== undefined) {
        // Cleanup previously created pipes
        cleanup();
        if (!sync)
          throw new Error('ERR_IPC_ONE_PIPE');
        else
          throw new Error('ERR_IPC_SYNC_FORK');
      }

      ipc = new Pipe(true);
      ipcFd = i;

      acc.push({
        type: 'pipe',
        handle: ipc,
        ipc: true
      });
    } else if (stdio === 'inherit') {
      acc.push({
        type: 'inherit',
        fd: i
      });
    } else if (typeof stdio === 'number' || typeof stdio.fd === 'number') {
      acc.push({
        type: 'fd',
        fd: typeof stdio === 'number' ? stdio : stdio.fd
      });
    } else if (getHandleWrapType(stdio) || getHandleWrapType(stdio.handle) ||
               getHandleWrapType(stdio._handle)) {
      var handle = getHandleWrapType(stdio) ?
        stdio :
        getHandleWrapType(stdio.handle) ? stdio.handle : stdio._handle;

      acc.push({
        type: 'wrap',
        wrapType: getHandleWrapType(handle),
        handle: handle
      });
    } else if (typeof stdio === 'string') {
      if (!sync) {
        cleanup();
        throw new TypeError('ERR_INVALID_SYNC_FORK_INPUT');
      }
    } else {
      // Cleanup
      cleanup();
      throw new errors.TypeError('ERR_INVALID_OPT_VALUE');
    }
    return acc;
  }, []);

  return { 
    stdio: stdio,
    ipc: ipc,
    ipcFd: ipcFd,
  };
}

function normalizeSpawnArguments(file, args, options) {
  if (typeof file !== 'string' || file.length === 0)
    throw new TypeError('"file" argument must be a non-empty string');

  if (Array.isArray(args)) {
    args = args.slice(0);
  } else if (args !== undefined &&
             (args === null || typeof args !== 'object')) {
    throw new TypeError('Incorrect value of args option');
  } else {
    options = args;
    args = [];
  }

  if (options === undefined)
    options = {};
  else if (options === null || typeof options !== 'object')
    throw new TypeError('"options" argument must be an object');

  // Validate the cwd, if present.
  if (options.cwd != null &&
      typeof options.cwd !== 'string') {
    throw new TypeError('"cwd" must be a string');
  }

  // Validate detached, if present.
  if (options.detached != null &&
      typeof options.detached !== 'boolean') {
    throw new TypeError('"detached" must be a boolean');
  }

  // Validate the uid, if present.
  if (options.uid != null && !Number.isInteger(options.uid)) {
    throw new TypeError('"uid" must be an integer');
  }

  // Validate the gid, if present.
  if (options.gid != null && !Number.isInteger(options.gid)) {
    throw new TypeError('"gid" must be an integer');
  }

  // Validate the shell, if present.
  if (options.shell != null &&
      typeof options.shell !== 'boolean' &&
      typeof options.shell !== 'string') {
    throw new TypeError('"shell" must be a boolean or string');
  }

  // Validate argv0, if present.
  if (options.argv0 != null &&
      typeof options.argv0 !== 'string') {
    throw new TypeError('"argv0" must be a string');
  }

  // Make a shallow copy so we don't clobber the user's options object.
  options = Object.assign({}, options);

  if (options.shell) {
    var command = [file].concat(args).join(' ');

    if (process.platform === 'win32') {
      throw new Error('not supported for windows');
    } else {
      if (typeof options.shell === 'string')
        file = options.shell;
      else if (process.platform === 'android')
        file = '/system/bin/sh';
      else
        file = '/bin/sh';
      args = ['-c', command];
    }
  }

  if (typeof options.argv0 === 'string') {
    args.unshift(options.argv0);
  } else {
    args.unshift(file);
  }

  var env = options.env || process.env;
  var envPairs = [];

  for (var key in env) {
    envPairs.push(key + '=' + env[key]);
  }

  return {
    file: file,
    args: args,
    options: options,
    envPairs: envPairs
  };
}

function maybeClose(subprocess) {
  subprocess._closesGot++;

  if (subprocess._closesGot === subprocess._closesNeeded) {
    subprocess.emit('close', subprocess.exitCode, subprocess.signalCode);
  }
}

exports.ChildProcess = ChildProcess;

exports.fork = function(modulePath /*, args, options*/) {
  // TODO
};

exports.exec = function(command /*, options, callback*/) {
  // TODO
};

exports.spawn = function(/*file, args, options*/) {
  var opts = normalizeSpawnArguments.apply(null, arguments);
  var options = opts.options;
  var child = new ChildProcess();
  child.spawn({
    file: opts.file,
    args: opts.args,
    cwd: options.cwd,
    // TODO(Yorkie): not supported for windows option
    // windowsVerbatimArguments: !!options.windowsVerbatimArguments,
    detached: !!options.detached,
    envPairs: opts.envPairs,
    stdio: options.stdio,
    uid: options.uid,
    gid: options.gid
  });

  return child;
};

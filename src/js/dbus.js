'use strict';

var fs = require('fs');
var sax = require('sax');
var util = require('util');
var EventEmitter = require('events').EventEmitter;
var DBus = native.DBus;
var DBUS_TYPES = {
  'system': 0,
  'session': 1,
};

var _busInstance = null;

function initEnv() {
  try {
    var lines = fs.readFileSync(
      '/var/run/dbus/session').toString('utf8').split('\n');
    lines.forEach(function(line) {
      var rBusAddress = /^DBUS_SESSION_BUS_ADDRESS=/;
      if (rBusAddress.test(line)) {
        process.set('env', {
          'DBUS_SESSION_BUS_ADDRESS': line.replace(rBusAddress, ''),
        });
      }
    });
  } catch (err) {
    console.log('skip dbus session setup');
  }
}

function convertData2Array(data) {
  var args = [];
  for (var key in data) {
    args[parseInt(key, 10)] = data[key];
  }
  return args;
}

/**
 * @class Bus
 * @param {String} name - the bus name
 */
function Bus(name) {
  EventEmitter.call(this);
  this.name = name || 'session';
  this.dbus = new DBus();
  this.dbus.getBus(DBUS_TYPES[this.name]);
  this.dbus.setSignalHandler(this.handleSignal.bind(this));
  this._object = null;
}
util.inherits(Bus, EventEmitter);

/**
 * @method getInterface
 */
Bus.prototype.getInterface = function(serviceName,
                                      objectPath, interfaceName, callback) {
  var self = this;
  self.introspect(serviceName, objectPath, function(err) {
    var iface = self._object.interfaces[interfaceName];
    self.getUniqueServiceName(serviceName, function(err, uniqueName) {
      var hash = uniqueName + ':' + objectPath + ':' + interfaceName;
      self.on(hash, function(item) {
        iface.emit.apply(iface, [item.name].concat(item.args));
      });
      self.addSignalFilter(serviceName, objectPath, interfaceName, function() {
        callback(null, iface);
      });
    });
  });
};

/**
 * @method callMethod
 * @param {String} serviceName
 * @param {String} objectPath
 * @param {String} interfaceName
 * @param {String} member
 * @param {String} signature
 * @param {Array} args
 * @param {Function} callback
 */
Bus.prototype.callMethod = function(serviceName, objectPath,
                                    interfaceName, member,
                                    signature, args, callback) {
  function cb(data) {
    callback.apply(null, [null].concat(convertData2Array(data)));
  }
  this.dbus.callMethod(serviceName,
                       objectPath,
                       interfaceName,
                       member,
                       signature,
                       args,
                       cb);
};

/**
 * @method handleSignal
 */
Bus.prototype.handleSignal = function(sender, objectPath,
                                      interfaceName, signal, data) {
  if (objectPath === '/org/freedesktop/DBus/Local' &&
    interfaceName === 'org.freedesktop.DBus.Local' &&
    signal === 'Disconnected') {
    this.reconnect();
  } else {
    var identify = sender + ':' + objectPath + ':' + interfaceName;
    this.emit(identify, {
      name: signal,
      args: convertData2Array(data),
    });
  }
};

/**
 * @method getUniqueServiceName
 */
Bus.prototype.getUniqueServiceName = function(serviceName, callback) {
  this.callMethod(
    'org.freedesktop.DBus',
    '/',
    'org.freedesktop.DBus',
    'GetNameOwner',
    's',
    [serviceName],
    callback
  );
};

/**
 * @method addSignalFilter
 */
Bus.prototype.addSignalFilter = function(sender, objectPath,
                                         interfaceName, callback) {
  var rule = 'type=\'signal\',sender=\'' +
    sender + '\',interface=\'' +
    interfaceName + '\',path=\'' +
    objectPath + '\'';
  this.dbus.addSignalFilter(rule);
  process.nextTick(function() {
    if (typeof callback === 'function') callback();
  });
};

/**
 * @method reconnect
 */
Bus.prototype.reconnect = function() {
  this.dbus.releaseBus();
  this.dbus.getBus(DBUS_TYPES[this.name]);
};

/**
 * Destroy the current bus instance
 */
Bus.prototype.destroy = function() {
  this.dbus.releaseBus();
  _busInstance = false;
};

/**
 * parse xml to json
 */
function xml2js(buf) {
  var json = {};
  var curr = json;
  var history = [];
  var parser = sax.parser(true);
  parser.onopentag = function(node) {
    if (!Array.isArray(curr)) {
      curr[node.name] = {
        attributes: node.attributes,
        children: [],
      };
      history.push(curr);
      curr = json[node.name].children;
    } else {
      curr.push({
        name: node.name,
        attributes: node.attributes,
        children: [],
      });
      history.push(curr);
      curr = curr[curr.length - 1].children;
    }
  };
  parser.onclosetag = function() {
    curr = history.pop();
  };
  parser.write(buf);
  return json;
}

/**
 * @method introspect
 */
Bus.prototype.introspect = function(serviceName, objectPath, callback) {
  var self = this;
  function ondata(err, text) {
    var object = self._object = {
      path: null,
      interfaces: {}
    };
    if (!text)
      throw new Error('no introspectable found');

    var json = xml2js(text).node;
    object.path = json.attributes.name;

    function readInterfaces(data) {
      // create interface from EventEmitter.
      var iface = object.interfaces[data.attributes.name] = new EventEmitter();
      iface.name = data.attributes.name;
      for (var i = 0; i < data.children.length; i++) {
        readMethod(data.children[i], iface);
      }
    }

    function readMethod(data, iface) {
      var name = data.attributes.name;
      var argsIn = [];
      // var argOut = null;
      for (var i = 0; i < data.children.length; i++) {
        var arg = data.children[i];
        if (arg.attributes.direction === 'in') {
          argsIn.push(arg.attributes.type || 's');
        } else if (arg.attributes.direction === 'out') {
          // argOut = arg.attributes.type || 's';
        }
      }
      iface[name] = function() {
        var max = argsIn.length;
        var args = Array.prototype.slice.call(arguments, 0, max);
        var cb = arguments[argsIn.length];
        self.callMethod(
          serviceName,
          objectPath,
          iface.name,
          name,
          argsIn.join(''),
          args || [],
          cb
        );
      };
    }

    for (var i = 0; i < json.children.length; i++) {
      readInterfaces(json.children[i]);
    }
    callback(null);
  }
  this.callMethod(
    serviceName,
    objectPath,
    'org.freedesktop.DBus.Introspectable',
    'Introspect',
    '',
    [],
    ondata);
};

/**
 * @method getService
 * @param {String} name - the service name
 */
Bus.prototype.getService = function(name) {
  return new Service(this, name);
};

/**
 * @class Service
 * @param {Bus} bus
 * @param {String} name
 */
function Service(bus, name) {
  Object.defineProperty(this, '_bus', {
    value: bus,
    writable: false,
    enumerable: false,
  });
  this._dbus = bus.dbus;
  this._dbus.setMessageHandler(this.handleMessage.bind(this));
  this._dbus.requestName(name);
  this._interfaces = {};
  this._serviceName = name;
  this._objectPath = null;
  this._initIntrospectable();
  this._initProperties();
}

/**
 * @method _initIntrospectable
 */
Service.prototype._initIntrospectable = function() {
  var self = this;
  var iface = self.createInterface('org.freedesktop.DBus.Introspectable');
  iface.addMethod('Introspect', { out: ['s'] }, function(cb) {
    // TODO
    var header = '<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object' +
      ' Introspection 1.0//EN"' +
      ' "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">';
    var contents = '<node name="' + self._objectPath + '">' +
      self.getInterfacesMarkup() + '</node>';
    cb(null, header + contents);
  });
};

/**
 * @method initProperties
 */
Service.prototype._initProperties = function() {
  var iface = this.createInterface('org.freedesktop.DBus.Properties');
  var args = {
    getter: {
      in: ['s', 's'],
      out: ['v']
    },
    setter: {
      in: ['s', 's', 'v']
    }
  };
  function getter(iface, prop, cb) {
    // TODO
  }
  function setter(iface, prop, value, cb) {
    // TODO
  }
  iface.addMethod('Get', args.getter, getter);
  iface.addMethod('Set', args.setter, setter);
};

/**
 * @method getInterfacesMarkup
 */
Service.prototype.getInterfacesMarkup = function() {
  var str = '';
  for (var name in this._interfaces) {
    var iface = this._interfaces[name];
    str += '<interface name="' + name + '">' +
      iface.getMethodsMarkup() +
      iface.getSignalsMarkup() +
    '</interface>';
  }
  return str;
};

/**
 * @method handleMessage
 * @param {String} sender
 * @param {String} objectPath
 * @param {String} iface
 * @param {String} member
 * @param {Object} data
 */
Service.prototype.handleMessage = function(sender, objectPath,
                                           iface, member, data) {
  this._interfaces[iface].makeCall(member, data);
};

/**
 * @method createObject
 * @param {String} path
 */
Service.prototype.createObject = function(path) {
  this._dbus.registerObjectPath(path);
  this._objectPath = path;
  return {
    createInterface: this.createInterface.bind(this)
  };
};

/**
 * @method createInterface
 * @param {String} name
 */
Service.prototype.createInterface = function(name) {
  this._interfaces[name] = new ServiceInterface(this._dbus, name, this);
  return this._interfaces[name];
};

/**
 * @class ServiceInterface
 * @param {DBus} dbus
 * @param {String} name
 */
function ServiceInterface(dbus, name, service) {
  this._dbus = dbus;
  this._name = name;
  this._service = service;
  this._methods = {};
  this._properties = {};
  this._signals = {};
}

/**
 * @method makeCall
 * @param {String} member
 * @param {Object} data
 */
ServiceInterface.prototype.makeCall = function(member, data) {
  var self = this;
  var metadata = self._methods[member];
  if (!metadata || typeof metadata.handler !== 'function')
    throw new TypeError(`Invalid caller, ${member} is not callable`);

  var args = convertData2Array(data);
  args.push(function(err, response) {
    if (err) throw err;
    var outSig = (metadata.opts.out || []).join('');
    self._dbus.sendMessageReply(data, response, outSig);
  });
  metadata.handler.apply(self, args);
};

/**
 * @method getMethodsMarkup
 */
ServiceInterface.prototype.getMethodsMarkup = function() {
  var str = '';
  for (var member in this._methods) {
    var idx = 0;
    var method = this._methods[member];
    var argsIn = method.opts.in || [];
    var argsOut = method.opts.out || [];
    str += '<method name="' + member + '">';
    for (idx = 0; idx < argsIn.length; idx++) {
      str += '<arg direction="in" type="' + argsIn[idx] + '"/>';
    }
    for (idx = 0; idx < argsOut.length; idx++) {
      str += '<arg direction="out" type="' + argsOut[idx] + '"/>';
    }
    str += '</method>';
  }
  return str;
};

/**
 * @method getSignalsMarkup
 */
ServiceInterface.prototype.getSignalsMarkup = function() {
  var str = '';
  for (var member in this._signals) {
    var idx = 0;
    var signal = this._signals[member];
    var types = signal.opts.types || [];
    str += '<signal name="' + member + '">';
    for (idx = 0; idx < types.length; idx++) {
      str += '<arg type="' + types[idx] + '"/>';
    }
    str += '</signal>';
  }
  return str;
};

/**
 * @method addMethod
 * @param {String} name
 * @param {Object} opts
 * @param {Array} opts.in
 * @param {Array} opts.out
 * @param {Function} handler
 */
ServiceInterface.prototype.addMethod = function(name, opts, handler) {
  this._methods[name] = {
    opts: opts,
    handler: handler,
  };
};

/**
 * @method addProperty
 */
ServiceInterface.prototype.addProperty = function() {
  // TODO no supported
};

/**
 * @method addSignal
 */
ServiceInterface.prototype.addSignal = function(name, opts) {
  this._signals[name] = {
    name: name,
    opts: opts,
  };
};

/**
 * @method emit
 */
ServiceInterface.prototype.emit = function(name, val) {
  var objectPath = this._service._objectPath;
  var iface = this._name;
  var signal = this._signals[name];
  if (!signal) {
    throw new Error('signal ' + name + ' are not found.');
  }
  var types = signal.opts.types || [];
  // TODO(Yorkie): only support 1 argument for signal
  this._dbus.emitSignal(objectPath,
                        iface, signal.name, types.join(''), val);
};

/**
 * @method update
 */
ServiceInterface.prototype.update = function() {
  // Do nothing, just for comp.
};

/**
 * @module dbus
 * @method getBus
 * @param {String} name
 */
function getBus(name) {
  if (_busInstance === false) {
    throw new Error('dbus connection has been destroyed');
  }
  if (_busInstance === null) {
    _busInstance = new Bus(name);
  }
  return _busInstance;
}

/**
 * @module dbus
 * @method registerService
 * @param {String} name
 * @param {String} service
 */
function registerService(name, service) {
  return getBus(name).getService(service);
}

/**
 * @module dbus
 * @method Define
 * @param {String} type
 */
exports.Define = function(type) {
  if (type === String) {
    return 's';
  } else if (type === 'Number') {
    return 'i';
  } else {
    return type;
  }
};

initEnv();
exports.getBus = getBus;
exports.registerService = registerService;

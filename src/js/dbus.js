'use strict';

var DBus = native.DBus;
var DBUS_TYPES = {
  'system': 0,
  'session': 1,
};

/**
 * @class Bus
 * @param {String} name - the bus name
 */
function Bus(name) {
  this.dbus = new DBus();
  this.dbus.getBus(DBUS_TYPES[name]);
}

/**
 * @method getInterface
 */
Bus.prototype.getInterface = function(serviceName, objectPath) {
  this.introspect(serviceName, objectPath);
};

/**
 * @method introspect
 */
Bus.prototype.introspect = function(serviceName, objectPath, callback) {
  function ondata(err, data) {
    console.log(err, data);
  }
  this.dbus.callMethod(
    serviceName, 
    objectPath, 
    'org.freedesktop.DBus.Introspectable', 
    'Introspect', 
    '', [], ondata);
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
    var header = '<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN" "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">';
    var contents = 
      '<node name="' + self._objectPath + '">' + self.getInterfacesMarkup() + '</node>';
    cb(null, header + contents);
  });
};

/**
 * @method initProperties
 */
Service.prototype._initProperties = function() {
  var iface = this.createInterface('org.freedesktop.DBus.Properties');
  iface.addMethod('Get', { in: ['s', 's'], out: ['v'] }, function(iface, prop, cb) {
    // TODO
  });
  iface.addMethod('Set', { in: ['s', 's', 'v'] }, function(iface, prop, value, cb) {
    // TODO
  });
};

/**
 * @method getInterfacesMarkup
 */
Service.prototype.getInterfacesMarkup = function() {
  var str = '';
  for (var name in this._interfaces) {
    var iface = this._interfaces[name];
    str += '<interface name="' + name + '">' + iface.getMethodsMarkup() + '</interface>';
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
Service.prototype.handleMessage = function(sender, objectPath, iface, member, data) {
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
  this._interfaces[name] = new ServiceInterface(this._dbus, name);
  return this._interfaces[name];
};

/**
 * @class ServiceInterface
 * @param {DBus} dbus
 * @param {String} name
 */
function ServiceInterface(dbus, name) {
  this._dbus = dbus;
  this._methods = {};
  this._properties = {};
}

/**
 * @method makeCall
 * @param {String} member
 * @param {Object} data
 */
ServiceInterface.prototype.makeCall = function(member, data) {
  var self = this;
  var metadata = self._methods[member];
  var args = [];
  for (var key in data) {
    args[parseInt(key)] = data[key];
  }
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
ServiceInterface.prototype.addSignal = function() {
  // TODO no supported
};

/**
 * @module dbus
 * @method getBus
 * @param {String} name
 */
function getBus(name) {
  return new Bus(name);
}

/**
 * @module dbus
 * @method registerService
 * @param {String} name
 * @param {String} service
 */
function registerService(name, service) {
  var bus = new Bus(name);
  return bus.getService(service);
}

exports.getBus = getBus;
exports.registerService = registerService;

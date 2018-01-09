'use strict';

var fs = require('fs');
var sax = require('sax');
var DBus = native.DBus;
var DBUS_TYPES = {
  'system': 0,
  'session': 1,
};

function initEnv() {
  try {
    var lines = fs.readFileSync('/var/run/dbus/session', 'utf8').split('\n');
    lines.forEach(function(line) {
      var rBusAddress = /^DBUS_SESSION_BUS_ADDRESS=/;
      if (rBusAddress.test(line)) {
        process.env.DBUS_SESSION_BUS_ADDRESS = line.replace(rBusAddress, '');
      }
    });
  } catch (err) {
    console.log('skip dbus session setup');
  }
}

/**
 * @class Bus
 * @param {String} name - the bus name
 */
function Bus(name) {
  this.dbus = new DBus();
  this.dbus.getBus(DBUS_TYPES[name]);
  this._object = null;
}

/**
 * @method getInterface
 */
Bus.prototype.getInterface = function(serviceName, objectPath, interfaceName, callback) {
  var self = this;
  self.introspect(serviceName, objectPath, function(err) {
    callback(null, self._object.interfaces[interfaceName]);
  });
};

/**
 * parse xml to json
 */
function xml2js(buf) {
  var json = {};
  var curr = json;
  var history = [];
  var parent = null;
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
  function ondata(data) {
    var object = self._object = {
      path: null,
      interfaces: {}
    };
    if (!data || !data['0'])
      throw new Error('no introspectable found');

    var json = xml2js(data['0']).node;
    object.path = json.attributes.name;

    function readInterfaces(data) {
      var iface = object.interfaces[data.attributes.name] = {
        name: data.attributes.name,
      };
      for (var i = 0; i < data.children.length; i++) {
        readMethod(data.children[i], iface);
      }
    }

    function readMethod(data, iface) {
      var name = data.attributes.name;
      var argsIn = [];
      var argOut = null;
      for (var i = 0; i < data.children.length; i++) {
        var arg = data.children[i];
        if (arg.attributes.direction === 'in') {
          argsIn.push(arg.attributes.type || 's');
        } else if (arg.attributes.direction === 'out') {
          argOut = arg.attributes.type || 's';
        }
      }
      iface[name] = function() {
        var max = argsIn.length - 1;
        var args = Array.prototype.slice.call(arguments, 0, max);
        var cb = arguments[argsIn.length];
        self.dbus.callMethod(
          serviceName,
          objectPath,
          iface.name,
          name,
          argsIn.join(''),
          [],
          function(res) {
            cb(null, res['0'], res['1']);
          }
        );
      };
    }

    for (var i = 0; i < json.children.length; i++) {
      readInterfaces(json.children[i]);
    }
    callback(null);
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

initEnv();
exports.getBus = getBus;
exports.registerService = registerService;

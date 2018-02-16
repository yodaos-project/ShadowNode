# OS

The `os` module provides a number of operating system-related utility methods. It can be accessed using:

```js
var os = require('os');
```

### os.freemem()

* Returns {Number}

The `os.freemem()` method returns the amount of free system memory in bytes as an integer.

### os.totalmem()

* Returns {Number}

The `os.totalmem()` method returns the amount of total system memory in bytes as an integer.

### os.hostname()

* Returns {String}

The `os.hostname()` method returns the hostname of the operating system as a string.

### os.networkInterfaces()

* Returns {Object}

The `os.networkInterfaces()` method returns an object containing only network interfaces that have been assigned a network address.

Each key on the returned object identifies a network interface. The associated value is an array of objects that each describe an assigned network address.

The properties available on the assigned network address object include:

* `address` {String} The assigned IPv4 or IPv6 address
* `netmask` {String} The IPv4 or IPv6 network mask
* `family` {String} Either IPv4 or IPv6
* `mac` {String} The MAC address of the network interface
* `internal` {Boolean} true if the network interface is a loopback or similar interface that is not remotely accessible; otherwise false
* `scopeid` {Number} The numeric IPv6 scope ID (only specified when family is IPv6)
* `cidr` {String} The assigned IPv4 or IPv6 address with the routing prefix in CIDR notation. If the netmask is invalid, this property is set to null

```js
{
  lo: [
    {
      address: '127.0.0.1',
      netmask: '255.0.0.0',
      family: 'IPv4',
      mac: '00:00:00:00:00:00',
      internal: true,
      cidr: '127.0.0.1/8'
    },
    {
      address: '::1',
      netmask: 'ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff',
      family: 'IPv6',
      mac: '00:00:00:00:00:00',
      internal: true,
      cidr: '::1/128'
    }
  ],
  eth0: [
    {
      address: '192.168.1.108',
      netmask: '255.255.255.0',
      family: 'IPv4',
      mac: '01:02:03:0a:0b:0c',
      internal: false,
      cidr: '192.168.1.108/24'
    },
    {
      address: 'fe80::a00:27ff:fe4e:66a1',
      netmask: 'ffff:ffff:ffff:ffff::',
      family: 'IPv6',
      mac: '01:02:03:0a:0b:0c',
      internal: false,
      cidr: 'fe80::a00:27ff:fe4e:66a1/64'
    }
  ]
}
```
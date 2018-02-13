# ShadowNode

The Node.js Runtime in shadow, which supports NPM packages partially for embeddable platform.

[![License](https://img.shields.io/badge/licence-Apache%202.0-brightgreen.svg?style=flat)](LICENSE)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FRokid%2Fshadow-node.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2FRokid%2Fshadow-node?ref=badge_shield)
[![IRC Channel](https://img.shields.io/badge/chat-on%20freenode-brightgreen.svg)](https://kiwiirc.com/client/irc.freenode.net/#shadow-node)

## Quick Start

This project is another runtime for your [Node.js][] app, which is to be used in low memory devices. It's inspired and forked
from the original awesome project [Samsung/iotjs][].

To get started with shadow-node, you should follow the [build](#build) step on a Linux or OSX machine. And this project 
will not be supported on Windows unfortunately.

> Memory usage and Binary footprint are measured at [here](https://samsung.github.io/js-remote-test) with real target daily.

### Getting Source Code

```sh
$ git clone https://github.com/Rokid/ShadowNode.git
$ cd shadow-node
```

### Build

```sh
$ tools/build.py --no-check-test
```

To get supported options, run:

```sh
$ tools/build.py --help
```

### Test

```sh
$ build/x86_64-linux/debug/bin/iotjs tools/check_test.js
```

For Additional information see [Getting Started](docs/Getting-Started.md).

## What's different with Node.js

The [shadow-node][]'s purpose is not going to run the absolute same code both on the [Node.js][] 
and [shadow-node][], within embeddable devices might be severe and less memory requirement, you should 
write your apps in an light thinking. However we are hoping to share the same community for developers 
as possible, and the following are the status:

- [x] [Assert](docs/api/IoT.js-API-Assert.md)
- [x] [Buffer](docs/api/IoT.js-API-Buffer.md)
- [x] [Child Process](docs/api/IoT.js-API-Child-Process.md)
- [x] [Crypto](docs/api/IoT.js-API-Crypto.md)
- [x] [DNS](docs/api/IoT.js-API-DNS.md)
- [x] [Events](docs/api/IoT.js-API-Events.md)
- [x] [File System](docs/api/IoT.js-API-File-System.md)
- [x] [HTTP](docs/api/IoT.js-API-HTTP.md)
- [x] [Module](docs/api/IoT.js-API-Module.md)
- [x] [Net](docs/api/IoT.js-API-Net.md)
- [x] [OS](docs/api/IoT.js-API-OS.md)
- [x] [Process](docs/api/IoT.js-API-Process.md)
- [x] [Stream](docs/api/IoT.js-API-Stream.md)
- [x] [Timers](docs/api/IoT.js-API-Timers.md)
- [x] [TLS](docs/api/IoT.js-API-TLS.md)
- [x] [UDP/Datagram](docs/api/IoT.js-API-DGRAM.md)
- [x] [Zlib](docs/api/IoT.js-API-Zlib.md)

In desktop and embedable ecosystem, the service via `DBus` is quite normal, in shadow-node, the `DBus`
will be the built-in module. For detailed API, see [IoT.js-API-DBUS](docs/api/IoT.js-API-DBUS.md).

The `MQTT` protocol is used for communication between IoT devices, then [shadow-node][] would support
this protocol natively, and we keep the API consistent with the pop library [MQTT.js][]. See 
[IoT.js-API-MQTT](docs/api/IoT.js-API-MQTT.md) for details.

For hardware geek, this project benefits from the upstream [IoT.js][], which has supported the 
following hardware interfaces, you are able to port [shadow-node][] to your platform(s) and 
start hacking with JavaScript:

- [ADC](IoT.js-API-ADC.md)
- [BLE](IoT.js-API-BLE.md)
- [GPIO](IoT.js-API-GPIO.md)
- [I2C](IoT.js-API-I2C.md)
- [PWM](IoT.js-API-PWM.md)
- [SPI](IoT.js-API-SPI.md)
- [UART](IoT.js-API-UART.md)

## Documentation

- [Getting Started](docs/Getting-Started.md)
- [API Reference](docs/api/IoT.js-API-reference.md)

## License

[shadow-node][] is Open Source software under the [Apache 2.0 license][].
Complete license and copyright information can be found within the code.

[shadow-node]: https://github.com/Rokid/shadow-node
[Node.js]: https://github.com/nodejs/node
[Iot.js]: https://github.com/Samsung/iotjs
[Samsung/iotjs]: https://github.com/Samsung/iotjs
[MQTT.js]: https://github.com/mqttjs/MQTT.js
[Apache 2.0 license]: https://www.apache.org/licenses/LICENSE-2.0

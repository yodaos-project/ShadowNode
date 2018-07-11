# ShadowNode

The Node.js Runtime in shadow, which supports NPM packages partially for embeddable platform.

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/7c70ae6d6c5d4d778d18ccd70f11bd0d)](https://app.codacy.com/app/yorkie/ShadowNode?utm_source=github.com&utm_medium=referral&utm_content=Rokid/ShadowNode&utm_campaign=badger)
[![Build Status](https://travis-ci.org/Rokid/ShadowNode.svg?branch=master)](https://travis-ci.org/Rokid/ShadowNode)
[![License](https://img.shields.io/badge/licence-Apache%202.0-brightgreen.svg?style=flat)](LICENSE)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FRokid%2Fshadow-node.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2FRokid%2Fshadow-node?ref=badge_shield)

Currently supports the following platform:

- [x] OSX
- [x] Linux

## Quick Start

This project is another runtime for your [Node.js][] app, which is to be used in low memory devices. It's inspired and forked
from the original awesome project [Samsung/iotjs][].

To get started with shadow-node, you should follow the [build](#build) step on a Linux or OSX machine. And this project 
will not be supported on Windows unfortunately.

> Memory usage and Binary footprint are measured at [here](https://samsung.github.io/js-remote-test) with real target daily.

### Getting Source Code

```sh
$ git clone https://github.com/Rokid/ShadowNode.git
$ cd ShadowNode
```

### Build

```sh
$ npm run build
```

To get supported options, run:

```sh
$ tools/build.py --help
```

### Install

```sh
$ tools/build.py --install
```

### Test

```sh
$ npm test
```

For Additional information see [Getting Started](docs/Getting-Started.md).

## What's different with Node.js

The [ShadowNode][]'s purpose is not going to run the absolute same code both on the [Node.js][] 
and [ShadowNode][], within embeddable devices might be severe and less memory requirement, you should 
write your apps in an light thinking. However we are hoping to share the same community for developers 
as possible, and the following are the status:

- [Assert](docs/api/Assert.md)
- [Buffer](docs/api/Buffer.md)
- [Child Process](docs/api/Child-Process.md)
- [Crypto](docs/api/Crypto.md)
- [DNS](docs/api/DNS.md)
- [Events](docs/api/Events.md)
- [File System](docs/api/File-System.md)
- [HTTP](docs/api/HTTP.md)
- [Module](docs/api/Module.md)
- [Net](docs/api/Net.md)
- [OS](docs/api/OS.md)
- [Process](docs/api/Process.md)
- [Stream](docs/api/Stream.md)
- [Timers](docs/api/Timers.md)
- [TLS](docs/api/TLS.md)
- [UDP/Datagram](docs/api/DGRAM.md)
- [Zlib](docs/api/Zlib.md)

In desktop and embedable ecosystem, the service via `DBus` is quite normal, in shadow-node, the `DBus`
will be the built-in module. For detailed API, see [DBUS API](docs/api/DBUS.md).

The `MQTT` protocol is used for communication between IoT devices, then [ShadowNode][] would support
this protocol natively, and we keep the API consistent with the pop library [MQTT.js][]. See 
[MQTT API](docs/api/MQTT.md) for details.

The `WebSocket` is the popular protocol in IoT environment as well, and also supported by [ShadowNode][]
natively. See [WebSocket API](docs/api/WebSocket.md).

For hardware geek, this project benefits from the upstream [IoT.js][], which has supported the 
following hardware interfaces, you are able to port [shadow-node][] to your platform(s) and 
start hacking with JavaScript:

- [ADC](docs/api/ADC.md)
- [BLE](docs/api/BLE.md)
- [GPIO](docs/api/GPIO.md)
- [I2C](docs/api/I2C.md)
- [PWM](docs/api/PWM.md)
- [SPI](docs/api/SPI.md)
- [UART](docs/api/UART.md)

## Documentation

- [Getting Started](docs/Getting-Started.md)
- [API Reference](docs/api/README.md)

## License

[ShadowNode][] is Open Source software under the [Apache 2.0 license][].
Complete license and copyright information can be found within the code.

[ShadowNode]: https://github.com/Rokid/shadow-node
[Node.js]: https://github.com/nodejs/node
[Iot.js]: https://github.com/Samsung/iotjs
[Samsung/iotjs]: https://github.com/Samsung/iotjs
[MQTT.js]: https://github.com/mqttjs/MQTT.js
[Apache 2.0 license]: https://www.apache.org/licenses/LICENSE-2.0

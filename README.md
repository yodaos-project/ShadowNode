# shadow-node

The Node.js Runtime in shadow, which supports NPM packages partially for embeddable platform.

[![License](https://img.shields.io/badge/licence-Apache%202.0-brightgreen.svg?style=flat)](LICENSE)
[![Build Status](https://travis-ci.org/Samsung/iotjs.svg?branch=master)](https://travis-ci.org/Samsung/iotjs)
[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/12140.svg)](https://scan.coverity.com/projects/samsung-iotjs)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FSamsung%2Fiotjs.svg?type=shield)](https://app.fossa.io/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FSamsung%2Fiotjs?ref=badge_shield)
[![IRC Channel](https://img.shields.io/badge/chat-on%20freenode-brightgreen.svg)](https://kiwiirc.com/client/irc.freenode.net/#iotjs)

Memory usage and Binary footprint are measured at [here](https://samsung.github.io/js-remote-test) with real target daily.

The following table shows the latest results on the devices:

|      Artik053         | [![Remote Testrunner](https://samsung.github.io/js-remote-test/status/artik053.svg)](https://samsung.github.io/js-remote-test/?view=artik053)  |
|        :---:          |                                             :---:                                                                                                |
| **Raspberry Pi 2**    | [![Remote Testrunner](https://samsung.github.io/js-remote-test/status/rpi2.svg)](https://samsung.github.io/js-remote-test/?view=rpi2)          |
| **STM32F4-Discovery** | [![Remote Testrunner](https://samsung.github.io/js-remote-test/status/stm32f4dis.svg)](https://samsung.github.io/js-remote-test/?view=stm32)   |

## Quick Start
### Getting the sources

```bash
git clone https://github.com/Rokid/shadow-node.git
cd shadow-node
```

### How to Build

```bash
tools/build.py
```

### How to Test

```bash
build/x86_64-linux/debug/bin/iotjs tools/check_test.js
```


For Additional information see [Getting Started](docs/Getting-Started.md).

## Documentation

- [Getting Started](docs/Getting-Started.md)
- [API Reference](docs/api/IoT.js-API-reference.md)

## License

shadow-node is Open Source software under the [Apache 2.0 license](https://www.apache.org/licenses/LICENSE-2.0). 
Complete license and copyright information can be found within the code.
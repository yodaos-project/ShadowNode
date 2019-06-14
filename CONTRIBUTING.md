# Contributing to ShadowNode

## Overview
ShadowNode is one of the Node.js runtimes for edge device, support for most [Node.js API](https://nodejs.org/en/docs/) such as `Event`, `Module` and `N-API`. We plan to support the most of packages in NPM for Node.js in the future. If you are interested in joining it, we will welcome it very much.

## Getting and Building
Firstly, you can only build it in Linux and macOS now.
### Prerequisites
  - The tools needed to build ShadowNode from scratch:
    - Standard C development headers
    - Git 1.9+
    - CMake 3.1+
    - Node.js and NPM
    - DBus(built in on ubuntu)

Note that at least 4GB of RAM is required to build from source and run tests.

## Building ShadowNode
You can have a look at the `package.json` file and get the build script.
```shell
$ npm run build
```

On macOS, you may need specify included directory of `DBus` by flag `--external-include-dir`.
```shell
$ tools/build.py --clean --external-include-dir /yourdbus/dir/include/dbus-1.0 --external-include-dir /yourdbus/dir/dbus-1.0/include # you need replace it to your dbus dir.
```
The executable file will be generated in `<ShadowNode>/build/x86_64-darwin/debug/bin`.

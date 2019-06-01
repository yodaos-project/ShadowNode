# Contributing to ShadowNode

## Overview
ShadowNode is one of the Node.js runtimes in shadow, support for most [Node.js API](https://nodejs.org/en/docs/) such as `Event`, `Module` and `N-API`. We plan to support the most of packages in NPM for Node.js in the future. If you are interested in joining it, we will welcome it very much.

## Getting and Building
Firstly, you can only build it in Linux and MaxOS now.
1. Install the following prerequisites, as necessary:
  - The tools needed to build ShadowNode from scratch:
    - Standard C development headers
    - Git 1.9+
    - CMake 3.1+
    - Node.js and NPM
    - DBus(Ubuntu is Builted-in)

Note that at least 4GB of RAM is required to build from source and run tests.

2. Get the ShadowNode code:
```shell
$ git clone https://github.com/yodaos-project/ShadowNode.git
```

3. Run Build
You can have a look at the `package.json` file and get the build script.
```shell
$ npm run build # tools/build.py
```

And if you are in MacOS and install the `DBus` yourself, you need appoint the include-dir of `DBus` by the `--external-include-dir` flag:
```shell
$ tools/build.py --clean --external-include-dir /yourdbus/dir/include/dbus-1.0 --external-include-dir /yourdbus/dir/dbus-1.0/include # you need replace it to your dbus dir.
```
The executable file will generated in `ShadowNode/build/x86_64-darwin/debug/bin`.

### Overall steps to build for Android
1. Get the sources
2. Build all at once
3. Execute IoT.js
4. Clean build directory

***

#### Build Host
Ubuntu 14.04 is recommended. Other Unix like platforms can be used. If it doesn't seem to work properly on other platforms, please look into the [Issues](https://github.com/Samsung/iotjs/issues) page. Someone may have already tried. If you can't find any related one, please leave an issue for help.

#### Directory structure

This document assumes 'harmony' as the root directory. _JerryScript_, _libtuv_ are included as sub-modules in `deps` directory.

* harmony
    * iotjs
        * deps
            * http-parser
            * jerry
            * libtuv


※ harmony? It's from the initial code name of our project. (_Sounds good, isn't it? :)_)

#### Prerequisite

You need to install some packages to build IoT.js, as follows;

```
sudo apt-get install gyp cmake build-essential valgrind
```

gcc compiler 4.8 or higher versions are required to compile. If you don't know how to do it, you can get some help from [how-to-install-gcc-4-8](http://askubuntu.com/questions/271388/how-to-install-gcc-4-8) or google.

### 1. Get the sources

Clone our repository to look around and test it. If it attracts you and you want to try something interested, please fork it.

To get the source for this repository,
```
cd harmony
git clone https://github.com/Samsung/iotjs.git
cd iotjs
```

Sub-modules(_http-parser_, _JerryScript_, _libuv_ and _libtuv_) will be pulled. And matching hash will be checked out for your current IoT.js version when you run the build script.


### 2. Build all at once

IoT.js and required sub-modules are generated all at once in tools directory with build.py.

```sh
cd iotjs
NDK_DIR=$HOME/Android/sdk/ndk-bundle # or change to your ndk directory
./tools/build.py \
  --target-arch=aarch64 \
  --target-os=linux-android \
  --cmake-param=-DANDROID_NDK=$NDK_DIR \
  --cmake-param=-DANDROID_NATIVE_API_LEVEL=23 \
  --cmake-param=-DANDROID_ABI=arm64-v8a \
  --cmake-param=-DANDROID_TOOLCHAIN=clang \
  --cmake-toolchain-file=$NDK_DIR/build/cmake/android.toolchain.cmake\
  --profile=./profiles/basic.profile
```


#### Set build options
Some basic options are provided.

Existing build options are listed as follows;
```
buildtype=debug|release (debug is default)
builddir=build (build is default)
clean
buildlib (default is False)
profile=path-to-profile (default: profiles/default.profile)
target-arch=x86|x86_64|x64|i686|arm|aarch64 (depends on your host platform)
target-os=linux|linux-android|nuttx|darwin|osx (linux is default)
target-board
cmake-param
compile-flag
link_flag
external-include-dir
external-lib
jerry-cmake-param
jerry-compile-flag
jerry-link-flag
jerry-lto
jerry-heap-section
jerry-heaplimit (default is 81, may change)
jerry-memstat (default is False)
no-init-submodule (default is init)
no-check-tidy (default is check)
no-check-test (default is check)
no-parallel-build
no-snapshot
nuttx-home= (no default value)
```

To give options, please use two dashes '--' before the option name as described in the following sections.

Options that may need explanations.
* builddir: compile intermediate and output files are generated here.
* buildlib: generating _iotjs_ to a library if True(e.g. for NuttX). give __--buildlib__ to make it True.
* jerry-heaplimit: JerryScript default heap size (as of today) is 256Kbytes. This option is to change the size for embedded systems, NuttX for now, and current default is 81KB. For linux, this has no effect. While building nuttx if you see an error `region sram overflowed by xxxx bytes`, you may have to decrease about that amount.
* jerry-memstat: turn on the flag so that jerry dumps byte codes and literals and memory usage while parsing and execution.
* no-check-tidy: no checks codes are tidy. we recommend to check tidy.
* no-check-test: do not run all tests in test folder after build.
* nuttx-home: it's NuttX platform specific, to tell where the NuttX configuration and header files are.

If you want to know more details about options, please check the [Build Script](Build-Script.md) page.


#### Include extended module
There are two ways to include [extended module](../api/IoT.js-API-reference.md).

The first way is to specify the `ENABLE_MODULE_[NAME]=ON` CMake parameter, where `[NAME]` is the uppercase name of the module.

```
./tools/build.py --cmake-param=-DENABLE_MODULE_DGRAM=ON
```

The second way is by using profile descriptors, where a profile file contains the list of enabled modules. E.g.:

**my-profile**
```
ENABLE_MODULE_IOTJS_CORE_MODULES
ENABLE_MODULE_IOTJS_BASIC_MODULES
ENABLE_MODULE_DGRAM
```


```sh
./tools/build.py --profile=./my-profile
```


#### Options example

It's a good practice to build in separate directory, like 'build'. IoT.js generates all outputs into separate **'build'** directory. You can change this by --builddir option. Usually you won't need to use this option. Target and architecture name are used as a name for a directory inside 'build' directory.

To build debug version, type the command like below. And you can find the binary in 'output' directory.
```
./tools/build.py --builddir=output
```

To build 32bit version in x86_64 with debug version as a library, type the command like below.
```
./tools/build.py --target-arch=i686 --buildlib
```

To build release version with different jerry revision, type the command like below. (Assume that you have already checked it out.)
```
./tools/build.py --buildtype=release --no-init-submodule
```



#### Build only IoT.js with given build option

This section for explaining how to build only IoT.js when you did some modification. IoT.js uses [CMake](http://www.cmake.org/) for makefile generation. You can go inside the build folder and build with 'make' command. Go inside where your target platform name is, for example x86_64 linux,
```
cd build/x86_64-linux/release/iotjs
make
```

#### What build script does

1. It will clone sub-modules, this will be done only once when version hash has not changed.
2. Checkout matching version for each sub-modules.
3. Build sub-modules, you can see the outputs at build/(target-arch)-(target-os)/(buildtype)/libs folder.
4. Build IoT.js


### 3. Execute IoT.js

Executable name is **'iotjs'** and resides in (target-arch)-(target-os)/(buildtype)/iotjs.
To run greetings JavaScript in test folder, for example;

```
./build/aarch64-linux-android/debug/bin/iotjs ./test/run_pass/test_console.js
```

#### Set execution Options

Some execution options are provided as follows;
```
memstat
show-opcodes
```

To give options, please use two dashes '--' before the option name as described in following sections.

For more details on options, please see below.
* memstat: dump memory statistics. To get this, must build with __jerry-memstat__ option.
* show-opcodes: print compiled byte-code.


#### Options example

To print memory statistics, follow the below steps;
```
./tools/build.py --jerry-memstat

./build/aarch64-linux-android/debug/bin/iotjs --memstat ./test/run_pass/test_console.js
```

With given `show-opcodes` option, opcodes will be shown.
```
./build/aarch64-linux-android/debug/bin/iotjs --show-opcodes ./test/run_pass/test_console.js
```

### 4. Clean build directory

Just remove the folder as follows;
```
rm -rf build
```

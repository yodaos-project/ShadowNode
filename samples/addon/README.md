# Native Addon

This tutorial shows how to write a native module.

## Requirements

- CMake

## Steps

### Write the `CMakeLists.txt` under your project

```cmake
cmake_minimum_required(VERSION 2.8.12)
project(mybinding C)

set(ADDON_HEADERS 
  ../../include/
  ../../src/
  ../../deps/jerry/jerry-core/include
  ../../deps/libtuv/include
)

add_library(mybinding MODULE binding.c)
set_target_properties(mybinding PROPERTIES
  PREFIX "" 
  SUFFIX ".node" 
  OUTPUT_NAME "binding"
  LINK_FLAGS "-undefined dynamic_lookup"
)
target_include_directories(mybinding PUBLIC ${ADDON_HEADERS})
```

Be notice the following:

- must include the `deps/jerry/include`, `src` and `include`, if you are working with async handle by libtuv, please include `deps/libtuv/include`.
- keep the following `PROPERTIES`:
  - PREFIX to be ""
  - SUFFIX to be ".node", we keep consistent with Node.js as well
  - Add the link flag "-undefined dynamic_lookup", which let the build system not check the
    symbols when linking to library.

Note: the "-undefined dynamic_lookup" might be unsafe as I noticed, so be carefully when you
are writing your native module.

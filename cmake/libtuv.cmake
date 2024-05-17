# Copyright 2015-present Samsung Electronics Co., Ltd. and other contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8)

# Configure external libtuv
set(DEPS_TUV deps/libtuv)
set(DEPS_TUV_SRC ${ROOT_DIR}/${DEPS_TUV})

ExternalProject_Add(libtuv
  PREFIX ${DEPS_TUV}
  SOURCE_DIR ${DEPS_TUV_SRC}
  BUILD_IN_SOURCE 0
  BINARY_DIR ${DEPS_TUV}
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}
    -DCMAKE_TOOLCHAIN_ROOT=${CMAKE_TOOLCHAIN_ROOT}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DTARGET_PLATFORM=${PLATFORM_DESCRIPTOR}
    -DLIBTUV_CUSTOM_LIB_OUT=lib
    -DBUILDTESTER=NO
    -DBUILDAPIEMULTESTER=NO
    -DTARGET_SYSTEMROOT=${TARGET_SYSTEMROOT}
    -DTARGET_BOARD=${TARGET_BOARD}
    -DANDROID_ABI=${ANDROID_ABI}
)
add_library(tuv STATIC IMPORTED)
add_dependencies(tuv libtuv)
set_property(TARGET tuv PROPERTY
  IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libtuv.a)
set_property(DIRECTORY APPEND PROPERTY
  ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/lib/libtuv.a)
set(TUV_INCLUDE_DIR ${DEPS_TUV_SRC}/include)
set(TUV_LIBS tuv)

if(${TARGET_OS} STREQUAL "LINUX")
  list(APPEND TUV_LIBS pthread)
endif()

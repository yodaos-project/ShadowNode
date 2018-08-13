cmake_minimum_required(VERSION 2.8.12)
project(napi_test C)

add_library(napi_test MODULE napi.test.c)
set(CMAKE_C_FLAGS "-Os")

set_target_properties(napi_test PROPERTIES
  PREFIX ""
  SUFFIX ".node"
  OUTPUT_NAME "napi_test"
  LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set_target_properties(napi_test PROPERTIES
    LINK_FLAGS "-undefined dynamic_lookup")
else()
  set_target_properties(napi_test PROPERTIES
    LINK_FLAGS "-rdynamic")
endif()

message("project source dir ${PROJECT_SOURCE_DIR}")
include_directories(${PROJECT_SOURCE_DIR}/../../include)

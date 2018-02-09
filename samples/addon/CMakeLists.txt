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

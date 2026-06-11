cmake_minimum_required(VERSION 3.22)

project(column-engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CAMKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wpedantic")

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_RLE "Enable RLE encoding for columns" ON)
option(ENABLE_DELTA "Enable Delta encoding for integer columns" ON)
option(ENABLE_DICT "Enable Dictionary encoding for string columns" ON)
option(ENABLE_LZ4     "Enable LZ4 block compression"                   ON)
option(ENABLE_BITPACK "Enable Bit-Packing after RLE/Delta/Dict encoding" ON)

include(FetchContent)

if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(googletest)
endif()

add_subdirectory(src)
add_subdirectory(exe)

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

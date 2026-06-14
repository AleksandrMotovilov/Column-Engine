cmake_minimum_required(VERSION 3.22)

project(column-engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CAMKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function -Wpedantic")

option(BUILD_TESTS "Build tests" ON)
option(ENABLE_DELTA_INT     "Enable Delta for integer columns"                OFF)
option(ENABLE_RLE_INT       "Enable RLE for integer columns"                   ON)
option(ENABLE_RLE_FLOAT     "Enable RLE for float columns"                     ON)
option(ENABLE_RLE_STR       "Enable RLE for string columns"                    ON)
option(ENABLE_DICT_INT      "Enable Dict for integer columns"                  ON)
option(ENABLE_DICT_FLOAT    "Enable Dict for float columns"                    ON)
option(ENABLE_DICT_STR      "Enable Dict for string columns"                   ON)
option(ENABLE_BITPACK_INT   "Enable Bit-Packing for integers"                  ON)
option(ENABLE_BITPACK_FLOAT "Enable Bit-Packing for floats"                    ON)
option(ENABLE_BITPACK_STR   "Enable Bit-Packing for strings"                   ON)
option(ENABLE_LZ4           "Enable LZ4 block compression"                     ON)
option(ENABLE_ADAPTIVE      "Enable adaptive per-column compression selection" ON)

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

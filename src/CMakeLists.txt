add_library(objects
    kernel/types.cpp
    kernel/column_utils.cpp
    kernel/schema.cpp
    kernel/batch.cpp
)
target_include_directories(objects PUBLIC ${CMAKE_SOURCE_DIR})

add_library(encoding
    compression/encoding.cpp
    compression/encoding_bitpack.cpp
    compression/encoding_lz4.cpp
    compression/encoding_base.cpp
)
target_include_directories(encoding PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(encoding PUBLIC objects)

set(_ENCODING_DEFS)
foreach(_FLAG RLE_INT RLE_FLOAT RLE_STR DELTA_INT DICT_INT DICT_FLOAT DICT_STR
              LZ4 BITPACK_INT BITPACK_FLOAT BITPACK_STR ADAPTIVE)
  if(ENABLE_${_FLAG})
    list(APPEND _ENCODING_DEFS ENABLE_${_FLAG})
  endif()
endforeach()
target_compile_definitions(encoding PUBLIC ${_ENCODING_DEFS})

find_library(LZ4_LIB lz4 REQUIRED)
find_path(LZ4_INCLUDE_DIR lz4.h REQUIRED)
target_link_libraries(encoding PUBLIC ${LZ4_LIB})
target_include_directories(encoding PUBLIC ${LZ4_INCLUDE_DIR})

add_library(reader_writer_clmn convertion/reader_writer_clmn.cpp)
target_include_directories(reader_writer_clmn PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(reader_writer_clmn PUBLIC objects encoding)

add_library(reader_writer_csv convertion/reader_writer_csv.cpp)
target_include_directories(reader_writer_csv PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(reader_writer_csv PUBLIC objects)

add_library(converter_from_csv_to_clmn convertion/from_csv_to_clmn.cpp)
target_include_directories(converter_from_csv_to_clmn PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(converter_from_csv_to_clmn PUBLIC reader_writer_csv reader_writer_clmn)

add_library(converter_from_clmn_to_csv convertion/from_clmn_to_csv.cpp)
target_include_directories(converter_from_clmn_to_csv PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(converter_from_clmn_to_csv PUBLIC reader_writer_clmn reader_writer_csv)

add_library(execution
    execution/expressions.cpp
    execution/aggregation_functions.cpp
    execution/filter_operator.cpp
    execution/project_operator.cpp
    execution/scan_operator.cpp
    execution/global_aggregation_operator.cpp
    execution/group_by_aggregation_operator.cpp
    execution/top_k_operator.cpp
    execution/sort_operator.cpp
    execution/limit_operator.cpp
    execution/write_operator.cpp
)
find_library(RE2_LIB re2 REQUIRED)
find_path(RE2_INCLUDE_DIR re2/re2.h REQUIRED)
target_include_directories(execution PUBLIC ${CMAKE_SOURCE_DIR} ${RE2_INCLUDE_DIR})
target_link_libraries(execution PUBLIC reader_writer_clmn ${RE2_LIB})

target_compile_definitions(execution PUBLIC GROUP_BY_MAP_${GROUP_BY_MAP})

find_package(Boost 1.81 REQUIRED)
target_include_directories(execution PUBLIC ${Boost_INCLUDE_DIRS})

find_package(absl REQUIRED)
target_link_libraries(execution PUBLIC absl::flat_hash_map absl::node_hash_map)

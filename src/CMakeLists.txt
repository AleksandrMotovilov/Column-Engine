add_library(objects
    kernel/types.cpp
    kernel/column.cpp
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

if(ENABLE_RLE_INT)
  target_compile_definitions(encoding PUBLIC ENABLE_RLE_INT)
endif()
if(ENABLE_RLE_FLOAT)
  target_compile_definitions(encoding PUBLIC ENABLE_RLE_FLOAT)
endif()
if(ENABLE_RLE_STR)
  target_compile_definitions(encoding PUBLIC ENABLE_RLE_STR)
endif()
if(ENABLE_DELTA_INT)
  target_compile_definitions(encoding PUBLIC ENABLE_DELTA_INT)
endif()
if(ENABLE_DICT_INT)
  target_compile_definitions(encoding PUBLIC ENABLE_DICT_INT)
endif()
if(ENABLE_DICT_FLOAT)
  target_compile_definitions(encoding PUBLIC ENABLE_DICT_FLOAT)
endif()
if(ENABLE_DICT_STR)
  target_compile_definitions(encoding PUBLIC ENABLE_DICT_STR)
endif()
find_library(LZ4_LIB lz4 REQUIRED)
find_path(LZ4_INCLUDE_DIR lz4.h REQUIRED)
target_link_libraries(encoding PUBLIC ${LZ4_LIB})
target_include_directories(encoding PUBLIC ${LZ4_INCLUDE_DIR})
if(ENABLE_LZ4)
  target_compile_definitions(encoding PUBLIC ENABLE_LZ4)
endif()
if(ENABLE_BITPACK_INT)
  target_compile_definitions(encoding PUBLIC ENABLE_BITPACK_INT)
endif()
if(ENABLE_BITPACK_FLOAT)
  target_compile_definitions(encoding PUBLIC ENABLE_BITPACK_FLOAT)
endif()
if(ENABLE_BITPACK_STR)
  target_compile_definitions(encoding PUBLIC ENABLE_BITPACK_STR)
endif()
if(ENABLE_ADAPTIVE)
  target_compile_definitions(encoding PUBLIC ENABLE_ADAPTIVE)
endif()

add_library(reader_writer_clmn kernel/reader_writer_clmn.cpp)
target_include_directories(reader_writer_clmn PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(reader_writer_clmn PUBLIC objects encoding)

add_library(reader_writer_csv kernel/reader_writer_csv.cpp)
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
    execution/filter_operators.cpp
    execution/project_operators.cpp
    execution/scan_operators.cpp
    execution/global_aggregation_operators.cpp
    execution/group_by_aggregation_operators.cpp
    execution/top_k_operators.cpp
    execution/sort_operators.cpp
    execution/limit_operators.cpp
    execution/write_operators.cpp
)
find_library(RE2_LIB re2 REQUIRED)
find_path(RE2_INCLUDE_DIR re2/re2.h REQUIRED)
target_include_directories(execution PUBLIC ${CMAKE_SOURCE_DIR} ${RE2_INCLUDE_DIR})
target_link_libraries(execution PUBLIC reader_writer_clmn ${RE2_LIB})

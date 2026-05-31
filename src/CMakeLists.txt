add_library(objects
    kernel/types.cpp
    kernel/column.cpp
    kernel/column_utils.cpp
    kernel/schema.cpp
    kernel/batch.cpp
)
target_include_directories(objects PUBLIC ${CMAKE_SOURCE_DIR})

add_library(reader_writer_clmn kernel/reader_writer_clmn.cpp)
target_include_directories(reader_writer_clmn PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(reader_writer_clmn PUBLIC objects)

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
target_include_directories(execution PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(execution PUBLIC reader_writer_clmn)

add_executable(csv_to_clmn convert.cpp)
target_link_libraries(csv_to_clmn converter_from_csv_to_clmn)
target_include_directories(csv_to_clmn PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(run_query run_query.cpp)
target_link_libraries(run_query converter_from_clmn_to_csv execution)
target_include_directories(run_query PUBLIC ${CMAKE_SOURCE_DIR})

find_package(Threads)
add_executable(tst_normalization
                     test.cpp
                     common.h
                     common.cpp
                     )
target_compile_definitions(tst_normalization PRIVATE -DTEST_FILE="${CMAKE_SOURCE_DIR}/ucd/NormalizationTest.txt")

target_link_libraries(tst_normalization cedilla)

target_include_directories(tst_normalization PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/catch2/include
)

add_test(tst_normalization tst_normalization)

target_link_libraries(tst_normalization gcov)
setup_target_for_coverage(NAME tst_normalization_coverage DEPENDENCIES tst_normalization EXECUTABLE tst_normalization)

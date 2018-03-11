cmake_minimum_required(VERSION 3.5)


#find_package(Boost 1.66.0 REQUIRED PATHS /home/cor3ntin/dev/boost_1_66_0/)
find_package(PythonInterp 3 REQUIRED)

include_directories(/home/cor3ntin/dev/boost_1_66_0 include ${CMAKE_BINARY_DIR}/generated/include range-v3/include/ tests/include)

foreach(val RANGE 0 14)
    list(APPEND GENERATED_NORMALIZATIONS_TEST_CPP "${CMAKE_BINARY_DIR}/generated/test_decomposition_${val}.cpp")
endforeach()

SET(CMAKE_CXX_FLAGS "-O0")
SET(CMAKE_C_FLAGS "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")
set(CMAKE_CXX_STANDARD 17)

add_custom_command(OUTPUT
    ${GENERATED_NORMALIZATIONS_TEST_CPP}
    ${CMAKE_BINARY_DIR}/generated/decomposition.cpp
    ${CMAKE_BINARY_DIR}/generated/include/ucd/details/decomposition_data.h
                   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                   COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/gen_normalization_table.py
                   DEPENDS
                        ${PROJECT_SOURCE_DIR}/gen_normalization_table.py
                        ${PROJECT_SOURCE_DIR}/tpl/decomposition.cpp.tpl
                        ${PROJECT_SOURCE_DIR}/tpl/decomposition_data.h.tpl
                        ${PROJECT_SOURCE_DIR}/tpl/test_decomposition.cpp.tpl
                        ${PROJECT_SOURCE_DIR}/ucd/ucd.all.flat.xml
                        ${PROJECT_SOURCE_DIR}/ucd/NormalizationTest.txt
                   )

add_library(ucdcpp SHARED
    gen_normalization_table.py
    tpl/decomposition_data.h.tpl
    tpl/decomposition.cpp.tpl
    ${CMAKE_BINARY_DIR}/generated/decomposition.cpp
)


add_executable(tst_normalization
    ${GENERATED_NORMALIZATIONS_TEST_CPP}
    tests/tst_normalization.cpp
    tpl/test_decomposition.cpp.tpl
)
target_link_libraries(tst_normalization ucdcpp)

#set_property(GLOBAL PROPERTY JOB_POOLS two_jobs=2)
#set_property(TARGET tst_normalization PROPERTY JOB_POOL_COMPILE two_jobs)

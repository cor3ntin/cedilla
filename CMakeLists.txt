project(cedilla)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
include(CodeCoverage)

find_package(Boost)

find_package(PythonInterp 3 REQUIRED)

foreach(val RANGE 0 37)
    list(APPEND GENERATED_NORMALIZATIONS_TEST_CPP "${CMAKE_BINARY_DIR}/generated/test_decomposition_${val}.cpp")
endforeach()


if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ucd/ucd.all.flat.xml)
  message("Fetching unicode data")
  file(DOWNLOAD "ftp://ftp.unicode.org/Public/10.0.0/ucdxml/ucd.all.flat.zip" ${CMAKE_CURRENT_SOURCE_DIR}/ucd/ucd.all.flat.zip)
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvf ucd.all.flat.zip WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ucd)
endif()
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ucd/NormalizationTest.txt)
  message("Fetching unicode tests")
  file(DOWNLOAD "ftp://ftp.unicode.org/Public/10.0.0/ucd/NormalizationTest.txt" ${CMAKE_CURRENT_SOURCE_DIR}/ucd/NormalizationTest.txt)
endif()



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
                        ${PROJECT_SOURCE_DIR}/tpl/test_decomposition.cpp.tpl
                        ${PROJECT_SOURCE_DIR}/ucd/ucd.all.flat.xml
                        ${PROJECT_SOURCE_DIR}/ucd/NormalizationTest.txt
                   )

#APPEND_COVERAGE_COMPILER_FLAGS()

add_library(cedilla STATIC
    gen_normalization_table.py
    tpl/decomposition.cpp.tpl
    ${CMAKE_BINARY_DIR}/generated/decomposition.cpp
    include/cedilla/normalization.hpp
    include/cedilla/detail/hangul.hpp
    include/cedilla/detail/normalization_view.hpp
    include/cedilla/detail/unicode_base_view.hpp
    third_party/text_view-range-v3/src/error_status.cpp
)
target_link_libraries(cedilla)


target_include_directories(cedilla PUBLIC
    ${Boost_INCLUDE_DIRS}
    include
    ${CMAKE_BINARY_DIR}/generated/include
    third_party/range-v3/include
    third_party/text_view-range-v3/include
)

add_subdirectory(benchmark)

#add_executable(tst_normalization
#    ${GENERATED_NORMALIZATIONS_TEST_CPP}
#    tests/tst_normalization.cpp
#    tpl/test_decomposition.cpp.tpl
#)
#add_test(tst_normalization tst_normalization)
#target_link_libraries(tst_normalization ucdcpp gcov)
#setup_target_for_coverage(NAME tst_normalization_coverage DEPENDENCIES tst_normalization EXECUTABLE tst_normalization)

#set_property(GLOBAL PROPERTY JOB_POOLS two_jobs=2)
#set_property(TARGET tst_normalization PROPERTY JOB_POOL_COMPILE two_jobs)

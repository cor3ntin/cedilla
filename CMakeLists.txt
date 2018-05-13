project(cedilla)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
include(CodeCoverage)

find_package(Boost)

find_package(PythonInterp 3 REQUIRED)

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

add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/generated/normalization_data.cpp
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/gen_normalization_table.py
        DEPENDS
            ${PROJECT_SOURCE_DIR}/gen_normalization_table.py
            ${PROJECT_SOURCE_DIR}/tpl/normalization_data.cpp.tpl
            ${PROJECT_SOURCE_DIR}/ucd/ucd.all.flat.xml
        )

#APPEND_COVERAGE_COMPILER_FLAGS()

add_library(cedilla STATIC
    gen_normalization_table.py
    tpl/normalization_data.cpp.tpl
    ${CMAKE_BINARY_DIR}/generated/normalization_data.cpp
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

add_subdirectory(tests)

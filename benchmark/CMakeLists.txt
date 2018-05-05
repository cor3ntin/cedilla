find_package(Threads)
add_executable(bench bench.cpp
                     unilib/unilib/uninorms.cpp
                     unilib/unilib/uninorms.h
                     ogonek/src/ucd_data.c++

                     )
target_include_directories(bench PRIVATE "nonius/include" "ogonek/include" "ogonek/deps/taussig/include" "ogonek/deps/wheels/include")
target_compile_definitions(bench PRIVATE -DOGONEK_DETAIL_BOOST_HASH_UNICODE_HPP)
target_link_libraries(bench ucdcpp Threads::Threads)

include(FetchContent)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

file(GLOB test_files 
*_test.cpp
*_tests.cpp
*_test.cc
*_tests.cc
test_*.cpp
test_*.cc
)
add_executable(romplers_tests
    ${test_files}
)

target_link_libraries(romplers_tests PRIVATE doctest::doctest)
target_include_directories(romplers_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_features(romplers_tests PRIVATE cxx_std_23)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(romplers_tests)

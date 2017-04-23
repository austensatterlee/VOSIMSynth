cmake_minimum_required(VERSION 3.3)
project(VOSIMLib_tests)

set(CATCH_ROOT ${LIBS_DIR}/catch)

file(GLOB TEST_FILES *.cpp)
add_executable(vosimlib_tests ${TEST_FILES})
target_include_directories(vosimlib_tests PRIVATE ${CATCH_ROOT})
target_link_libraries(vosimlib_tests VOSIMLib)

enable_testing()
add_test(NAME run_tests COMMAND vosimlib_tests)

cmake_minimum_required(VERSION 3.3)
project(VOSIMLib_tests)

set(NONIUS_ROOT ${LIBS_DIR}/nonius)

##
# Unit test sources
##
file(GLOB TEST_FILES *.cpp)
include_directories(${NONIUS_ROOT})
add_executable(vosimlib_tests ${TEST_FILES})
target_link_libraries(vosimlib_tests VOSIMLib)

##
# Benchmark sources
##
add_executable(vosimlib_bench bench/bench.cpp)
target_link_libraries(vosimlib_bench VOSIMLib)
target_include_directories(vosimlib_bench PRIVATE ${NONIUS_ROOT})

if(WIN32)
  target_compile_definitions(vosimlib_bench PRIVATE -DWIN32_LEAN_AND_MEAN)
endif()

cmake_minimum_required(VERSION 3.3)
project(VOSIMLib-tests)

set(NONIUS_ROOT ${LIBS_DIR}/nonius CACHE PATH "")

##
# Unit test sources
##
add_executable(tests ${VOSIMLIB_ROOT}/tests/tests.cpp)
target_link_libraries(tests VOSIMLib)
target_include_directories(tests PRIVATE ${VOSIMLIB_ROOT}/tests/include)

##
# Benchmark sources
##
add_executable(bench ${VOSIMLIB_ROOT}/tests/bench.cpp)
target_link_libraries(bench VOSIMLib)
target_include_directories(bench PRIVATE ${NONIUS_ROOT})

if(WIN32)
  target_compile_definitions(tests PRIVATE -DWIN32_LEAN_AND_MEAN)
  target_compile_definitions(bench PRIVATE -DWIN32_LEAN_AND_MEAN)
endif()

cmake_minimum_required(VERSION 3.3)
project(VOSIMLib_bench)

set(NONIUS_ROOT ${LIBS_DIR}/nonius)

file(GLOB BENCH_FILES *.cpp)
add_executable(vosimlib_bench ${BENCH_FILES})
target_include_directories(vosimlib_bench PRIVATE ${NONIUS_ROOT})
target_link_libraries(vosimlib_bench VOSIMLib)

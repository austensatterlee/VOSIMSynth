cmake_minimum_required(VERSION 3.3)
project(VOSIMLib_bench)

set(NONIUS_ROOT ${LIBS_DIR}/nonius)

set(BENCH_FILES bench.cpp bench.h)
add_executable(vosimlib_bench ${BENCH_FILES})
target_include_directories(vosimlib_bench PRIVATE ${NONIUS_ROOT})
target_link_libraries(vosimlib_bench VOSIMLib)

set(PROFILE_FILES profile.cpp bench.h)
add_executable(vosimlib_profile ${PROFILE_FILES})
target_link_libraries(vosimlib_profile VOSIMLib)

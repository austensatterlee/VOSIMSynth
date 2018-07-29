cmake_minimum_required(VERSION 3.3)
project(VOSIMSynth_tests)

set(CATCH_ROOT ${LIBS_DIR}/catch)

file(GLOB TEST_FILES *.cpp)
add_executable(vosimsynth_tests ${TEST_FILES})
target_include_directories(vosimsynth_tests PRIVATE ${CATCH_ROOT})
target_link_libraries(vosimsynth_tests VOSIMSynth)

add_test(NAME vosimsynth_tests COMMAND vosimsynth_tests)

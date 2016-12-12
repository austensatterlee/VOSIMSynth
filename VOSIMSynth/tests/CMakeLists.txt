cmake_minimum_required(VERSION 3.3)
project(VOSIMLib_tests)

FILE(GLOB TEST_FILES *.cpp *.h)
add_executable(vosimsynth_tests ${TEST_FILES})

target_link_libraries(vosimsynth_tests VOSIMSynth)

if(WIN32)
  target_compile_definitions(vosimlib_bench PRIVATE -DWIN32_LEAN_AND_MEAN)
endif()

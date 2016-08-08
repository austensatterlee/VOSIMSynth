cmake_minimum_required(VERSION 3.3)
project(VOSIMLib)

if(WIN32)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

##
# VOSIMLib source files
##
file(GLOB VOSIMLIB_HDRS ${VOSIMLIB_ROOT}/include/*.h)
file(GLOB VOSIMLIB_SRCS ${VOSIMLIB_ROOT}/src/*.cpp)
file(GLOB VOSIMLIB_UNIT_HDRS ${VOSIMLIB_ROOT}/units/include/*.h)
file(GLOB VOSIMLIB_UNIT_SRCS ${VOSIMLIB_ROOT}/units/src/*.cpp)
set(VOSIMLIB_FILES ${VOSIMLIB_HDRS};${VOSIMLIB_UNIT_HDRS})
set(VOSIMLIB_FILES ${VOSIMLIB_FILES};${VOSIMLIB_SRCS};${VOSIMLIB_UNIT_SRCS})

source_group(VOSIMLib FILES ${VOSIMLIB_SRCS})
source_group(VOSIMLib/include FILES ${VOSIMLIB_HDRS})
source_group(VOSIMLib-units FILES ${VOSIMLIB_UNIT_SRCS})
source_group(VOSIMLib-units/include FILES ${VOSIMLIB_UNIT_HDRS})

include_directories(${VOSIMLIB_ROOT}/include ${VOSIMLIB_ROOT}/units/include)

add_library(VOSIMLib STATIC ${VOSIMLIB_FILES})

add_executable(unit_tests ${VOSIMLIB_ROOT}/tests/serialization.cpp)
target_link_libraries(unit_tests VOSIMLib)

add_executable(bench ${VOSIMLIB_ROOT}/tests/bench.cpp)
target_link_libraries(bench VOSIMLib)
target_include_directories(bench PRIVATE ${NONIUS_ROOT})

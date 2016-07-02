cmake_minimum_required(VERSION 3.3)
project(VOSIMLib)

##
# VOSIMLib source files
##
file(GLOB VOSIMLIB_HDRS ${VOSIMLIB_DIR}/include/*.h)
file(GLOB VOSIMLIB_SRCS ${VOSIMLIB_DIR}/src/*.cpp)
file(GLOB VOSIMLIB_UNIT_HDRS ${VOSIMLIB_DIR}/units/include/*.h)
file(GLOB VOSIMLIB_UNIT_SRCS ${VOSIMLIB_DIR}/units/src/*.cpp)
set(VOSIMLIB_FILES ${VOSIMLIB_HDRS};${VOSIMLIB_UNIT_HDRS})
set(VOSIMLIB_FILES ${VOSIMLIB_FILES};${VOSIMLIB_SRCS};${VOSIMLIB_UNIT_SRCS})

include_directories(${VOSIMLIB_DIR}/include)
include_directories(${VOSIMLIB_DIR}/units/include)

source_group(VOSIMLib FILES ${VOSIMLIB_SRCS})
source_group(VOSIMLib\\include FILES ${VOSIMLIB_HDRS})
source_group(VOSIMLib-units FILES ${VOSIMLIB_UNIT_SRCS})
source_group(VOSIMLib-units\\include FILES ${VOSIMLIB_UNIT_HDRS})

add_library(VOSIMLib STATIC 
  ${VOSIMLIB_FILES}
  )

add_executable(unit_tests ${VOSIMLIB_DIR}/tests/serialization.cpp)
target_link_libraries(unit_tests VOSIMLib)

cmake_minimum_required(VERSION 3.3)
project(VOSIMLib)

##
# VOSIMLib source files
##
file(GLOB VOSIMLIB_HDRS include/*.h)
file(GLOB VOSIMLIB_SRCS src/*.cpp)
file(GLOB VOSIMLIB_UNIT_HDRS units/include/*.h)
file(GLOB VOSIMLIB_UNIT_SRCS units/src/*.cpp)
file(GLOB VOSIMLIB_PYTHON_SRCS python/*.cpp)
set(VOSIMLIB_FILES ${VOSIMLIB_HDRS};${VOSIMLIB_UNIT_HDRS})
set(VOSIMLIB_FILES ${VOSIMLIB_FILES};${VOSIMLIB_SRCS};${VOSIMLIB_UNIT_SRCS})

source_group(VOSIMLib FILES ${VOSIMLIB_SRCS})
source_group(VOSIMLib/include FILES ${VOSIMLIB_HDRS})
source_group(VOSIMLib-units FILES ${VOSIMLIB_UNIT_SRCS})
source_group(VOSIMLib-units/include FILES ${VOSIMLIB_UNIT_HDRS})

include_directories(${VOSIMLIB_ROOT}/include ${VOSIMLIB_ROOT}/units/include)

add_library(VOSIMLib STATIC ${VOSIMLIB_FILES})

add_subdirectory(tests tests)

pybind11_add_module(pyVOSIMLib ${VOSIMLIB_PYTHON_SRCS})
target_link_libraries(pyVOSIMLib PRIVATE VOSIMLib)

if(WIN32)
  target_compile_definitions(VOSIMLib PRIVATE -DWIN32_LEAN_AND_MEAN)
endif()


cmake_minimum_required(VERSION 3.3)
project(VOSIMLib)

set_option(VOSIMLIB_SHARED FALSE BOOL "Build as a shared library?")

if(BUILD_SHARED_LIBS)
  set(VOSIMLIB_SHARED TRUE)
endif()

if(VOSIMLIB_SHARED)
  list(APPEND VOSIMLIB_DEFS -DVOSIMLIB_SHARED)
endif()

##
# Eigen
##
include_directories(${LIBS_DIR}/eigen)

##
# Generate wavetables
##
set(GENTABLES_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/lut_tables.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/lut_tables.h ${CMAKE_CURRENT_SOURCE_DIR}/src/table_data.cpp)
set(GENTABLES_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/lut_tables.cpp ${CMAKE_CURRENT_SOURCE_DIR}/include/lut_tables.h)
add_custom_command(OUTPUT ${GENTABLES_OUTPUT} COMMAND python ARGS ${CMAKE_CURRENT_SOURCE_DIR}/genTables.py --verbose COMMENT "Generating wavetables ${CMAKE_CURRENT_SOURCE_DIR}" PRE_BUILD VERBATIM WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/units/include)
add_definitions(${VOSIMLIB_DEFS})

##
# Add VOSIMLib target
##
if(VOSIMLIB_SHARED)
  add_library(VOSIMLib SHARED ${VOSIMLIB_FILES})
  set_target_properties(VOSIMLib PROPERTIES DEBUG_POSTFIX -d)
else()
  add_library(VOSIMLib STATIC ${VOSIMLIB_FILES})
  set_target_properties(VOSIMLib PROPERTIES DEBUG_POSTFIX -s-d)
  set_target_properties(VOSIMLib PROPERTIES RELEASE_POSTFIX -s)
  set_target_properties(VOSIMLib PROPERTIES MINSIZEREL_POSTFIX -s)
  set_target_properties(VOSIMLib PROPERTIES RELWITHDEBINFO_POSTFIX -s)
endif()

##
# Add tests target
##
add_subdirectory(tests tests)

##
# Add pybind11 target
##
pybind11_add_module(pyVOSIMLib ${VOSIMLIB_PYTHON_SRCS})
target_link_libraries(pyVOSIMLib PRIVATE VOSIMLib)

if(WIN32)
  target_compile_definitions(VOSIMLib PRIVATE -DWIN32_LEAN_AND_MEAN)
endif()


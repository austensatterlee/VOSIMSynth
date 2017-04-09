cmake_minimum_required(VERSION 3.3)
project(VOSIMLib)

option(VOSIMLIB_SHARED "Build as a shared library?" FALSE)

if(VOSIMLIB_SHARED)
  list(APPEND VOSIMLIB_DEFS -DVOSIMLIB_SHARED)
endif()

set(ADDITIONAL_LIBRARIES "")

##
# Eigen
##
include_directories(${LIBS_DIR}/eigen)

##
# Boost
##
set(Boost_NO_BOOST_CMAKE True)
set(Boost_DETAILED_FAILURE_MSG True)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS thread log REQUIRED)
get_property( BLUB TARGET Boost::thread PROPERTY IMPORTED_LOCATION_RELEASE)
set_property( TARGET Boost::thread PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${BLUB} )
include_directories(${Boost_INCLUDE_DIRS})

##
# MKL
##
list(APPEND ADDITIONAL_LIBRARIES ${MKL_LIBRARIES})
include_directories(${MKL_INCLUDE_DIR})

##
# Generate wavetables
##
set(GENTABLES_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/table_data.cpp)
set(GENTABLES_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/genTables.py)
add_custom_command(OUTPUT ${GENTABLES_OUTPUT} COMMAND python ARGS ${CMAKE_CURRENT_SOURCE_DIR}/genTables.py --verbose DEPENDS ${GENTABLES_DEPENDS} COMMENT "Generating wavetables..." PRE_BUILD VERBATIM WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

##
# VOSIMLib source files
##
file(GLOB VOSIMLIB_HDRS include/*.h)
file(GLOB VOSIMLIB_SRCS src/*.cpp src/table_data.cpp)
file(GLOB VOSIMLIB_UNIT_HDRS include/units/*.h)
file(GLOB VOSIMLIB_UNIT_SRCS src/units/*.cpp)
file(GLOB VOSIMLIB_PYTHON_SRCS python/*.cpp)
set(VOSIMLIB_FILES ${VOSIMLIB_HDRS};${VOSIMLIB_UNIT_HDRS})
set(VOSIMLIB_FILES ${VOSIMLIB_FILES};${VOSIMLIB_SRCS};${VOSIMLIB_UNIT_SRCS})

source_group(VOSIMLib FILES ${VOSIMLIB_SRCS})
source_group(VOSIMLib/include FILES ${VOSIMLIB_HDRS})
source_group(VOSIMLib-units FILES ${VOSIMLIB_UNIT_SRCS})
source_group(VOSIMLib-units/include FILES ${VOSIMLIB_UNIT_HDRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
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
target_link_libraries(VOSIMLib ${Boost_LIBRARIES} ${ADDITIONAL_LIBRARIES})

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


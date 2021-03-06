CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMSynth)

option(VOSIMSYNTH_BUILD_STANDALONE "Build standalone executable?" FALSE)

if(MSVC)
  add_compile_options("$<$<CONFIG:Debug>:/bigobj>")
endif()

list(APPEND VOSIMSYNTH_DEFS -DVST_API -DDLL_BUILD -DVST_FORCE_DEPRECATED -DVOSIMSYNTH_SHARED)

##
# Boost
##
set(Boost_DETAILED_FAILURE_MSG True)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

##
# Eigen
##
include_directories(${LIBS_DIR}/eigen)

##
# VOSIMLib
##
include_directories(${VOSIMLIB_DIR}/include)

##
# Run simple cmake converter to put font files into the data segment
##

# Glob up resource files
file(GLOB resources "${CMAKE_CURRENT_SOURCE_DIR}/resources/*.ttf")

# Concatenate resource files into a comma separated string
string (REGEX REPLACE "([^\\]|^);" "\\1," resources_string "${resources}")
string (REGEX REPLACE "[\\](.)" "\\1" resources_string "${resources_string}")

# Create command line for running bin2c cmake script
set(bin2c_cmdline
  -DOUTPUT_C=vosimsynth_resources.cpp
  -DOUTPUT_H=vosimsynth_resources.h
  "-DINPUT_FILES=${resources_string}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/resources/bin2c.cmake")

# Run bin2c on resource files
add_custom_command(
  OUTPUT vosimsynth_resources.cpp vosimsynth_resources.h
  COMMAND ${CMAKE_COMMAND} ARGS ${bin2c_cmdline}
  DEPENDS ${resources}
  COMMENT "Running bin2c"
  PRE_BUILD VERBATIM)

# Needed to include generated files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

##
# Collect VOSIMSynth sources
##
file(GLOB PROJECT_SRCS vosimsynth_resources.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB PROJECT_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/include/vosimsynth/*.h)
file(GLOB PROJECT_WIDGET_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/include/vosimsynth/widgets/*.h)
set(VOSIMSYNTH_PROJECT_FILES ${PROJECT_SRCS};${PROJECT_HDRS};${PROJECT_WIDGET_HDRS})
SET(ALL_VOSIMSYNTH_FILES ${VOSIMSYNTH_PROJECT_FILES} ${RESOURCE_FILES} ${WDL_FILES} ${IPLUG_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# add source groups ("filters" for MSVC)
source_group(include/vosimsynth/widgets FILES ${PROJECT_WIDGET_HDRS})
source_group(include/vosimsynth FILES ${PROJECT_HDRS})
source_group(src FILES ${PROJECT_SRCS})
source_group(Resources FILES ${RESOURCE_FILES})
source_group(WDL FILES ${WDL_FILES})
source_group(IPlug FILES ${IPLUG_FILES})

# Add standalone project
if(VOSIMSYNTH_BUILD_STANDALONE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/standalone standalone)
endif()

add_definitions(${VOSIMSYNTH_DEFS})

add_library(VOSIMSynth SHARED ${ALL_VOSIMSYNTH_FILES})
set_target_properties(VOSIMSynth PROPERTIES DEBUG_POSTFIX -d)
set_target_properties(VOSIMSynth PROPERTIES RELWITHDEBINFO_POSTFIX -rdb)

target_link_libraries(VOSIMSynth
  "$<$<CONFIG:Release>:${REL_LIBS}>"
  "$<$<NOT:$<CONFIG:Release>>:${DBG_LIBS}>"
  VOSIMLib
  ${MKL_LIBRARIES}
  )

if (WIN32)
  set_target_properties(VOSIMSynth PROPERTIES prefix "")
  target_compile_definitions(VOSIMSynth PRIVATE -DWIN32_LEAN_AND_MEAN)
endif ()

if(VOSIMPROJECT_BUILD_TESTS)
    add_subdirectory(tests)
endif()

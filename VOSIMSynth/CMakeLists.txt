CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMSynth)

option(VOSIMSYNTH_BUILD_VST "Build as a VST?" TRUE)

if(VOSIMSYNTH_BUILD_VST)
  list(APPEND VOSIMSYNTH_DEFS -DVST_API -DDLL_BUILD -DVST_FORCE_DEPRECATED -DVOSIMSYNTH_SHARED)
endif()

set(ADDITIONAL_LIBRARIES "")

set(VOSIMSYNTH_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(VOSIMSYNTH_STANDALONE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/standalone)


##
# Boost
##
set(Boost_NO_BOOST_CMAKE True)
set(Boost_DETAILED_FAILURE_MSG True)
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

##
# nanogui
##
list(APPEND ADDITIONAL_LIBRARIES nanogui ${NANOGUI_EXTRA_LIBS})
add_definitions(${NANOGUI_EXTRA_DEFS})
# # Remove eigen from nanogui's includes, since we have our own
# string(REGEX REPLACE "(;?)[^;]*eigen[^;]*(;?)" "\\1\\2" NANOGUI_EXTRA_INCS "${NANOGUI_EXTRA_INCS}")
include_directories(${NANOGUI_EXTRA_INCS})
include_directories(${LIBS_DIR}/nanogui/include)


##
# MKL
##
list(APPEND ADDITIONAL_LIBRARIES ${MKL_LIBRARIES})
include_directories(${MKL_INCLUDE_DIR})

##
# VOSIMLib
##
include_directories(${VOSIMLIB_DIR}/include)

##
# Compile resources into C code to be compiled into the exe
##
add_executable(vosimsynth_bin2c ${VOSIMSYNTH_RESOURCES}/bin2c.c)
set(RES_BIN vosimsynth_resources.cpp vosimsynth_resources.h)
set(bin2c_cmdline ${RES_BIN})
file(GLOB_RECURSE resources ${VOSIMSYNTH_RESOURCES} *.ttf *.png)

foreach(file ${resources})
  list(APPEND bin2c_cmdline ${file})
endforeach()

##
# Run bin2c on resource files
##
add_custom_command(OUTPUT ${RES_BIN} COMMAND vosimsynth_bin2c ARGS ${bin2c_cmdline} DEPENDS vosimsynth_bin2c ${resources} COMMENT "Running vosimsynth_bin2c" PRE_BUILD VERBATIM)
list(APPEND RESOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/vosimsynth_resources.cpp ${CMAKE_CURRENT_BINARY_DIR}/vosimsynth_resources.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

##
# Collect VOSIMSynth sources
##
file(GLOB PROJECT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB PROJECT_WIDGET_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.cpp)
set(PROJECT_SRCS ${PROJECT_SRCS};${CMAKE_CURRENT_SOURCE_DIR}/vosimsynth.cpp)
file(GLOB PROJECT_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h ${CMAKE_CURRENT_SOURCE_DIR}/include/vosimsynth/*.h)
file(GLOB PROJECT_WIDGET_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/include/widgets/*.h)
set(VOSIMSYNTH_PROJECT_FILES ${PROJECT_SRCS};${PROJECT_HDRS};${PROJECT_WIDGET_SRCS};${PROJECT_WIDGET_HDRS})

list(APPEND RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/vosimsynth.rc ${CMAKE_CURRENT_SOURCE_DIR}/include/resource.h)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/vosimsynth.rc LANGUAGE RC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/widgets)

# Add standalone project
if(NOT ${VOSIMSYNTH_BUILD_VST})
    add_subdirectory(${VOSIMSYNTH_STANDALONE_DIR} standalone)
endif()

add_definitions()
add_definitions(${VOSIMSYNTH_DEFS})

# add source groups ("filters" for MSVC)
source_group(VOSIMSynth/include/widgets FILES ${PROJECT_WIDGET_HDRS}/)
source_group(VOSIMSynth/include/vosimsynth FILES ${CMAKE_CURRENT_SOURCE_DIR})
source_group(VOSIMSynth/include FILES ${PROJECT_HDRS})
source_group(VOSIMSynth/src FILES ${PROJECT_SRCS})
source_group(VOSIMSynth/src/widgets FILES ${PROJECT_WIDGET_SRCS})
source_group(Resources FILES ${RESOURCE_FILES})
source_group(WDL FILES ${WDL_FILES})
source_group(IPlug FILES ${IPLUG_FILES})

SET(ALL_VOSIMSYNTH_FILES ${VOSIMSYNTH_PROJECT_FILES} ${RESOURCE_FILES} ${WDL_FILES} ${IPLUG_FILES})

if(VOSIMSYNTH_BUILD_VST)
  add_library(${PROJECT_NAME} SHARED ${ALL_VOSIMSYNTH_FILES})
  set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -d)
else()
  add_library(${PROJECT_NAME} STATIC ${ALL_VOSIMSYNTH_FILES})
  set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -s-d)
  set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX -s)
  set_target_properties(${PROJECT_NAME} PROPERTIES MINSIZEREL_POSTFIX -s)
  set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX -s)
endif()

list(REMOVE_DUPLICATES ADDITIONAL_LIBRARIES)
target_link_libraries(${PROJECT_NAME}
  "$<$<CONFIG:Release>:${REL_LIBS}>"
  "$<$<NOT:$<CONFIG:Release>>:${DBG_LIBS}>"
  VOSIMLib
 ${ADDITIONAL_LIBRARIES}
  )

if (WIN32)
  target_link_libraries(${PROJECT_NAME} comctl32 wininet)
  set_target_properties(${PROJECT_NAME} PROPERTIES prefix "")
endif ()

add_subdirectory(tests)

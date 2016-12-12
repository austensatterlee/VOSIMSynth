CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMSynth)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

set_option(VOSIMSYNTH_SHARED FALSE BOOL "Build as a shared library?")
set_option(VOSIMSYNTH_BUILD_STANDALONE FALSE BOOL "Build standalone application?")

if(BUILD_SHARED_LIBS)
  set(VOSIMSYNTH_SHARED TRUE)
endif()

if(VOSIMSYNTH_SHARED)
  list(APPEND VOSIMSYNTH_DEFS -DVOSIMSYNTH_SHARED)
endif()

if(NOT DEFINED ADDITIONAL_LIBRARIES)
  set(ADDITIONAL_LIBRARIES "")
endif()

set(NANOVG_ROOT ${LIBS_DIR}/nanovg CACHE PATH "")
set(VST2_SDK ${LIBS_DIR}/VST_SDK CACHE PATH "")
set(GLEW_ROOT "${LIBS_DIR_SYS}/glew" CACHE PATH "")
set(SFML_ROOT "${LIBS_DIR_SYS}/SFML" CACHE PATH "")
set(TGUI_ROOT ${LIBS_DIR_SYS}/tgui CACHE PATH "")
set(SFTOOLS_ROOT ${LIBS_DIR}/sftools CACHE PATH "")
set(VOSIMSYNTH_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources CACHE PATH "")
set(VOSIMSYNTH_STANDALONE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/standalone CACHE PATH "")

## sftools
file(GLOB SFTOOLS_FILES ${SFTOOLS_ROOT}/include/sftools/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Animation/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/ResourceManager/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Common/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Singleton/*.hpp)
include_directories(${SFTOOLS_ROOT}/include)

##
# SFML
set(SFML_STATIC_LIBRARIES true)
set(SFML_STATIC true)
add_definitions(-DSFML_STATIC)
add_definitions(-DSFTOOLS_NO_AUDIO)
set(ENV{SFML_ROOT} ${SFML_ROOT})
find_package(SFML 2 REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})
list(APPEND DBG_LIBS ${SFML_SYSTEM_LIBRARY_DEBUG} ${SFML_WINDOW_LIBRARY_DEBUG} ${SFML_GRAPHICS_LIBRARY_DEBUG})
list(APPEND REL_LIBS ${SFML_SYSTEM_LIBRARY_RELEASE} ${SFML_WINDOW_LIBRARY_RELEASE} ${SFML_GRAPHICS_LIBRARY_RELEASE})
list(APPEND ADDITIONAL_LIBRARIES ${SFML_DEPENDENCIES})
message(STATUS "SFML include: ${SFML_INCLUDE_DIR}")

##
# TGUI
set(TGUI_SHARED_LIBS false)
set(ENV{TGUI_ROOT} ${TGUI_ROOT})
find_package(TGUI REQUIRED)
include_directories(${TGUI_INCLUDE_DIR})
list(APPEND DBG_LIBS ${TGUI_LIBRARY_DEBUG})
list(APPEND REL_LIBS ${TGUI_LIBRARY_RELEASE})

##
# GLEW
# message(STATUS "GLEW_ROOT: ${GLEW_ROOT}")
# set(CMAKE_PREFIX_PATH ${GLEW_ROOT})
# find_package(GLEW REQUIRED)
# add_definitions(-DGLEW_MX -DGLEW_STATIC)
# include_directories(${GLEW_INCLUDE_DIR})
# message(STATUS "GLEW libs: ${GLEW_LIBRARY}" )
# message(STATUS "GLEW includes: ${GLEW_INCLUDE_DIRS}" )
# string(REGEX REPLACE "(.*)\\.(.*)" "\\1$<$<CONFIG:Debug>:d>.\\2" GLEW_LIBRARY "${GLEW_LIBRARY}")
# list(APPEND ADDITIONAL_LIBRARIES ${GLEW_LIBRARY})

##
# OpenGL
# find_package(OpenGL REQUIRED)
# include_directories(${OPENGL_INCLUDE_DIR})
# list(APPEND ADDITIONAL_LIBRARIES ${OPENGL_LIBRARIES})

##
# NanoVG
# add_definitions(-DNANOVG_GLEW -DNANOVG_GL3_IMPLEMENTATION -DSTB_IMAGE_STATIC)
# file(GLOB NANOVG_FILES ${NANOVG_ROOT}/src/*.c ${NANOVG_ROOT}/src/*.h)
# include_directories(${NANOVG_ROOT}/src)

##############################################################
# Compile resources into C code to be compiled into the exe  #
##############################################################
add_executable(bin2c ${VOSIMSYNTH_RESOURCES}/bin2c.c)
set(RES_BIN vosimsynth_resources.cpp vosimsynth_resources.h)
set(bin2c_cmdline ${RES_BIN})
file(GLOB_RECURSE resources ${VOSIMSYNTH_RESOURCES} *.ttf *.png)

foreach(file ${resources})
  list(APPEND bin2c_cmdline ${file})
endforeach()

# Run bin2c on resource files
add_custom_command(OUTPUT ${RES_BIN} COMMAND bin2c ARGS ${bin2c_cmdline} DEPENDS bin2c ${resources} COMMENT "Running bin2c" PRE_BUILD VERBATIM)
list(APPEND RESOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/vosimsynth_resources.cpp ${CMAKE_CURRENT_BINARY_DIR}/vosimsynth_resources.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

##############################
# Collect VOSIMSynth sources #
##############################

file(GLOB PROJECT_SRCS ${VOSIMSYNTH_ROOT}/src/*.cpp)
set(PROJECT_SRCS ${PROJECT_SRCS};${VOSIMSYNTH_ROOT}/vosimsynth.cpp)
file(GLOB PROJECT_HDRS ${VOSIMSYNTH_ROOT}/include/*.h)
set(VOSIMSYNTH_PROJECT_FILES ${PROJECT_SRCS};${PROJECT_HDRS})

list(APPEND RESOURCE_FILES ${VOSIMSYNTH_ROOT}/vosimsynth.rc ${VOSIMSYNTH_ROOT}/include/resource.h)
set_source_files_properties(${VOSIMSYNTH_ROOT}/vosimsynth.rc LANGUAGE RC)

include_directories(${VOSIMSYNTH_ROOT}/include)

# Add standalone project
if(${VOSIMSYNTH_BUILD_STANDALONE})
    add_subdirectory(${VOSIMSYNTH_STANDALONE_ROOT} standalone)
endif()

add_definitions(-DVST_API -DDLL_BUILD -DVST_FORCE_DEPRECATED)
add_definitions(${VOSIMSYNTH_DEFS})

# add source groups ("filters" for MSVC)
source_group(VOSIMSynth\\include FILES ${PROJECT_HDRS})
source_group(VOSIMSynth FILES ${PROJECT_SRCS})
source_group(Resources FILES ${RESOURCE_FILES})
source_group(WDL FILES ${WDL_FILES})
source_group(IPlug FILES ${IPLUG_FILES})
source_group(NanoVG FILES ${NANOVG_FILES})
source_group(sftools FILES ${SFTOOLS_FILES})

SET(ALL_VOSIMSYNTH_FILES ${VOSIMSYNTH_PROJECT_FILES} ${RESOURCE_FILES} ${WDL_FILES} ${IPLUG_FILES} ${NANOVG_FILES} ${SFTOOLS_FILES})

if(VOSIMSYNTH_SHARED)
  add_library(${PROJECT_NAME} SHARED ${ALL_VOSIMSYNTH_FILES})
  set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -d)
else()
  add_library(${PROJECT_NAME} STATIC ${ALL_VOSIMSYNTH_FILES})
  set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -s-d)
  set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX -s)
  set_target_properties(${PROJECT_NAME} PROPERTIES MINSIZEREL_POSTFIX -s)
  set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX -s)
endif()

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${VOSIMSYNTH_RESOURCES}/widgets $<TARGET_FILE_DIR:${PROJECT_NAME}>/widgets)

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

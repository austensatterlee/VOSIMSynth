CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMSynth)

set(PLUGIN_NAME VOSIMSynth)

set(NANOVG_ROOT ${LIBS_DIR}/nanovg CACHE PATH "")
set(VST2_SDK ${LIBS_DIR}/VST_SDK CACHE PATH "")
set(GLEW_ROOT_64 "${LIBS_DIR_SYS}/glew" CACHE PATH "")
set(GLEW_ROOT_32 "${LIBS_DIR_SYS}/glew" CACHE PATH "")
set(SFML_ROOT_64 "${LIBS_DIR_SYS}/SFML" CACHE PATH "")
set(SFML_ROOT_32 "${LIBS_DIR_SYS}/SFML" CACHE PATH "")
set(SFTOOLS_ROOT ${LIBS_DIR}/sftools CACHE PATH "")
set(RESOURCES_DIR ${CMAKE_SOURCE_DIR}/vosimsynth/resources CACHE PATH "")
set(STANDALONE_ROOT ${CMAKE_SOURCE_DIR}/vosimsynth/standalone CACHE PATH "")

##
# sftools
##
file(GLOB SFTOOLS_FILES ${SFTOOLS_ROOT}/include/sftools/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Animation/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/ResourceManager/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Common/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Singleton/*.hpp)
include_directories(${SFTOOLS_ROOT}/include)

# Ensure cmake searches specified paths

if(ARCH EQUAL 64)
  set(SFML_ROOT ${SFML_ROOT_64})
  set(GLEW_ROOT ${GLEW_ROOT_64})
else()
  set(SFML_ROOT ${SFML_ROOT_32})
  set(GLEW_ROOT ${GLEW_ROOT_32})
endif()

# Add SFML cmake module
set(SFML_STATIC true)
set(SFML_STATIC_LIBRARIES true)
add_definitions(-DSFTOOLS_NO_AUDIO)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

# SFML
find_package(SFML 2 REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})
list(APPEND DBG_LIBS ${SFML_SYSTEM_LIBRARY_DEBUG} ${SFML_WINDOW_LIBRARY_DEBUG} ${SFML_GRAPHICS_LIBRARY_DEBUG})
list(APPEND REL_LIBS ${SFML_SYSTEM_LIBRARY_RELEASE} ${SFML_WINDOW_LIBRARY_RELEASE} ${SFML_GRAPHICS_LIBRARY_RELEASE})
list(APPEND ADDITIONAL_LIBRARIES ${SFML_DEPENDENCIES})
message(STATUS "SFML include: ${SFML_INCLUDE_DIR}")

# GLEW
message(STATUS "GLEW_ROOT: ${GLEW_ROOT}")
set(CMAKE_PREFIX_PATH ${GLEW_ROOT})
find_package(GLEW REQUIRED)
add_definitions(-DGLEW_MX -DGLEW_STATIC)
include_directories(${GLEW_INCLUDE_DIR})
message(STATUS "GLEW libs: ${GLEW_LIBRARY}" )
message(STATUS "GLEW includes: ${GLEW_INCLUDE_DIRS}" )
string(REGEX REPLACE "(.*)\\.(.*)" "\\1$<$<CONFIG:Debug>:d>.\\2" GLEW_LIBRARY "${GLEW_LIBRARY}")
list(APPEND ADDITIONAL_LIBRARIES ${GLEW_LIBRARY})

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
list(APPEND ADDITIONAL_LIBRARIES ${OPENGL_LIBRARIES})

##
# NanoVG
##
add_definitions(-DNANOVG_GLEW -DNANOVG_GL3_IMPLEMENTATION -DSTB_IMAGE_STATIC)
file(GLOB NANOVG_FILES ${NANOVG_ROOT}/src/*.c ${NANOVG_ROOT}/src/*.h)
include_directories(${NANOVG_ROOT}/src)

##############################################################
# Compile resources into C code to be compiled into the exe  #
##############################################################
add_executable(bin2c ${RESOURCES_DIR}/bin2c.c)
set(RES_BIN vosimsynth_resources.cpp vosimsynth_resources.h)
set(bin2c_cmdline ${RES_BIN})
file(GLOB_RECURSE resources ${RESOURCES_DIR} *.ttf *.png)

foreach(file ${resources})
  list(APPEND bin2c_cmdline ${file})
endforeach()

# Run bin2c on resource files
add_custom_command(
  OUTPUT ${RES_BIN}
  COMMAND bin2c ARGS ${bin2c_cmdline}
  DEPENDS bin2c ${resources}
  COMMENT "Running bin2c"
  PRE_BUILD VERBATIM)

list(APPEND RESOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/vosimsynth_resources.cpp ${CMAKE_CURRENT_BINARY_DIR}/vosimsynth_resources.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

##############################
# Collect VOSIMSynth sources #
##############################

file(GLOB PROJECT_SRCS ${VOSIMSYNTH_ROOT}/src/*.cpp)
set(PROJECT_SRCS ${PROJECT_SRCS};${VOSIMSYNTH_ROOT}/vosimsynth.cpp)
file(GLOB PROJECT_HDRS ${VOSIMSYNTH_ROOT}/include/*.h)
set(PROJECT_FILES ${PROJECT_SRCS};${PROJECT_HDRS})

list(APPEND RESOURCE_FILES ${VOSIMSYNTH_ROOT}/vosimsynth.rc ${VOSIMSYNTH_ROOT}/include/resource.h)
set_source_files_properties(${VOSIMSYNTH_ROOT}/vosimsynth.rc LANGUAGE RC)

include_directories(${VOSIMSYNTH_ROOT}/include)

# Add standalone project
add_subdirectory(${STANDALONE_ROOT} standalone)

add_definitions(-DVST_API -DDLL_BUILD -DVST_FORCE_DEPRECATED)

# add source groups ("filters" for MSVC)
source_group(VOSIMSynth\\include FILES ${PROJECT_HDRS})
source_group(VOSIMSynth FILES ${PROJECT_SRCS})
source_group(Resources FILES ${RESOURCE_FILES})
source_group(WDL FILES ${WDL_FILES})
source_group(IPlug FILES ${IPLUG_FILES})
source_group(NanoVG FILES ${NANOVG_FILES})
source_group(sftools FILES ${SFTOOLS_FILES})

add_library(${PLUGIN_NAME} SHARED
  ${PROJECT_FILES}
  ${RESOURCE_FILES}
  ${WDL_FILES}
  ${IPLUG_FILES}
  ${NANOVG_FILES}
  ${SFTOOLS_FILES}
  )

list(REMOVE_DUPLICATES ADDITIONAL_LIBRARIES)
target_link_libraries(${PLUGIN_NAME}
  "$<$<CONFIG:Release>:${REL_LIBS}>"
  "$<$<NOT:$<CONFIG:Release>>:${DBG_LIBS}>"
  VOSIMLib
 ${ADDITIONAL_LIBRARIES}
  )

if (WIN32)
  target_link_libraries(${PLUGIN_NAME} comctl32 wininet)
  set_target_properties(${PLUGIN_NAME} PROPERTIES prefix "")
endif ()

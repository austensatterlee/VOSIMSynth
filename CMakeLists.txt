CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMProject)
enable_language(C CXX RC)

set(PLUGIN_NAME ${CMAKE_PROJECT_NAME})

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# define source directories
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs CACHE PATH "")

set(NANOVG_ROOT ${LIBS_DIR}/nanovg CACHE PATH "")
set(WDL_ROOT ${LIBS_DIR}/wdl CACHE PATH "")
set(IPLUG_ROOT ${WDL_ROOT}/iplug CACHE PATH "")
set(VST2_SDK ${LIBS_DIR}/VST_SDK CACHE PATH "")
set(EIGEN_ROOT ${LIBS_DIR}/eigen CACHE PATH "")
set(CEREAL_ROOT ${LIBS_DIR}/cereal CACHE PATH "")
set(GLEW_ROOT "C:/Program Files (x86)/glew" CACHE PATH "")
set(SFML_ROOT "C:/Program Files/sfml" CACHE PATH "")
set(SFTOOLS_ROOT ${LIBS_DIR}/sftools CACHE PATH "")
set(NONIUS_ROOT ${LIBS_DIR}/nonius CACHE PATH "")

set(FFTS_ROOT "C:/Program Files (x86)/ffts" CACHE PATH "")
set(FFTS_INCLUDE_DIR ${FFTS_ROOT}/include CACHE PATH "")

set(VOSIMLIB_ROOT ${CMAKE_SOURCE_DIR}/vosimlib CACHE PATH "")
set(VOSIMSYNTH_ROOT ${CMAKE_SOURCE_DIR}/vosimsynth CACHE PATH "")
set(RESOURCES_DIR ${VOSIMSYNTH_ROOT}/resources CACHE PATH "")
set(STANDALONE_ROOT ${VOSIMSYNTH_ROOT}/standalone CACHE PATH "")

message (STATUS "Build tool is ${CMAKE_BUILD_TOOL}")
message (STATUS "System is ${CMAKE_SYSTEM_NAME}")

##
# Set compiler/linker settings
##

if (MINGW)
  set(CMAKE_CXX_FLAGS "-std=gnu++0x -fpermissive -wno-unknown-pragmas-wno-write-strings" CACHE STRING "")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math  -D__NO_INLINE__" CACHE STRING "")
  # compile the runtime as static (this will make sure that libgcc_s_seh-1.dll et al. won't show up in DEPENDENCY walker)
  set(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")
  set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")

  set(CMAKE_RC_COMPILER_INIT windres CACHE STRING "" FORCE)
  set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>" CACHE STRING "")

  # some flags apparently needed to compile with gcc
  add_definitions(-Dstricmp=strcasecmp -DWIN32 -DNTDDI_VERSION=0x05010000 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0500) 
endif (MINGW)

if(MSVC)
  # Runtime library, exceptions, parallel compilation
  set(MSVC_REL /MT /MP)
  set(MSVC_RDEB /EHsc /MTd /MP)
  set(MSVC_DEB /EHsc /MTd /MP)

  # Disable some warnings
  set(MSVC_DISABLED_WARNINGS /wd4996 /wd4224 /wd4244 /wd4091 /wd4018)
  add_compile_options("${MSVC_DISABLED_WARNINGS}")
  add_definitions(-D_ENABLE_ATOMIC_ALIGNMENT_FIX)
  # /Ox full opt                        /Ob2 any suitable inline function expansion
  # /Oi enable intrinsics               /Ot favor fast code
  # /GL whole program optimization      /arch:SSE2 simd extensions
  # /fp:fast floating point model       /Qpar enable parallel code gen
  # /Oy omit frame pointers             /GT fiber-safe optimizations
  list(APPEND MSVC_REL /Ox /GL /Ob2 /Oi /Ot /Oy /arch:SSE2 /fp:fast /fp:except- /Qpar /GT)
  list(APPEND MSVC_RDEB /Ox /Ob2 /Oi /Ot /Oy /arch:SSE2 /fp:fast /Qpar /GT)
  add_compile_options("$<$<CONFIG:Release>:${MSVC_REL}>")
  add_compile_options("$<$<CONFIG:RelWithDebInfo>:${MSVC_RDEB}>")
  add_compile_options("$<$<CONFIG:Debug>:${MSVC_DEB}>")

  # Linker optimizations
  # /LTCG link time code generation
  # /OPT:REF remove code that is never referenced
  # /OPT:ICF perform identical COMDAT folding
  set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
endif()

# configurations for all compilers
add_compile_options("$<$<NOT:$<CONFIG:Release>>:-D_DEBUG>")
# add_compile_options("$<$<CONFIG:Release>:-DNDEBUGLIBPNG>")
# add_compile_options(-DPNG_NO_ASSEMBLER_CODE -DPNG_LIBPNG_SPECIALBUILD) 

################################
# Collect VOSIMSynth sources
################################

##
# VOSIMSynth 
##
file(GLOB PROJECT_SRCS ${VOSIMSYNTH_ROOT}/src/*.cpp)
set(PROJECT_SRCS ${PROJECT_SRCS};${VOSIMSYNTH_ROOT}/vosimsynth.cpp)
file(GLOB PROJECT_HDRS ${VOSIMSYNTH_ROOT}/include/*.h)
set(PROJECT_FILES ${PROJECT_SRCS};${PROJECT_HDRS})

set(RESOURCE_FILES ${VOSIMSYNTH_ROOT}/vosimsynth.rc ${VOSIMSYNTH_ROOT}/include/resource.h)
set_source_files_properties(${VOSIMSYNTH_ROOT}/vosimsynth.rc LANGUAGE RC)

include_directories(${VOSIMSYNTH_ROOT}/include)

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

#############################################
# Configure project-integrated dependencies #
#############################################

##
# WDL
##
include_directories(${WDL_ROOT})

##
# IPlug
##
set(IPLUG_FILES
  ${IPLUG_ROOT}/Containers.h ${IPLUG_ROOT}/Hosts.cpp ${IPLUG_ROOT}/Hosts.h
  ${IPLUG_ROOT}/IPlugBase.cpp ${IPLUG_ROOT}/IPlugBase.h
  ${IPLUG_ROOT}/IPlugOSDetect.h ${IPLUG_ROOT}/IPlugStructs.cpp
  ${IPLUG_ROOT}/IPlugStructs.h ${IPLUG_ROOT}/IPlugVST.cpp
  ${IPLUG_ROOT}/IPlugVST.h ${IPLUG_ROOT}/IPlug_Prefix.pch
  ${IPLUG_ROOT}/IPlug_include_in_plug_hdr.h
  ${IPLUG_ROOT}/IPlug_include_in_plug_src.h ${IPLUG_ROOT}/Log.cpp
  ${IPLUG_ROOT}/Log.h
)
include_directories(${IPLUG_ROOT})

##
# NanoVG
##
add_definitions(-DNANOVG_GL3_IMPLEMENTATION -DNANOVG_GLEW)
file(GLOB NANOVG_FILES ${NANOVG_ROOT}/src/*.c ${NANOVG_ROOT}/src/*.h)
include_directories(${NANOVG_ROOT}/src)

##
# sftools
##
file(GLOB SFTOOLS_FILES ${SFTOOLS_ROOT}/include/sftools/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Animation/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/ResourceManager/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Common/*.hpp
  ${SFTOOLS_ROOT}/include/sftools/Singleton/*.hpp)
include_directories(${SFTOOLS_ROOT}/include)

##
# Cereal
##
include_directories(${CEREAL_ROOT}/include)

##
# Eigen
##
include_directories(${EIGEN_ROOT})

######################################
# Configure precompiled dependencies #
######################################

list(APPEND CMAKE_PREFIX_PATH ${GLEW_ROOT})
list(APPEND CMAKE_PREFIX_PATH ${SFML_ROOT})
list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)

set(BUILD_SHARED_LIBS false)

# Add boost
set(Boost_NO_BOOST_CMAKE True)
set(Boost_DETAILED_FAILURE_MSG True)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# Add VOSIMLib
add_subdirectory(${VOSIMLIB_ROOT} VOSIMLib)
include_directories(${VOSIMLIB_ROOT}/include)
include_directories(${VOSIMLIB_ROOT}/units/include)

# Add SFML
set(SFML_STATIC true)
set(SFML_STATIC_LIBRARIES true)
add_definitions(-DSFTOOLS_NO_AUDIO)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

# Find SFML
find_package(SFML 2 REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})
list(APPEND DBG_LIBS ${SFML_SYSTEM_LIBRARY_DEBUG} ${SFML_WINDOW_LIBRARY_DEBUG} ${SFML_GRAPHICS_LIBRARY_DEBUG})
list(APPEND REL_LIBS ${SFML_SYSTEM_LIBRARY_RELEASE} ${SFML_WINDOW_LIBRARY_RELEASE} ${SFML_GRAPHICS_LIBRARY_RELEASE})
list(APPEND ADDITIONAL_LIBRARIES ${SFML_DEPENDENCIES})
# message(STATUS "SFML debug libs: ${SFML_DBG_LIBS}" )
# message(STATUS "SFML release libs: ${SFML_REL_LIBS}" )
# message(STATUS "SFML deps: ${SFML_DEPENDENCIES}" )

# Find glew
find_package(glew REQUIRED)
add_definitions(-DGLEW_MX -DGLEW_STATIC)
include_directories(${GLEW_INCLUDE_DIR})
#message(STATUS "GLEW libs: ${GLEW_LIBRARY}" )
string(REGEX REPLACE "(.*)\\.(.*)" "\\1$<$<CONFIG:Debug>:d>.\\2" GLEW_LIBRARY "${GLEW_LIBRARY}")
list(APPEND ADDITIONAL_LIBRARIES ${GLEW_LIBRARY})

# Find OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
list(APPEND ADDITIONAL_LIBRARIES ${OPENGL_LIBRARIES})
#message(STATUS "OPENGL libs: ${OPENGL_LIBRARIES}" )

# Link FFTS
find_library(FFTS_RELEASE_LIBRARY ffts ffts_static PATHS ${FFTS_ROOT} PATH_SUFFIXES lib)
find_library(FFTS_DEBUG_LIBRARY fftsd ffts_staticd PATHS ${FFTS_ROOT} PATH_SUFFIXES lib)
include_directories(${FFTS_INCLUDE_DIR})
list(APPEND DBG_LIBS ${FFTS_DEBUG_LIBRARY})
list(APPEND REL_LIBS ${FFTS_RELEASE_LIBRARY})

list(REMOVE_DUPLICATES ADDITIONAL_LIBRARIES)


################################
# Specify targets
################################

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

message(STATUS "REL LIBS: ${REL_LIBS}>")
message(STATUS "DBG LIBS: ${DBG_LIBS}>")

target_link_libraries(${PLUGIN_NAME} ${ADDITIONAL_LIBRARIES}
  "$<$<CONFIG:Release>:${REL_LIBS}>"
  "$<$<NOT:$<CONFIG:Release>>:${DBG_LIBS}>"
  VOSIMLib
  )

if (WIN32)
  target_link_libraries(${PLUGIN_NAME} comctl32 wininet)
  set_target_properties(${PLUGIN_NAME} PROPERTIES prefix "")
endif ()

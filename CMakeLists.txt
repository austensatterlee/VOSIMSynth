CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMProject)
enable_language(C CXX RC)

set(PLUGIN_NAME ${CMAKE_PROJECT_NAME})

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# define source directories
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs CACHE PATH "")
set(ALLOY_DIR ${LIBS_DIR}/alloy CACHE PATH "")
set(CEREAL_DIR ${LIBS_DIR}/cereal CACHE PATH "")
set(WDL_DIR ${LIBS_DIR}/wdl CACHE PATH "")
set(IPLUG_DIR ${WDL_DIR}/iplug CACHE PATH "")
set(VOSIMLIB_DIR ${CMAKE_SOURCE_DIR}/vosimlib CACHE PATH "")
set(VOSIMSYNTH_DIR ${CMAKE_SOURCE_DIR}/vosimsynth CACHE PATH "")
set(STANDALONE_DIR ${VOSIMSYNTH_DIR}/standalone CACHE PATH "")

message (STATUS "Path to alloy is ${ALLOY_DIR}")
message (STATUS "Path to cereal is ${CEREAL_DIR}")
message (STATUS "Path to wdl is ${WDL_DIR}")
message (STATUS "Path to iplug is ${IPLUG_DIR}")
message (STATUS "Path to vosimlib is ${VOSIMLIB_DIR}")
message (STATUS "Path to vosimsynth is ${VOSIMSYNTH_DIR}")
message (STATUS "Path to vosimsynth standalone is ${STANDALONE_DIR}")

message (STATUS "Build tool is ${CMAKE_BUILD_TOOL}")
message (STATUS "System is ${CMAKE_SYSTEM_NAME}")

# Build alloy
set(ALLOY_BUILD_EXAMPLE OFF CACHE BOOL "")
add_subdirectory(${ALLOY_DIR} "libs/alloy")

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
  # Runtime library
  set(MSVC_REL /EHsc /MT)
  set(MSVC_DEB /EHsc /MT)

  # Disable some warnings
  set(MSVC_DISABLED_WARNINGS /wd4996 /wd4224 /wd4244 /wd4091 /wd4018)
  add_compile_options("$<$<CONFIG:Debug>:${MSVC_DISABLED_WARNINGS}>")
  # /Ox full opt                        /Ob2 any suitable inline function expansion
  # /Oi enable intrinsics               /Ot favor fast code
  # /GL whole program optimization      /arch:SSE2 simd extensions
  # /fp:fast fast floating point model  /Qpar enable parallel code gen
  # /Oy omit frame pointers             /GT fiber-safe optimizations
  list(APPEND MSVC_REL /Ox /Ob2 /Oi /Ot /GL /arch:SSE2 /fp:fast /Qpar /Oy /GT)
  set(MSVC_RDEB /Ob2 /Oi /arch:SSE2 /fp:fast /GT)
  add_compile_options("$<$<CONFIG:Release>:${MSVC_REL}>")
  add_compile_options("$<$<CONFIG:RelWithDebInfo>:${MSVC_RDEB}>")

  # Linker optimizations
  # /LTCG link time code generation
  # /OPT:REF remove code that is never referenced
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/LTCG /INCREMENTAL:NO /OPT:REF")
  set(CMAKE_SHARED_EXE_LINKER_FLAGS_RELEASE "/LTCG /INCREMENTAL:NO /OPT:REF")
endif()

# configurations for all compilers
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUGLIBPNG" CACHE STRING "")
add_definitions(-DWDL_FFT_REALSIZE=8)
add_definitions(-DPNG_NO_ASSEMBLER_CODE -DPNG_LIBPNG_SPECIALBUILD) 

################################
# Collect sources
################################

# WDL source files
set(WDL_FILES ${WDL_DIR}/fft.c ${WDL_DIR}/fft.h)

# VOSIMSynth source files
file(GLOB PROJECT_SRCS ${VOSIMSYNTH_DIR}/src/*.cpp)
set(PROJECT_SRCS ${PROJECT_SRCS};${VOSIMSYNTH_DIR}/vosimsynth.cpp)
file(GLOB PROJECT_HDRS ${VOSIMSYNTH_DIR}/include/*.h)
set(PROJECT_FILES ${PROJECT_SRCS};${PROJECT_HDRS})

set(RESOURCE_FILES ${VOSIMSYNTH_DIR}/vosimsynth.rc ${VOSIMSYNTH_DIR}/include/resource.h)
set_source_files_properties(${VOSIMSYNTH_DIR}/vosimsynth.rc LANGUAGE RC)

################################
# Compile dependencies
################################

# Add boost
set(Boost_NO_BOOST_CMAKE True)
set(Boost_DETAILED_FAILURE_MSG True)
find_package(Boost)
message(STATUS "Boost include dirs (post-search): ${Boost_INCLUDE_DIRS}")
include_directories(${Boost_INCLUDE_DIRS})

# VOSIMLib
add_subdirectory(${VOSIMLIB_DIR} VOSIMLib)

################################
# Include header directories
################################
include_directories(${VOSIMSYNTH_DIR}/include)

include_directories(${VOSIMLIB_DIR}/include)
include_directories(${VOSIMLIB_DIR}/units/include)

include_directories(${CEREAL_DIR}/include)
include_directories(${ALLOY_DIR}/include/core)
if(WIN32)
  include_directories(${ALLOY_DIR}/ext/glew/include ${ALLOY_DIR}/ext/glfw/include)
endif()
include_directories(${WDL_DIR})
include_directories(${IPLUG_DIR})

# IPlug
add_subdirectory(${IPLUG_DIR} "libs/IPlugLib")

# Add standalone project
add_subdirectory(${STANDALONE_DIR} standalone)

add_definitions(-DVST_API -DDLL_BUILD -DVST_FORCE_DEPRECATED)

# add source groups ("filters" for MSVC)
source_group(VOSIMSynth\\include FILES ${PROJECT_HDRS})
source_group(VOSIMSynth FILES ${PROJECT_SRCS})
source_group(Resources FILES ${RESOURCE_FILES})
source_group(WDL FILES ${WDL_FILES})


add_library(${PLUGIN_NAME} SHARED
  ${PROJECT_FILES}
  ${RESOURCE_FILES}
  ${WDL_FILES}
  )

target_link_libraries(${PLUGIN_NAME} VOSIMLib IPlugLib alloy)

if (WIN32)
  target_link_libraries(${PLUGIN_NAME} comctl32 wininet)
  set_target_properties(${PLUGIN_NAME} PROPERTIES prefix "")
endif ()

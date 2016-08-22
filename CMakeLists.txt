CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMProject)
enable_language(C CXX RC)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# define source directories
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs CACHE PATH "")

set(WDL_ROOT ${LIBS_DIR}/wdl CACHE PATH "")
set(IPLUG_ROOT ${WDL_ROOT}/iplug CACHE PATH "")
set(EIGEN_ROOT ${LIBS_DIR}/eigen CACHE PATH "")
set(CEREAL_ROOT ${LIBS_DIR}/cereal CACHE PATH "")

set(FFTS_ROOT "C:/Program Files (x86)/ffts" CACHE PATH "")
set(FFTS_INCLUDE_DIR ${FFTS_ROOT}/include CACHE PATH "")

set(VOSIMLIB_ROOT ${CMAKE_SOURCE_DIR}/vosimlib CACHE PATH "")
set(VOSIMSYNTH_ROOT ${CMAKE_SOURCE_DIR}/vosimsynth CACHE PATH "")

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
  set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
endif()

# configurations for all compilers
add_compile_options("$<$<NOT:$<CONFIG:Release>>:-D_DEBUG>")

#################################
# Configure include directories #
#################################

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

set(BUILD_SHARED_LIBS False)

# Add boost
set(Boost_NO_BOOST_CMAKE True)
set(Boost_DETAILED_FAILURE_MSG True)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# FFTS
find_library(FFTS_RELEASE_LIBRARY ffts ffts_static PATHS ${FFTS_ROOT} PATH_SUFFIXES lib)
find_library(FFTS_DEBUG_LIBRARY fftsd ffts_staticd PATHS ${FFTS_ROOT} PATH_SUFFIXES lib)
include_directories(${FFTS_INCLUDE_DIR})
list(APPEND DBG_LIBS ${FFTS_DEBUG_LIBRARY})
list(APPEND REL_LIBS ${FFTS_RELEASE_LIBRARY})

# Add VOSIMLib
add_subdirectory(${VOSIMLIB_ROOT} VOSIMLib)
include_directories(${VOSIMLIB_ROOT}/include)
include_directories(${VOSIMLIB_ROOT}/units/include)

add_subdirectory(${VOSIMSYNTH_ROOT} VOSIMSynth)

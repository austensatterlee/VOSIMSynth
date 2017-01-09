CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMProject)
enable_language(C CXX RC)

# Define a macro that helps defining an option
macro(set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(ARCH 64)
  set(LIBS_DIR_SYS "C:/Program Files")
  set(PYTHON_EXECUTABLE "C:/Python27/python.exe" CACHE PATH "")
else()
  set(ARCH 32)
  set(LIBS_DIR_SYS "C:/Program Files (x86)")
  set(PYTHON_EXECUTABLE "C:/Python27_32bit/python.exe" CACHE PATH "")
endif()

# define source directories
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs CACHE PATH "")

set(FFTS_ROOT "${LIBS_DIR_SYS}/ffts" CACHE PATH "")
set(FFTS_INCLUDE_DIR ${FFTS_ROOT}/include CACHE PATH "")

set(VOSIMLIB_DIR ${CMAKE_SOURCE_DIR}/vosimlib)
set(VOSIMSYNTH_DIR ${CMAKE_SOURCE_DIR}/vosimsynth)

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
  set(MSVC_REL /EHsc /MT /MP)
  set(MSVC_RDEB /EHsc /MTd /Gm)
  set(MSVC_DEB /EHsc /MTd /Gm)

  # Disable some warnings
  set(MSVC_DISABLED_WARNINGS /wd4996 /wd4224 /wd4244 /wd4091 /wd4018 /wd4503 /wd4390)
  add_compile_options("${MSVC_DISABLED_WARNINGS}")
  add_definitions(-D_ENABLE_ATOMIC_ALIGNMENT_FIX)
  # /Ox full opt                        /Ob2 any suitable inline function expansion
  # /Oi enable intrinsics               /Ot favor fast code
  # /GL whole program optimization      /arch:SSE2 simd extensions
  # /fp:fast floating point model       /Qpar enable parallel code gen
  # /Oy omit frame pointers             /GT fiber-safe optimizations
  list(APPEND MSVC_REL /Ox /GL /Ob2 /Oi /Ot /Oy /fp:fast /fp:except- /Qpar /GT /GL)
  list(APPEND MSVC_RDEB /Ox /Ob2 /Oi /Ot /Oy /fp:fast /Qpar /GT /GL)

  if(ARCH EQUAL 32)
    list(APPEND MSVC_REL /arch:SSE2)
    list(APPEND MSVC_RDEB /arch:SSE2)
  elseif(ARCH EQUAL 64)
    list(APPEND MSVC_REL /arch:AVX2)
    list(APPEND MSVC_RDEB /arch:AVX2)
  endif()
    
  add_compile_options("$<$<CONFIG:Release>:${MSVC_REL}>")
  add_compile_options("$<$<CONFIG:RelWithDebInfo>:${MSVC_RDEB}>")
  add_compile_options("$<$<CONFIG:Debug>:${MSVC_DEB}>")

  # Linker optimizations
  # /LTCG link time code generation
  # /OPT:REF remove code that is never referenced
  # /OPT:ICF perform identical COMDAT folding
  set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
endif()

# configurations for all compilers
add_compile_options("$<$<CONFIG:RelWithDebInfo>:-D_RELWITHDEBINFO>")

##
# Configure compiled dependencies 
##

set(BUILD_SHARED_LIBS False)

# Add boost
message(STATUS "Boost root: ${BOOST_ROOT}")
message(STATUS "Boost root (env): $ENV{BOOST_ROOT}")
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

##
# pybind11
##
add_subdirectory(${LIBS_DIR}/pybind11)

##
# nanogui
##
# Disable building extras we won't need (pure C++ project)
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED  OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED  OFF CACHE BOOL " " FORCE)

# Add the configurations from nanogui
add_subdirectory(${LIBS_DIR}/nanogui)

# For reliability of parallel build, make the NanoGUI targets dependencies
set_property(TARGET glfw nanogui PROPERTY FOLDER "dependencies")

#################################
# Configure include directories #
#################################

##
# WDL
##
include_directories(${LIBS_DIR}/wdl)

##
# IPlug
##
set(IPLUG_FILES
  ${LIBS_DIR}/wdl/iplug/Containers.h ${LIBS_DIR}/wdl/iplug/Hosts.cpp ${LIBS_DIR}/wdl/iplug/Hosts.h
  ${LIBS_DIR}/wdl/iplug/IPlugBase.cpp ${LIBS_DIR}/wdl/iplug/IPlugBase.h
  ${LIBS_DIR}/wdl/iplug/IPlugOSDetect.h ${LIBS_DIR}/wdl/iplug/IPlugStructs.cpp
  ${LIBS_DIR}/wdl/iplug/IPlugStructs.h ${LIBS_DIR}/wdl/iplug/IPlugVST.cpp
  ${LIBS_DIR}/wdl/iplug/IPlugVST.h ${LIBS_DIR}/wdl/iplug/IPlug_Prefix.pch
  ${LIBS_DIR}/wdl/iplug/IPlug_include_in_plug_hdr.h
  ${LIBS_DIR}/wdl/iplug/IPlug_include_in_plug_src.h ${LIBS_DIR}/wdl/iplug/Log.cpp
  ${LIBS_DIR}/wdl/iplug/Log.h
)
include_directories(${LIBS_DIR}/wdl/iplug)


##
# JSON for Modern C++
##
include_directories(${LIBS_DIR}/json)


##
# ETL
##
include_directories(${LIBS_DIR}/etl/include)

##
# Add VOSIMLib
##
add_subdirectory(${VOSIMLIB_DIR} VOSIMLib)

##
# Add VOSIMSynth
##
add_subdirectory(${VOSIMSYNTH_DIR} VOSIMSynth)

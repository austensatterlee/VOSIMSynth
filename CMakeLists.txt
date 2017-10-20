CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMProject)
enable_language(C CXX RC)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

macro(dbgmsg varname)
  message(STATUS "${varname}: ${${varname}}")
endmacro()

macro(setenv varname value)
    if(DEFINED ENV{${varname}})
        if(${ARGC} GREATER 2)
            set(${varname} $ENV{${varname}} ${ARGV2} ${ARGV3} "${ARGV4}" ${ARGV5})
        else()
            set(${varname} $ENV{${varname}})
        endif()
    elseif(NOT ${varname} STREQUAL "")
        if(${ARGC} GREATER 2)
            set(${varname} ${value} ${ARGV2} ${ARGV3} "${ARGV4}" ${ARGV5})
        else()
            set(${varname} ${value})
        endif()
    endif()
endmacro()

macro(optionenv varname desc value)
    if(DEFINED ENV{${varname}})
        option(${varname} ${desc} $ENV{${varname}})
    else()
        option(${varname} ${desc} ${value})
    endif()
endmacro()


# Define a macro that helps defining an option
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

#  Source/library directory options
setenv(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs CACHE PATH "")

setenv(PYTHON_EXECUTABLE_32 "C:/Python27_32/python.exe" CACHE PATH "")
setenv(PYTHON_EXECUTABLE_64 "C:/Python27/python.exe" CACHE PATH "")
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set(ARCH 64)
  set(LIBS_DIR_SYS "C:/Program Files")
  set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE_64})
else()
  set(ARCH 32)
  set(LIBS_DIR_SYS "C:/Program Files (x86)")
  set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE_32})
endif()

setenv(FFTS_ROOT ${LIBS_DIR_SYS}/ffts CACHE PATH "")
setenv(FFTS_INCLUDE_DIR ${FFTS_ROOT}/include CACHE PATH "")
setenv(BOOST_ROOT "C:/users/austen/OneDrive/Code/libs/boost/boost_1_64_0" CACHE PATH "Boost directory (i.e. .../boost/boost_1_60)")
setenv(MKL_DIR "E:/Program Files (x86)/IntelSWTools/compilers_and_libraries/windows/mkl" CACHE PATH "")

# Options
optionenv(EIGEN_USE_MKL "Should eigen use MKL as its backend?" TRUE)
optionenv(VOSIMPROJECT_USE_AVX2 "Compile with AVX2 optimizations?" FALSE)
optionenv(VOSIMPROJECT_TRACER_BUILD "Enable tracing?" FALSE)
optionenv(VOSIMPROJECT_BUILD_TESTS "Build tests?" TRUE)
optionenv(VOSIMPROJECT_BUILD_BENCHMARKS "Build benchmarks?" FALSE)
setenv(VOSIMSYNTH_GL_VERSION 3.3 CACHE STRING "Version of OpenGL to use when creating GUI." )
set_property(CACHE VOSIMSYNTH_GL_VERSION PROPERTY STRINGS 3.3 3.1)

# Subproject directories
set(VOSIMLIB_DIR ${CMAKE_SOURCE_DIR}/vosimlib)
set(VOSIMSYNTH_DIR ${CMAKE_SOURCE_DIR}/vosimsynth)

message (STATUS "Build tool is ${CMAKE_BUILD_TOOL}")
message (STATUS "System is ${CMAKE_SYSTEM_NAME}")

##
# Set compiler/linker settings
##

if(MSVC)
  # Disable some warnings
  set(MSVC_DISABLED_WARNINGS /wd4996 /wd4224 /wd4244 /wd4091 /wd4018 /wd4503 /wd4390)
  add_compile_options(${MSVC_DISABLED_WARNINGS})
  add_definitions(-D_ENABLE_ATOMIC_ALIGNMENT_FIX)

  # Runtime library, exceptions, parallel compilation
  set(MSVC_REL /EHsc /MT /MP /Z7)
  set(MSVC_RDEB /EHsc /MT /MP /Z7)
  set(MSVC_DEB /EHsc /MTd /MP /Z7)
  # /Ox Full opt                        /Ob2 Any suitable inline function expansion
  #                                     /Ob0 Disable inlining
  # /Oi Enable intrinsics               /Ot Favor fast code
  # /GL Whole program optimization      /arch:SSE2 SIMD extensions
  # /fp:fast Floating point model       /Qpar Enable parallel code gen
  # /Oy Omit frame pointers             /GT Fiber-safe optimizations
  list(APPEND MSVC_REL  /Ox  /Ob2  /Oi  /Ot  /Oy  /fp:fast  /fp:except-  /Qpar  /GT  /GL)
  list(APPEND MSVC_RDEB /Ox  /Ob0  /Oi  /Ot  /Oy-  /fp:fast  /fp:except-  /Qpar  /GT /GL)

  if(VOSIMPROJECT_USE_AVX2)
    list(APPEND MSVC_REL /arch:AVX2)
    list(APPEND MSVC_RDEB /arch:AVX2)
  else()
    list(APPEND MSVC_REL /arch:AVX)
    list(APPEND MSVC_RDEB /arch:AVX)
  endif()
    
  add_compile_options("$<$<CONFIG:Release>:${MSVC_REL}>")
  add_compile_options("$<$<CONFIG:RelWithDebInfo>:${MSVC_RDEB}>")
  add_compile_options("$<$<CONFIG:Debug>:${MSVC_DEB}>")

  # Linker optimizations
  # /LTCG link time code generation
  # /OPT:REF remove code that is never referenced
  # /OPT:ICF perform identical COMDAT folding
  set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")

  set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "/LTCG")
  set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF")
endif()

# configurations for all compilers
add_compile_options("$<$<CONFIG:RelWithDebInfo>:-D_RELWITHDEBINFO>")
if(VOSIMPROJECT_TRACER_BUILD)
  add_definitions(-DTRACER_BUILD)
endif()
if(VOSIMPROJECT_BUILD_TESTS)
    enable_testing()
endif()

##
# Configure compiled dependencies 
##

# FFTS
find_library(FFTS_RELEASE_LIBRARY ffts ffts_static PATHS ${FFTS_ROOT} PATH_SUFFIXES lib)
find_library(FFTS_DEBUG_LIBRARY fftsd ffts_staticd PATHS ${FFTS_ROOT} PATH_SUFFIXES lib)
if(FFTS_RELEASE_LIBRARY STREQUAL "FFTS_RELEASE_LIBRARY-NOTFOUND")
    message(WARNING "Unable to locate FFTS release library.")
endif()
if(FFTS_DEBUG_LIBRARY STREQUAL "FFTS_DEBUG_LIBRARY-NOTFOUND")
    message(WARNING "Unable to locate FFTS debug library.")
endif()
if(NOT FFTS_RELEASE_LIBRARY STREQUAL "FFTS_RELEASE_LIBRARY-NOTFOUND" AND NOT FFTS_DEBUG_LIBRARY STREQUAL "FFTS_DEBUG_LIBRARY-NOTFOUND")
    include_directories(${FFTS_INCLUDE_DIR})
    list(APPEND DBG_LIBS ${FFTS_DEBUG_LIBRARY})
    list(APPEND REL_LIBS ${FFTS_RELEASE_LIBRARY})
endif()

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
  ${LIBS_DIR}/wdl/IPlug/Containers.h ${LIBS_DIR}/wdl/IPlug/Hosts.cpp ${LIBS_DIR}/wdl/IPlug/Hosts.h
  ${LIBS_DIR}/wdl/IPlug/IPlugBase.cpp ${LIBS_DIR}/wdl/IPlug/IPlugBase.h
  ${LIBS_DIR}/wdl/IPlug/IPlugOSDetect.h ${LIBS_DIR}/wdl/IPlug/IPlugStructs.cpp
  ${LIBS_DIR}/wdl/IPlug/IPlugStructs.h ${LIBS_DIR}/wdl/IPlug/IPlugVST.cpp
  ${LIBS_DIR}/wdl/IPlug/IPlugVST.h ${LIBS_DIR}/wdl/IPlug/IPlug_Prefix.pch
  ${LIBS_DIR}/wdl/IPlug/IPlug_include_in_plug_hdr.h
  ${LIBS_DIR}/wdl/IPlug/IPlug_include_in_plug_src.h ${LIBS_DIR}/wdl/IPlug/Log.cpp
  ${LIBS_DIR}/wdl/IPlug/Log.h
)

##
# MKL
##
set(MKL_INTEGER_MODE LP)
set(MKL_THREADING_MODE INTELTHREAD)
find_package(MKL)
if(EIGEN_USE_MKL AND MKL_FOUND)
  add_definitions(-DEIGEN_USE_MKL_ALL)
endif()

##
# JSON for Modern C++
##
include_directories(${LIBS_DIR}/json)

##
# Add VOSIMLib
##
add_subdirectory(${VOSIMLIB_DIR} VOSIMLib)

##
# nanogui
##

# Parse and set the requested OpenGL version.
string(REPLACE "." ";" VOSIMSYNTH_GL_VERSION ${VOSIMSYNTH_GL_VERSION})
list(GET VOSIMSYNTH_GL_VERSION 0 VOSIMSYNTH_GL_VERSION_MAJOR)
list(GET VOSIMSYNTH_GL_VERSION 1 VOSIMSYNTH_GL_VERSION_MINOR)
add_definitions(-DGL_VERSION_MAJOR=${VOSIMSYNTH_GL_VERSION_MAJOR})
add_definitions(-DGL_VERSION_MINOR=${VOSIMSYNTH_GL_VERSION_MINOR})
if(VOSIMSYNTH_GL_VERSION_MAJOR EQUAL 3)
  add_definitions(-DNANOVG_GL3_IMPLEMENTATION)
elseif(VOSIMSYNTH_GL_VERSION_MAJOR EQUAL 2)
  add_definitions(-DNANOVG_GL2_IMPLEMENTATION)
else()
  message(FATAL_ERROR "Use OpenGL version 2 or 3 (VOSIMSYNTH_GL_VERSION_MAJOR is set to ${VOSIMSYNTH_GL_VERSION_MAJOR}, must be set to either 2 or 3).")
endif()

# Disable building extras we won't need (pure C++ project)
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL "" FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED  OFF CACHE BOOL " " FORCE)

# add_definitions(-DFONS_USE_FREETYPE)
# Add the configurations from nanogui
add_subdirectory(${LIBS_DIR}/nanogui)
# For reliability of parallel build, make the NanoGUI targets dependencies
set_property(TARGET glfw nanogui PROPERTY FOLDER "dependencies")

add_definitions(${NANOGUI_EXTRA_DEFS})
link_libraries(nanogui ${NANOGUI_EXTRA_LIBS})
include_directories(${NANOGUI_EXTRA_INCS})
include_directories(${LIBS_DIR}/nanogui/include)

##
# Add VOSIMSynth
##
add_subdirectory(${VOSIMSYNTH_DIR} VOSIMSynth)

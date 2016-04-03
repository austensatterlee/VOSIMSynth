CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMProject C CXX)

set(PLUGIN_NAME ${CMAKE_PROJECT_NAME})
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# edit available configrations to make them available in ide that support
# multiple-configuration (for example visual studio)
# has to be between "project" and "enable_language" to work as intended!
if(CMAKE_CONFIGURATION_TYPES)
  list(APPEND CMAKE_CONFIGURATION_TYPES DBGTRACE RELDBGTRACE)
  list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING 
    "append user-defined configuration to list of configurations to make it usable in visual studio" FORCE)
endif()


if (MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -fpermissive -wno-unknown-pragmas")
  # seems to be used in the whole c::b project
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wno-write-strings")
  # compile the runtime as static (this will make sure that libgcc_s_seh-1.dll et al. won't show up in DEPENDENCY walker)
  set(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
  set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

  if (WIN32)
    # some flags apparently needed to compile with gcc
    add_definitions(-Dstricmp=strcasecmp)
    add_definitions(-DWIN32)
    add_definitions(-DNTDDI_VERSION=0x05010000)
    add_definitions(-D_WIN32_WINNT=0x0501)
    add_definitions(-DWINVER=0x0501)
    add_definitions(-D_WIN32_IE=0x0500)
  endif()

  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math  -D__NO_INLINE__")

  set(CMAKE_RC_COMPILER_INIT windres)
  enable_language(RC)
  set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif (MINGW)

if(MSVC)
  # Runtime library
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

  # Disable some warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4224 /wd4244 /wd4091 /wd4018")
  # /Ox full opt
  # /Ob2 any suitable inline function expansion
  # /Oi enable intrinsics
  # /Ot favor fast code
  # /GL whole program optimization
  # /arch:SSE2 simd extensions
  # /fp:fast fast floating point model
  # /Qpar enable parallel code gen
  # /Oy omit frame pointers
  # /GT fiber-safe optimizations
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Ob2 /Oi /Ot /GL /arch:SSE2 /fp:fast /Qpar /Oy /GT")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Ob2 /Oi /arch:SSE2 /fp:fast /GT")

  # linker optimizations
  # /LTCG link time code generation
  # /OPT:REF remove code that is never referenced
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF")
  set(CMAKE_SHARED_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /INCREMENTAL:NO /OPT:REF")

  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

# configurations for all compilers
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUGLIBPNG")
add_definitions(-DWDL_FFT_REALSIZE=8)
add_definitions(-DPNG_NO_ASSEMBLER_CODE -DPNG_LIBPNG_SPECIALBUILD)
add_definitions(-D_CRT_SECURE_NO_DEPRECATE) 

# enable tracer build configurations
set(CMAKE_CXX_FLAGS_RELDBGTRACE "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DTRACER_BUILD")
set(CMAKE_EXE_LINKER_FLAGS_RELDBGTRACE "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_SHARED_LINKER_FLAGS_RELDBGTRACE "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")

mark_as_advanced(
  CMAKE_CXX_FLAGS_RELDBGTRACE
  CMAKE_EXE_LINKER_FLAGS_RELDBGTRACE
  CMAKE_SHARED_LINKER_FLAGS_RELDBGTRACE 
  )

set(CMAKE_CXX_FLAGS_DBGTRACE "${CMAKE_CXX_FLAGS_DEBUG} -DTRACER_BUILD")
set(CMAKE_EXE_LINKER_FLAGS_DBGTRACE "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_DBGTRACE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")

mark_as_advanced(
  CMAKE_CXX_FLAGS_DBGTRACE
  CMAKE_EXE_LINKER_FLAGS_DBGTRACE
  CMAKE_SHARED_LINKER_FLAGS_DBGTRACE 
  )

# message(STATUS "dbgtrace flags: ")
# message(STATUS "cxx: ${CMAKE_CXX_FLAGS_DBGTRACE}")
# message(STATUS "exe linker: ${CMAKE_EXE_LINKER_FLAGS_DBGTRACE}")
# message(STATUS "shared linker: ${CMAKE_SHARED_LINKER_FLAGS_DBGTRACE}")
# message(STATUS "dbgtrace flags: ")
# message(STATUS "cxx: ${CMAKE_CXX_FLAGS_RELDBGTRACE}")
# message(STATUS "exe linker: ${CMAKE_EXE_LINKER_FLAGS_RELDBGTRACE}")
# message(STATUS "shared linker: ${CMAKE_SHARED_LINKER_FLAGS_RELDBGTRACE}")

# begin collecting sources
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs)
set(WDL_DIR ${LIBS_DIR}/wdl)
set(LICE_DIR ${WDL_DIR}/lice)
set(IPLUG_DIR ${WDL_DIR}/iplug)
set(VOSIMLIB_DIR ${CMAKE_SOURCE_DIR}/vosimlib)
set(VOSIMSYNTH_DIR ${CMAKE_SOURCE_DIR}/vosimsynth)
set(STANDALONE_DIR ${VOSIMSYNTH_DIR}/standalone)

message (STATUS "Path to wdl is ${WDL_DIR}")
message (STATUS "Path to lice is ${LICE_DIR}")
message (STATUS "Path to iplug is ${IPLUG_DIR}")
message (STATUS "Path to vosimlib is ${VOSIMLIB_DIR}")
message (STATUS "Path to vosimsynth is ${VOSIMSYNTH_DIR}")
message (STATUS "Path to vosimsynth standalone is ${STANDALONE_DIR}")
message (STATUS "Build tool is ${CMAKE_BUILD_TOOL}")
message (STATUS "Build type is ${CMAKE_BUILD_TYPE}")
message (STATUS "System is ${CMAKE_SYSTEM_NAME}")

##
# WDL source files
##
set(WDL_FILES ${WDL_DIR}/fft.c ${WDL_DIR}/fft.h)

##
# VOSIMSynth source files
##
file(GLOB PROJECT_SRCS ${VOSIMSYNTH_DIR}/src/*.cpp)
set(PROJECT_SRCS ${PROJECT_SRCS};${VOSIMSYNTH_DIR}/vosimsynth.cpp)
file(GLOB PROJECT_HDRS ${VOSIMSYNTH_DIR}/include/*.h)
set(PROJECT_FILES ${PROJECT_SRCS};${PROJECT_HDRS})

##
# VOSIMLib source files
##
file(GLOB VOSIMLIB_HDRS ${VOSIMLIB_DIR}/include/*.h)
file(GLOB VOSIMLIB_SRCS ${VOSIMLIB_DIR}/src/*.cpp)
file(GLOB VOSIMLIB_UNIT_HDRS ${VOSIMLIB_DIR}/units/include/*.h)
file(GLOB VOSIMLIB_UNIT_SRCS ${VOSIMLIB_DIR}/units/src/*.cpp)
set(VOSIMLIB_FILES ${VOSIMLIB_HDRS};${VOSIMLIB_UNIT_HDRS})
set(VOSIMLIB_FILES ${VOSIMLIB_FILES};${VOSIMLIB_SRCS};${VOSIMLIB_UNIT_SRCS})

set(RESOURCE_FILES ${VOSIMSYNTH_DIR}/vosimsynth.rc ${VOSIMSYNTH_DIR}/include/resource.h)
set_source_files_properties(${VOSIMSYNTH_DIR}/vosimsynth.rc LANGUAGE RC)

# Add lice and iplug projects
add_subdirectory(${LICE_DIR} LICELib)
add_subdirectory(${IPLUG_DIR} IPlugLib)

# Add boost
set(Boost_NO_BOOST_CMAKE True)
set(Boost_DETAILED_FAILURE_MSG True)
find_package(Boost)
message(STATUS "Boost include dirs (post-search): ${Boost_INCLUDE_DIRS}")
include_directories(${Boost_INCLUDE_DIRS})

# Include directories
include_directories(${WDL_DIR})
include_directories(${VOSIMLIB_DIR}/include)
include_directories(${VOSIMLIB_DIR}/units/include)
include_directories(${VOSIMSYNTH_DIR}/include)
include_directories(${IPLUG_DIR})
include_directories(${LICE_DIR})

# Add standalone project
add_subdirectory(${STANDALONE_DIR} standalone)

add_definitions(-DVST_API -DDLL_BUILD -DVST_FORCE_DEPRECATED)

# add source groups ("filters" for MSVC)
source_group(VOSIMLib FILES ${VOSIMLIB_SRCS})
source_group(VOSIMLib\\include FILES ${VOSIMLIB_HDRS})
source_group(VOSIMLib-units FILES ${VOSIMLIB_UNIT_SRCS})
source_group(VOSIMLib-units\\include FILES ${VOSIMLIB_UNIT_HDRS})

source_group(VOSIMSynth\\include FILES ${PROJECT_HDRS})
source_group(VOSIMSynth FILES ${PROJECT_SRCS})
source_group(Resources FILES ${RESOURCE_FILES})
source_group(WDL FILES ${WDL_FILES})


add_library(${PLUGIN_NAME} SHARED
  ${PROJECT_FILES}
  ${VOSIMLIB_FILES}
  ${RESOURCE_FILES}
  ${WDL_FILES}
  )

target_link_libraries(${PLUGIN_NAME} LICELib IPlugLib)

#add_library(VOSIMLIB STATIC IMPORTED)
#get_filename_component(VOSIMLIB_PATH ${VOSIMLIB_DIR}/build/release/libvosimlib.a absolute)
#set_target_properties(VOSIMLIB PROPERTIES IMPORTED_LOCATION ${VOSIMLIB_PATH})
#target_link_libraries(${PLUGIN_NAME} VOSIMLIB)

if (WIN32)
  target_link_libraries(${PLUGIN_NAME} comctl32 wininet)
  set_target_properties(${PLUGIN_NAME} PROPERTIES prefix "")
endif ()

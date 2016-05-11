CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(VOSIMProject)
enable_language(C CXX RC)

set(PLUGIN_NAME ${CMAKE_PROJECT_NAME})

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

# define source directories
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs CACHE PATH "")
set(NANOVG_DIR ${LIBS_DIR}/nanovg CACHE PATH "")
set(WDL_DIR ${LIBS_DIR}/wdl CACHE PATH "")
set(IPLUG_DIR ${WDL_DIR}/iplug CACHE PATH "")
set(LICE_DIR ${WDL_DIR}/lice CACHE PATH "")
set(VOSIMLIB_DIR ${CMAKE_SOURCE_DIR}/vosimlib CACHE PATH "")
set(VOSIMSYNTH_DIR ${CMAKE_SOURCE_DIR}/vosimsynth CACHE PATH "")
set(STANDALONE_DIR ${VOSIMSYNTH_DIR}/standalone CACHE PATH "")
set(VST2_SDK ${LIBS_DIR}/VST_SDK CACHE PATH "")
set(SFTOOLS_DIR ${LIBS_DIR}/sftools CACHE PATH "")
set(EIGEN_DIR ${LIBS_DIR}/eigen CACHE PATH "")
set(RESOURCES_DIR ${VOSIMSYNTH_DIR}/resources CACHE PATH "")

message (STATUS "Path to nanovg is ${NANOVG_DIR}")
message (STATUS "Path to sftools is ${SFTOOLS_DIR}")
message (STATUS "Path to wdl is ${WDL_DIR}")
message (STATUS "Path to iplug is ${IPLUG_DIR}")
message (STATUS "Path to lice is ${LICE_DIR}")
message (STATUS "Path to vosimlib is ${VOSIMLIB_DIR}")
message (STATUS "Path to vosimsynth is ${VOSIMSYNTH_DIR}")
message (STATUS "Path to vosimsynth standalone is ${STANDALONE_DIR}")
message (STATUS "Path to VST2SDK is ${VST2_SDK}")

message (STATUS "Build tool is ${CMAKE_BUILD_TOOL}")
message (STATUS "System is ${CMAKE_SYSTEM_NAME}")

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
  set(MSVC_RDEB /EHsc /MTd)
  set(MSVC_DEB /EHsc /MTd)

  # Disable some warnings
  set(MSVC_DISABLED_WARNINGS /wd4996 /wd4224 /wd4244 /wd4091 /wd4018)
  add_compile_options("${MSVC_DISABLED_WARNINGS}")
  add_definitions(-D_ENABLE_ATOMIC_ALIGNMENT_FIX)
  # /Ox full opt                        /Ob2 any suitable inline function expansion
  # /Oi enable intrinsics               /Ot favor fast code
  # /GL whole program optimization      /arch:SSE2 simd extensions
  # /fp:fast fast floating point model  /Qpar enable parallel code gen
  # /Oy omit frame pointers             /GT fiber-safe optimizations
  list(APPEND MSVC_REL /Ox /Ob2 /Oi /Ot /GL /arch:SSE2 /fp:fast /Qpar /Oy /GT)
  list(APPEND MSVC_RDEB /Ob2 /Oi /arch:SSE2 /fp:fast /GT)
  add_compile_options("$<$<CONFIG:Release>:${MSVC_REL}>")
  add_compile_options("$<$<CONFIG:RelWithDebInfo>:${MSVC_RDEB}>")
  add_compile_options("$<$<CONFIG:Debug>:${MSVC_DEB}>")

  # Linker optimizations
  # /LTCG link time code generation
  # /OPT:REF remove code that is never referenced
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/LTCG /INCREMENTAL:NO /OPT:REF")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /INCREMENTAL:NO /OPT:REF")
endif()

# configurations for all compilers
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUGLIBPNG" CACHE STRING "")
add_definitions(-DWDL_FFT_REALSIZE=8)
add_definitions(-DPNG_NO_ASSEMBLER_CODE -DPNG_LIBPNG_SPECIALBUILD) 


################################
# Collect sources
################################

##
# WDL 
##
set(WDL_FILES ${WDL_DIR}/fft.c ${WDL_DIR}/fft.h)

##
# libs
##

##
# VOSIMSynth 
##
file(GLOB PROJECT_SRCS ${VOSIMSYNTH_DIR}/src/*.cpp)
set(PROJECT_SRCS ${PROJECT_SRCS};${VOSIMSYNTH_DIR}/vosimsynth.cpp)
file(GLOB PROJECT_HDRS ${VOSIMSYNTH_DIR}/include/*.h)
set(PROJECT_FILES ${PROJECT_SRCS};${PROJECT_HDRS})

set(RESOURCE_FILES ${VOSIMSYNTH_DIR}/vosimsynth.rc ${VOSIMSYNTH_DIR}/include/resource.h)
set_source_files_properties(${VOSIMSYNTH_DIR}/vosimsynth.rc LANGUAGE RC)

# compile resources into C code to be compiled into the exe
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

list(APPEND RESOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/vosimsynth_resources.cpp)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

##
# IPlug source files
##
set(IPLUG_FILES
  ${IPLUG_DIR}/Containers.h ${IPLUG_DIR}/Hosts.cpp ${IPLUG_DIR}/Hosts.h
  ${IPLUG_DIR}/IControl.cpp ${IPLUG_DIR}/IControl.h
  ${IPLUG_DIR}/IGraphics.cpp ${IPLUG_DIR}/IGraphics.h
  ${IPLUG_DIR}/IParam.cpp ${IPLUG_DIR}/IParam.h ${IPLUG_DIR}/IPlugBase.cpp
  ${IPLUG_DIR}/IPlugBase.h ${IPLUG_DIR}/IPlugOSDetect.h
  ${IPLUG_DIR}/IPlugStructs.cpp ${IPLUG_DIR}/IPlugStructs.h
  ${IPLUG_DIR}/IPlugVST.cpp ${IPLUG_DIR}/IPlugVST.h
  ${IPLUG_DIR}/IPlug_Prefix.pch ${IPLUG_DIR}/IPlug_include_in_plug_hdr.h
  ${IPLUG_DIR}/IPlug_include_in_plug_src.h ${IPLUG_DIR}/IPopupMenu.cpp
  ${IPLUG_DIR}/IPopupMenu.h ${IPLUG_DIR}/Log.cpp ${IPLUG_DIR}/Log.h
)

if (WIN32)
  add_definitions(-D__OS_WINDOWS__)
  list(APPEND IPLUG_FILES ${IPLUG_DIR}/IGraphicsWin.cpp;${IPLUG_DIR}/IGraphicsWin.h)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-D__OS_MACOSX__)
  list(APPEND IPLUG_FILES ${IPLUG_DIR}/IGraphicsCarbon.h;${IPLUG_DIR}/IGraphicsMac.h;${IPLUG_DIR}/IGraphicsMac.cpp)
endif ()

##
# NanoVG source files
##
add_definitions(-DNANOVG_GL3_IMPLEMENTATION -DNANOVG_GLEW)
file(GLOB NANOVG_FILES ${NANOVG_DIR}/src/*.c ${NANOVG_DIR}/src/*.h)

##
# sftools source files
##
file(GLOB SFTOOLS_FILES ${SFTOOLS_DIR}/include/sftools/*.hpp
  ${SFTOOLS_DIR}/include/sftools/Animation/*.hpp
  ${SFTOOLS_DIR}/include/sftools/ResourceManager/*.hpp
  ${SFTOOLS_DIR}/include/sftools/Common/*.hpp
  ${SFTOOLS_DIR}/include/sftools/Singleton/*.hpp)

################################
# Compile dependencies
################################

list(APPEND CMAKE_PREFIX_PATH "C:/Program Files (x86)/glew")
list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/sfml")
list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)

set(BUILD_SHARED_LIBS false)

# Add boost
set(Boost_NO_BOOST_CMAKE True)
set(Boost_DETAILED_FAILURE_MSG True)
find_package(Boost REQUIRED)
#message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
include_directories(${Boost_INCLUDE_DIRS})

set(SFML_STATIC true)
set(SFML_STATIC_LIBRARIES true)
add_definitions(-DSFTOOLS_NO_AUDIO)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

# find SFML
find_package(SFML 2 REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})
set(SFML_DBG_LIBS ${SFML_SYSTEM_LIBRARY_DEBUG} ${SFML_WINDOW_LIBRARY_DEBUG} ${SFML_GRAPHICS_LIBRARY_DEBUG})
set(SFML_REL_LIBS ${SFML_SYSTEM_LIBRARY_RELEASE} ${SFML_WINDOW_LIBRARY_RELEASE} ${SFML_GRAPHICS_LIBRARY_RELEASE})
list(APPEND ADDITIONAL_LIBRARIES ${SFML_DEPENDENCIES})
message(STATUS "SFML debug libs: ${SFML_DBG_LIBS}" )
message(STATUS "SFML release libs: ${SFML_REL_LIBS}" )
message(STATUS "SFML deps: ${SFML_DEPENDENCIES}" )

# find glew
find_package(glew REQUIRED)
add_definitions(-DGLEW_MX -DGLEW_STATIC)
include_directories(${GLEW_INCLUDE_DIR})
#message(STATUS "GLEW libs: ${GLEW_LIBRARY}" )
string(REGEX REPLACE "(.*)\\.(.*)" "\\1$<$<CONFIG:Debug>:d>.\\2" GLEW_LIBRARY "${GLEW_LIBRARY}")
list(APPEND ADDITIONAL_LIBRARIES ${GLEW_LIBRARY})

# find OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
list(APPEND ADDITIONAL_LIBRARIES ${OPENGL_LIBRARIES})
#message(STATUS "OPENGL libs: ${OPENGL_LIBRARIES}" )

list(REMOVE_DUPLICATES ADDITIONAL_LIBRARIES)

################################
# Include header directories
################################

include_directories(${NANOVG_DIR}/src)
include_directories(${WDL_DIR})
include_directories(${IPLUG_DIR})
include_directories(${LICE_DIR})
include_directories(${VOSIMSYNTH_DIR}/include)
include_directories(${SFTOOLS_DIR}/include)
include_directories(${EIGEN_DIR})

# LICE
add_subdirectory(${LICE_DIR} "libs/LICELib")

# IPlug
# add_subdirectory(${IPLUG_DIR} "libs/IPlugLib")

# VOSIMLib
include_directories(${VOSIMLIB_DIR}/include)
include_directories(${VOSIMLIB_DIR}/units/include)
add_subdirectory(${VOSIMLIB_DIR} VOSIMLib)

# Add standalone project
add_subdirectory(${STANDALONE_DIR} "standalone")

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

target_link_libraries(${PLUGIN_NAME} ${ADDITIONAL_LIBRARIES} "$<$<CONFIG:Release>:${SFML_REL_LIBS}>" "$<$<NOT:$<CONFIG:Release>>:${SFML_DBG_LIBS}>")
target_link_libraries(${PLUGIN_NAME} VOSIMLib LICELib)

if (WIN32)
  target_link_libraries(${PLUGIN_NAME} comctl32 wininet)
  set_target_properties(${PLUGIN_NAME} PROPERTIES prefix "")
endif ()

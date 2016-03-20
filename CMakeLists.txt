CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
PROJECT(VOSIMProject)
SET(PLUGIN_NAME ${CMAKE_PROJECT_NAME})

#SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DPNG_NO_ASSEMBLER_CODE -DPNG_LIBPNG_SPECIALBUILD -DDLL_BUILD")

IF (MINGW)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -fpermissive -Wno-unknown-pragmas")
    # Seems to be used in the whole C::B project
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings -DVST_FORCE_DEPRECATED")
    # Flags used for Debug, Release and Tracer builds
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32 -D_CRT_SECURE_NO_DEPRECATE")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNTDDI_VERSION=0x05010000 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_WIN32_IE=0x0500")
    # Compile the runtime as static (this will make sure that LIBGCC_S_SEH-1.DLL et al. won't show up in Dependency Walker)
    SET(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

    ADD_DEFINITIONS(-Dstricmp=strcasecmp)

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -ffast-math -DPNG_NO_ASSEMBLER_CODE -DPNG_LIBPNG_SPECIALBUILD -DDLL_BUILD -DNDEBUGLIBPNG -D__NO_INLINE__")

    SET(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
ENDIF (MINGW)

if(MSVC)
	# Runtime Library
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	# Full opt /Ox
	# Any suitable inline function expansion /Ob2
	# Enable intrinsics /Oi
	# Favor fast code /Ot
	# Whole program optimization /GL
	# SIMD Extensions /arch:SSE2
	# Fast floating point model /fp:fast
	# Enable parallel code gen /Qpar
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Ob2 /Oi /Ot /GL /arch:SSE2 /fp:fast /Qpar")
endif()

ADD_DEFINITIONS(-DVST_API)
ADD_DEFINITIONS(-DWDL_FFT_REALSIZE=8)

SET(VST2_SDK libs/VST_SDK)
SET(ASIO_SDK libs/ASIO_SDK)
SET(WDL libs/WDL)
SET(LICE ${WDL}/lice)
SET(IPLUG ${WDL}/IPlug)
SET(LIBPNG ${WDL}/libpng)
SET(ZLIB ${WDL}/zlib)
SET(VOSIMLIB_DIR VOSIMLib)

message (STATUS "Path to WDL is ${WDL}")
message (STATUS "Path to IPlug is ${IPLUG}")
message (STATUS "Path to libpng is ${LIBPNG}")
message (STATUS "Path to lice is ${LICE}")
message (STATUS "Path to zlib is ${ZLIB}")
message (STATUS "Path to VOSIMLib is ${VOSIMLIB_DIR}")

##
# IPlug source files
##
SET(IPLUG_SOURCE_FILES
    ${IPLUG}/Containers.h ${IPLUG}/Hosts.cpp ${IPLUG}/Hosts.h ${IPLUG}/IControl.cpp ${IPLUG}/IControl.h
    ${IPLUG}/IGraphics.cpp ${IPLUG}/IGraphics.h ${IPLUG}/IParam.cpp ${IPLUG}/IParam.h
    ${IPLUG}/IPlugBase.cpp ${IPLUG}/IPlugBase.h ${IPLUG}/IPlugOSDetect.h ${IPLUG}/IPlugStructs.cpp ${IPLUG}/IPlugStructs.h
    ${IPLUG}/IPlugVST.cpp ${IPLUG}/IPlugVST.h ${IPLUG}/IPlug_Prefix.pch ${IPLUG}/IPlug_include_in_plug_hdr.h ${IPLUG}/IPlug_include_in_plug_src.h
    ${IPLUG}/IPopupMenu.cpp ${IPLUG}/IPopupMenu.h ${IPLUG}/Log.cpp ${IPLUG}/Log.h #  ${IPLUG}/VSTHosts.h
    )

IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    ADD_DEFINITIONS(-D__OS_WINDOWS__)
    LIST(APPEND IPLUG_SOURCE_FILES ${IPLUG}/IGraphicsWin.cpp;${IPLUG}/IGraphicsWin.h)
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    ADD_DEFINITIONS(-D__OS_MACOSX__)
    LIST(APPEND IPLUG_SOURCE_FILES ${IPLUG}/IGraphicsCarbon.h;${IPLUG}/IGraphicsMac.h;${IPLUG}/IGraphicsMac.cpp)
ENDIF ()

##
# LIBPNG source files
##
SET(LIBPNG_SOURCE_FILES
    ${LIBPNG}/png.c ${LIBPNG}/png.h ${LIBPNG}/pngconf.h ${LIBPNG}/pngerror.c ${LIBPNG}/pngget.c
    ${LIBPNG}/pngmem.c ${LIBPNG}/pngpread.c ${LIBPNG}/pngread.c ${LIBPNG}/pngrio.c ${LIBPNG}/pngrtran.c
    ${LIBPNG}/pngrutil.c ${LIBPNG}/pngset.c ${LIBPNG}/pngtrans.c ${LIBPNG}/pngwio.c ${LIBPNG}/pngwrite.c
    ${LIBPNG}/pngwtran.c ${LIBPNG}/pngwutil.c
    )
##
# LICE source files
##
SET(LICE_SOURCE_FILES
    ${LICE}/lice.cpp ${LICE}/lice.h ${LICE}/lice_arc.cpp ${LICE}/lice_bezier.h ${LICE}/lice_colorspace.cpp
    ${LICE}/lice_combine.h ${LICE}/lice_extended.h ${LICE}/lice_image.cpp ${LICE}/lice_line.cpp ${LICE}/lice_palette.cpp
    ${LICE}/lice_png.cpp ${LICE}/lice_texgen.cpp ${LICE}/lice_text.cpp ${LICE}/lice_text.h ${LICE}/lice_textnew.cpp
    )
##
# ZLib source files
##
SET(ZLIB_SOURCE_FILES ${ZLIB}/adler32.c ${ZLIB}/compress.c ${ZLIB}/crc32.c ${ZLIB}/crc32.h ${ZLIB}/deflate.c
    ${ZLIB}/deflate.h ${ZLIB}/infback.c ${ZLIB}/inffast.c ${ZLIB}/inffast.h ${ZLIB}/inffixed.h
    ${ZLIB}/inflate.c ${ZLIB}/inflate.h ${ZLIB}/inftrees.c ${ZLIB}/inftrees.h ${ZLIB}/ioapi.h
    ${ZLIB}/trees.c ${ZLIB}/trees.h ${ZLIB}/uncompr.c ${ZLIB}/unzip.h ${ZLIB}/zconf.h #  ${ZLIB}/zconf.in.h
    ${ZLIB}/zlib.h ${ZLIB}/zutil.c ${ZLIB}/zutil.h
    )

##
# WDL source files
##
SET(WDL_SOURCE_FILES ${WDL}/fft.c ${WDL}/fft.h)

##
# VOSIMSynth source files
##
FILE(GLOB PROJECT_SRCS VOSIMSynth/src/*.cpp)
SET(PROJECT_SRCS ${PROJECT_SRCS};VOSIMSynth/VOSIMSynth.cpp)
FILE(GLOB PROJECT_HDRS VOSIMSynth/include/*.h)
SET(PROJECT_SOURCE_FILES ${PROJECT_SRCS};${PROJECT_HDRS})

##
# VOSIMLib source files
##
FILE(GLOB VOSIMLIB_HDRS ${VOSIMLIB_DIR}/include/*.h)
FILE(GLOB VOSIMLIB_SRCS ${VOSIMLIB_DIR}/src/*.cpp)
FILE(GLOB VOSIMLIB_UNIT_HDRS ${VOSIMLIB_DIR}/units/include/*.h)
FILE(GLOB VOSIMLIB_UNIT_SRCS ${VOSIMLIB_DIR}/units/src/*.cpp)
SET(VOSIMLIB_FILES ${VOSIMLIB_HDRS};${VOSIMLIB_UNIT_HDRS})
SET(VOSIMLIB_FILES ${VOSIMLIB_FILES};${VOSIMLIB_SRCS};${VOSIMLIB_UNIT_SRCS})

SET(RESOURCE_FILES)

INCLUDE_DIRECTORIES(${WDL})
INCLUDE_DIRECTORIES(${IPLUG})
INCLUDE_DIRECTORIES(${VOSIMLIB_DIR}/include)
INCLUDE_DIRECTORIES(${VOSIMLIB_DIR}/units/include)
INCLUDE_DIRECTORIES(${VOSIMLIB_DIR}/external)
INCLUDE_DIRECTORIES(VOSIMSynth/include)

ADD_LIBRARY(${PLUGIN_NAME} SHARED
            ${PROJECT_SOURCE_FILES}
            ${VOSIMLIB_FILES}
            ${RESOURCE_FILES}
            ${IPLUG_SOURCE_FILES}
            ${LIBPNG_SOURCE_FILES}
            ${LICE_SOURCE_FILES}
            ${ZLIB_SOURCE_FILES}
			${WDL_SOURCE_FILES}
            )

#ADD_LIBRARY(VOSIMLib STATIC IMPORTED)
#GET_FILENAME_COMPONENT(VOSIMLIB_PATH ${VOSIMLIB_DIR}/build/release/libVOSIMLib.a ABSOLUTE)
#SET_TARGET_PROPERTIES(VOSIMLib PROPERTIES IMPORTED_LOCATION ${VOSIMLIB_PATH})
#TARGET_LINK_LIBRARIES(${PLUGIN_NAME} VOSIMLib)

IF (WIN32)
    TARGET_LINK_LIBRARIES(${PLUGIN_NAME} comctl32 wininet)
    SET_TARGET_PROPERTIES(${PLUGIN_NAME} PROPERTIES PREFIX "")
ENDIF ()

if(MSVC)
	# Link time code generation
    # set_property(TARGET ${PLUGIN_NAME} APPEND PROPERTY LINK_FLAGS /LCTG)
endif()

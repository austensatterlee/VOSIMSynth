CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(LICELib)

SET(LIBPNG_DIR ${WDL_DIR}/libpng)
SET(ZLIB_DIR ${WDL_DIR}/zlib)

message (STATUS "Path to libpng is ${LIBPNG_DIR}")
message (STATUS "Path to zlib is ${ZLIB_DIR}")

##
# LICE_DIR source files
##
SET(LICE_FILES
  ${LICE_DIR}/lice.cpp ${LICE_DIR}/lice.h ${LICE_DIR}/lice_arc.cpp ${LICE_DIR}/lice_bezier.h ${LICE_DIR}/lice_colorspace.cpp
  ${LICE_DIR}/lice_combine.h ${LICE_DIR}/lice_extended.h ${LICE_DIR}/lice_image.cpp ${LICE_DIR}/lice_line.cpp ${LICE_DIR}/lice_palette.cpp
  ${LICE_DIR}/lice_png.cpp ${LICE_DIR}/lice_texgen.cpp ${LICE_DIR}/lice_text.cpp ${LICE_DIR}/lice_text.h ${LICE_DIR}/lice_textnew.cpp
  )

##
# LIBPNG source files
##
SET(LIBPNG_FILES
  ${LIBPNG_DIR}/png.c ${LIBPNG_DIR}/png.h ${LIBPNG_DIR}/pngconf.h ${LIBPNG_DIR}/pngerror.c ${LIBPNG_DIR}/pngget.c
  ${LIBPNG_DIR}/pngmem.c ${LIBPNG_DIR}/pngpread.c ${LIBPNG_DIR}/pngread.c ${LIBPNG_DIR}/pngrio.c ${LIBPNG_DIR}/pngrtran.c
  ${LIBPNG_DIR}/pngrutil.c ${LIBPNG_DIR}/pngset.c ${LIBPNG_DIR}/pngtrans.c ${LIBPNG_DIR}/pngwio.c ${LIBPNG_DIR}/pngwrite.c
  ${LIBPNG_DIR}/pngwtran.c ${LIBPNG_DIR}/pngwutil.c
  )

##
# ZLib source files
##
SET(ZLIB_FILES ${ZLIB_DIR}/adler32.c ${ZLIB_DIR}/compress.c ${ZLIB_DIR}/crc32.c ${ZLIB_DIR}/crc32.h ${ZLIB_DIR}/deflate.c
  ${ZLIB_DIR}/deflate.h ${ZLIB_DIR}/infback.c ${ZLIB_DIR}/inffast.c ${ZLIB_DIR}/inffast.h ${ZLIB_DIR}/inffixed.h
  ${ZLIB_DIR}/inflate.c ${ZLIB_DIR}/inflate.h ${ZLIB_DIR}/inftrees.c ${ZLIB_DIR}/inftrees.h ${ZLIB_DIR}/ioapi.h
  ${ZLIB_DIR}/trees.c ${ZLIB_DIR}/trees.h ${ZLIB_DIR}/uncompr.c ${ZLIB_DIR}/unzip.h ${ZLIB_DIR}/zconf.h #  ${ZLIB_DIR}/zconf.in.h
  ${ZLIB_DIR}/zlib.h ${ZLIB_DIR}/zutil.c ${ZLIB_DIR}/zutil.h
  )

INCLUDE_DIRECTORIES(${ZLIB_DIR})
INCLUDE_DIRECTORIES(${LIBPNG_DIR})
INCLUDE_DIRECTORIES(${WDL_DIR})

SOURCE_GROUP(LICE FILES ${LICE_FILES})
SOURCE_GROUP(libpng FILES ${LIBPNG_FILES})
SOURCE_GROUP(zlib FILES ${ZLIB_FILES})

ADD_LIBRARY(LICELib STATIC ${LICE_FILES} ${LIBPNG_FILES} ${ZLIB_FILES})

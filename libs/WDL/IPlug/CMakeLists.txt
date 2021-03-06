CMAKE_MINIMUM_REQUIRED(VERSION 3.3)

project(IPlugLib)

SET(VST2_SDK ${LIBS_DIR}/VST_SDK)

message (STATUS "Path to VST2SDK is ${VST2_SDK}")

##
# IPlug source files
##
SET(IPLUG_FILES
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

# IPlug standalone files
SET(IPLUG_STANDALONE_FILES ${IPLUG_DIR}/IPlugStandalone.cpp;${IPLUG_DIR}/IPlugStandalone.h)

IF (WIN32)
  ADD_DEFINITIONS(-D__OS_WINDOWS__)
  LIST(APPEND IPLUG_FILES ${IPLUG_DIR}/IGraphicsWin.cpp;${IPLUG_DIR}/IGraphicsWin.h)
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  ADD_DEFINITIONS(-D__OS_MACOSX__)
  LIST(APPEND IPLUG_FILES ${IPLUG_DIR}/IGraphicsCarbon.h;${IPLUG_DIR}/IGraphicsMac.h;${IPLUG_DIR}/IGraphicsMac.cpp)
ENDIF ()

INCLUDE_DIRECTORIES(${WDL_DIR})
INCLUDE_DIRECTORIES(${IPLUG_DIR})
INCLUDE_DIRECTORIES(${LICE_DIR})

SOURCE_GROUP(IPlug FILES ${IPLUG_FILES} ${IPLUG_STANDALONE_FILES})
SOURCE_GROUP(WDL FILES ${WDL_FILES})

ADD_LIBRARY(IPlugLib STATIC ${IPLUG_FILES} ${IPLUG_STANDALONE_FILES} ${WDL_FILES})

TARGET_LINK_LIBRARIES(IPlugLib LICELib)

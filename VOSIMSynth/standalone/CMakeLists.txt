# Standalone app
PROJECT(VOSIMSynth_standalone)

SET(ASIO_DIR ${LIBS_DIR}/ASIO_SDK)
SET(RTMIDI_DIR ${LIBS_DIR}/wdl/RtAudioMidi)

INCLUDE_DIRECTORIES(${ASIO_DIR})
INCLUDE_DIRECTORIES(${RTMIDI_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# Main app wrapper
FILE(GLOB STANDALONE_SRCS *.cpp)
FILE(GLOB STANDALONE_HDRS *.h)
SET(STANDALONE_FILES ${STANDALONE_SRCS} ${STANDALONE_HDRS} VOSIMSynth.rc)
set_source_files_properties(VOSIMSynth.rc LANGUAGE RC)

# Standalone IPlug
set(IPLUG_STANDALONE_FILES ${LIBS_DIR}/wdl/iplug/IPlugStandalone.cpp ${LIBS_DIR}/wdl/iplug/IPlugStandalone.h)

# RtAudio
FILE(GLOB RTMIDI_HDRS ${RTMIDI_DIR}/*.h)
FILE(GLOB RTMIDI_SRCS ${RTMIDI_DIR}/*.cpp)
SET(RTMIDI_FILES ${RTMIDI_HDRS};${RTMIDI_SRCS})

# ASIO SDK
FILE(GLOB ASIO_HDRS ${ASIO_DIR}/*.h)
FILE(GLOB ASIO_SRCS ${ASIO_DIR}/*.cpp)
SET(ASIO_FILES ${ASIO_HDRS};${ASIO_SRCS})

# add source groups ("filters" for MSVC)
source_group(include/vosimsynth/widgets FILES ${PROJECT_WIDGET_HDRS})
source_group(include/vosimsynth FILES ${PROJECT_HDRS})
source_group(src FILES ${PROJECT_SRCS})
source_group(standalone FILES ${STANDALONE_FILES})
source_group(Resources FILES ${RESOURCE_FILES})
source_group(WDL FILES ${WDL_FILES})
source_group(ASIO FILES ${ASIO_FILES})
source_group(RtAudio FILES ${RTMIDI_FILES})
source_group(IPlug FILES ${IPLUG_FILES} ${IPLUG_STANDALONE_FILES})

IF(WIN32)
  ADD_DEFINITIONS(-DSA_API -D__WINDOWS_DS__ -D__WINDOWS_MM__ -D__WINDOWS_ASIO__)
ENDIF()


ADD_EXECUTABLE (${PROJECT_NAME} WIN32
  ${ALL_VOSIMSYNTH_FILES}
  ${STANDALONE_FILES}
  ${RTMIDI_FILES}
  ${ASIO_FILES}
  ${IPLUG_STANDALONE_FILES}
  )

target_link_libraries(${PROJECT_NAME} ${ADDITIONAL_LIBRARIES}
  "$<$<CONFIG:Release>:${REL_LIBS}>"
  "$<$<NOT:$<CONFIG:Release>>:${DBG_LIBS}>"
  VOSIMLib
 ${ADDITIONAL_LIBRARIES}
  )

IF(WIN32)
  target_link_libraries(${PROJECT_NAME} dsound winmm comctl32 wininet)
ENDIF()

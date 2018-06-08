PEARY_ENABLE_DEFAULT(OFF)

# Define device and return the generated name as DEVICE_NAME
PEARY_DEVICE(DEVICE_NAME)

# Add source files to library
PEARY_DEVICE_SOURCES(${DEVICE_NAME}
    CLICpix2.cpp
    clicpix2_utilities.cpp
    clicpix2_frameDecoder.cpp
    clicpix2_frameDecoder_lfsr13.cpp
    clicpix2_frameDecoder_lfsr8.cpp
    clicpix2_frameDecoder_lfsr5.cpp
)

# Provide standard install target
PEARY_DEVICE_INSTALL(${DEVICE_NAME})

SET(DECODERFILES
  clicpix2RawDecoder.cpp
  clicpix2_utilities.cpp
  clicpix2_frameDecoder.cpp
  clicpix2_frameDecoder_lfsr13.cpp
  clicpix2_frameDecoder_lfsr8.cpp
  clicpix2_frameDecoder_lfsr5.cpp
)

ADD_EXECUTABLE(clicpix2_rawdecoder ${DECODERFILES})
TARGET_LINK_LIBRARIES(clicpix2_rawdecoder ${PROJECT_NAME} ${DEVICE_NAME})

INSTALL(TARGETS clicpix2_rawdecoder
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

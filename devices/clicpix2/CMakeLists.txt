SET(NAME "CLICpix2")

SET(SOURCEFILES
  clicpix2.cpp
  clicpix2_utilities.cpp
  clicpix2_frameDecoder.cpp
  clicpix2_frameDecoder_lfsr13.cpp
  clicpix2_frameDecoder_lfsr8.cpp
  clicpix2_frameDecoder_lfsr5.cpp
  )

INCLUDE_DIRECTORIES(.)

ADD_DEFINITIONS(-DDEVICE_NAME="${NAME}")
ADD_LIBRARY(${NAME} SHARED ${SOURCEFILES})
TARGET_LINK_LIBRARIES(${NAME} ${PROJECT_NAME})

ADD_EXECUTABLE(clicpix2_rawdecoder "clicpix2RawDecoder.cpp" "clicpix2_frameDecoder.cpp" "clicpix2_utilities.cpp")
TARGET_LINK_LIBRARIES(clicpix2_rawdecoder ${PROJECT_NAME} ${NAME})


INSTALL(TARGETS ${NAME} clicpix2_rawdecoder
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

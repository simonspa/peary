# Add source files for the peary caribou core library

INCLUDE_DIRECTORIES(device utils hal driver)

SET(LIB_SOURCE_FILES
  # device library
  "device/device.cc"
  # hardware abstraction layer
  "hal/hal.cc"
  # utilities
  "utils/utils.cc"
  )

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

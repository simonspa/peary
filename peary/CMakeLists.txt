# Add source files for the peary caribou core library

INCLUDE_DIRECTORIES(device utils hal driver)

SET(LIB_SOURCE_FILES
  # device library
  "device/device.cc"
  "device/devicemgr.cc"
  # hardware abstraction layer
  "hal/hal.cc"
  # utilities
  "utils/utils.cc"
  "utils/configuration.cc"
  )

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CMAKE_DL_LIBS})

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

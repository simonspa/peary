# Add source files for the peary caribou core library

INCLUDE_DIRECTORIES(interfaces)

SET(LIB_SOURCE_FILES
  # device library
  "device/device.cpp"
  "device/devicemgr.cpp"
  # hardware abstraction layer
  "hal/hal.cpp"
  # interfaces
  "interfaces/spi.cpp"
  "interfaces/i2c.cpp"
  # utilities
  "utils/utils.cpp"
  "utils/configuration.cpp"
  )

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CMAKE_DL_LIBS})

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

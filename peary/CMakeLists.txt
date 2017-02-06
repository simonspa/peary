# Add source files for the peary caribou core library

INCLUDE_DIRECTORIES(interfaces)

# Build flag for I2C interface implementation:
OPTION(INTERFACE_I2C "Build Caribou I2C interface?" ON)
# Build flag for SPI interface implementation:
OPTION(INTERFACE_SPI "Build DTB USB interface?" OFF)

SET(LIB_SOURCE_FILES
  # device library
  "device/device.cpp"
  "device/devicemgr.cpp"
  "interfaces/interface_manager.cpp"
  # hardware abstraction layer
  "hal/hal.cpp"
  # utilities
  "utils/utils.cpp"
  "utils/configuration.cpp"
  )

IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  add_definitions(-DYOCTO_COMPILATION)
ENDIF(CMAKE_SYSTEM_PROCESSOR EQUAL arm)  

IF(INTERFACE_I2C)
  # add I2C source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/i2c.cpp"
    )
  MESSAGE(STATUS "Building Caribou I2C interface.")
ENDIF(INTERFACE_I2C)

IF(INTERFACE_SPI)
  # add SPI source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/spi.cpp"
    )
  MESSAGE(STATUS "Building Caribou SPI interface.")
ENDIF(INTERFACE_SPI)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CMAKE_DL_LIBS})
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE SHARED_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

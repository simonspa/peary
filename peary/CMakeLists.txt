# Add source files for the peary caribou core library

INCLUDE_DIRECTORIES(interfaces)

# Build flag for I2C interface implementation:
OPTION(INTERFACE_I2C "Build Caribou I2C interface?" ON)
# Build flag for SPI interface implementation:
OPTION(INTERFACE_SPI "Build Caribou SPI interface?" ON)
# Build flag for SPI CLICpix2 interface implementation:
OPTION(INTERFACE_SPI_CLICpix2 "Build Caribou SPI CLICpix2 interface?" ON)
# Build flag for IP/Socket interface implementation:
OPTION(INTERFACE_IPSOCK "Build Caribou IP/Socket interface?" ON)
# Build flag for dummy loopback interface implementation:
OPTION(INTERFACE_LOOP "Build Caribou Loopback interface?" ON)

SET(LIB_SOURCE_FILES
  # device manager
  "device/devicemgr.cpp"
  "device/device.cpp"
  # HAL base
  "carboard/halbase.cpp"
  # interface manager
  "interfaces/interface_manager.cpp"
  # utilities
  "utils/utils.cpp"
  "utils/configuration.cpp"
  )

IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  add_definitions(-DYOCTO_COMPILATION)
ENDIF(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")

IF(INTERFACE_I2C)
  # add I2C source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/i2c.cpp"
    )
  MESSAGE(STATUS "Building Caribou I2C interface.")
ELSE(INTERFACE_I2C)
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/emulators/i2c.cpp"
    )
  MESSAGE(STATUS "Building emulator for Caribou I2C interface.")
ENDIF(INTERFACE_I2C)

IF(INTERFACE_SPI_CLICpix2)
  # Also build SPI since it depends on it:
  SET(INTERFACE_SPI ON)
  # add SPI source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/spi_CLICpix2.cpp"
    )
  MESSAGE(STATUS "Building Caribou SPI CLICpix2 interface.")
ELSE(INTERFACE_SPI_CLICpix2)
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/emulators/spi_CLICpix2.cpp"
    )
  MESSAGE(STATUS "Building emulator for Caribou SPI CLICpix2 interface.")
ENDIF(INTERFACE_SPI_CLICpix2)

IF(INTERFACE_SPI)
  # add SPI source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/spi.cpp"
    )
  MESSAGE(STATUS "Building Caribou SPI interface.")
ELSE(INTERFACE_SPI)
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/emulators/spi.cpp"
    )
  MESSAGE(STATUS "Building emulator for Caribou SPI interface.")
ENDIF(INTERFACE_SPI)

IF(INTERFACE_IPSOCK)
  # add IP/Socket source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/ipsocket.cpp"
    )
  MESSAGE(STATUS "Building Caribou IP/Socket interface.")
ENDIF(INTERFACE_IPSOCK)

# Loopback interface can be always built
IF(INTERFACE_LOOP)
  # add LOOP source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/loopback.cpp"
    )
  MESSAGE(STATUS "Building Caribou Loopback interface.")
ENDIF(INTERFACE_LOOP)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${CMAKE_DL_LIBS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC Threads::Threads)
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE SHARED_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

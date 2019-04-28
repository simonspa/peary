# Add source files for the peary caribou core library

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
# Build flag for memory access interface implementation:
OPTION(INTERFACE_MEM "Build Caribou Memory interface?" ON)

SET(LIB_SOURCE_FILES
  # device manager
  "device/DeviceManager.cpp"
  "device/Device.cpp"
  # HAL base
  "carboard/halbase.cpp"
  # interface manager
  "interfaces/InterfaceManager.cpp"
  # utilities
  "utils/log.cpp"
  "utils/lfsr.cpp"
  "utils/utils.cpp"
  "utils/configuration.cpp"
  )

IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  add_definitions(-DYOCTO_COMPILATION)
ENDIF()

IF(INTERFACE_I2C)
  # add I2C source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/I2C/i2c.cpp"
    )
  MESSAGE(STATUS "Caribou Interface I2C:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/I2C/emulator.cpp"
    )
  MESSAGE(STATUS "Caribou Interface I2C:\t(emulated)")
ENDIF()

IF(INTERFACE_SPI_CLICpix2)
  # Also build SPI since it depends on it:
  SET(INTERFACE_SPI ON)
  # add SPI source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SPI_CLICpix2/spi_CLICpix2.cpp"
    )
  MESSAGE(STATUS "Caribou Interface SPI CPx2:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SPI_CLICpix2/emulator.cpp"
    )
  MESSAGE(STATUS "Caribou Interface SPI CPx2:\t(emulated)")
ENDIF()

IF(INTERFACE_SPI)
  # add SPI source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SPI/spi.cpp"
    )
  MESSAGE(STATUS "Caribou Interface SPI:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/SPI/emulator.cpp"
    )
  MESSAGE(STATUS "Caribou Interface SPI:\t(emulated)")
ENDIF()

IF(INTERFACE_IPSOCK)
  # add IP/Socket source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/IPSocket/ipsocket.cpp"
    )
  MESSAGE(STATUS "Caribou Interface IP/Socket:\tON")
ELSE()
  MESSAGE(STATUS "Caribou Interface IP/Socket:\tOFF")
ENDIF()

# Loopback interface can be always built
IF(INTERFACE_LOOP)
  # add LOOP source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/Loopback/loopback.cpp"
    )
  MESSAGE(STATUS "Caribou Interface Loopback:\tON")
ELSE()
  MESSAGE(STATUS "Caribou Interface Loopback:\tOFF")
ENDIF()

IF(INTERFACE_MEM)
  # add MEM source files
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/Memory/memory.cpp"
    )
  MESSAGE(STATUS "Caribou Interface MEM:\tON")
ELSE()
  SET(LIB_SOURCE_FILES ${LIB_SOURCE_FILES}
    "interfaces/Memory/emulator.cpp"
    )
  MESSAGE(STATUS "Caribou Interface MEM:\t(emulated)")
ENDIF()

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${CMAKE_DL_LIBS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC Threads::Threads)
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE SHARED_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}")
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE "PEARY_LOCK_DIR=\"${PEARY_LOCK_DIR}\"")

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

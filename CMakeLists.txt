########################################################
# CMake file for the peary caribou DAQ
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
########################################################

# Project name
PROJECT(peary)


###############################
# Setup the build environment #
###############################

# Additional packages to be searched for by cmake
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Set the correct build type and allow command line options:
# Set a default build type if none was specified
IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  MESSAGE(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
ENDIF()

# Configure the installation prefix to allow system-wide installation
SET(INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" CACHE PATH "Prefix prepended to install directories")
SET(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}" CACHE INTERNAL "Prefix prepended to install directories" FORCE)

# Set up the RPATH so executables find the libraries even when installed in non-default location
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
# Add the automatically determined parts of the RPATH which point to directories outside
# the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

ADD_DEFINITIONS(-std=c++11)

IF(APPLE)
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -stdlib=libstdc++" )
ENDIF(APPLE)

##################################
# Define build targets for peary #
##################################

# Include directories
INCLUDE_DIRECTORIES(peary/utils
  peary/device
  peary/hal)

# Always build main peary library;
ADD_SUBDIRECTORY(peary)

# Include optional device support
ADD_SUBDIRECTORY(devices)

# Add targets for Doxygen code reference and LaTeX User manual
ADD_SUBDIRECTORY(doc)


################################################
# Define build targets for sample applications #
################################################

ADD_EXECUTABLE(peary_app "exec/sample_application.cc")
TARGET_LINK_LIBRARIES(peary_app ${PROJECT_NAME} )

INCLUDE_DIRECTORIES(devices/example)
ADD_EXECUTABLE(peary_direct "exec/direct_device.cc")
TARGET_LINK_LIBRARIES(peary_direct ${PROJECT_NAME} CaribouExampleDevice )

INSTALL(TARGETS peary_app peary_direct
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)


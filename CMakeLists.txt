########################################################
# CMake file for the peary caribou DAQ
CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)
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
SET(CMAKE_MACOSX_RPATH 1)
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

ADD_DEFINITIONS(-std=c++14)
INCLUDE(cmake/Platform.cmake)

########################################
# Figure out the current peary version #
########################################

# Check if this is a source tarball build
IF(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/.git)
  SET(SOURCE_PACKAGE 1)
ENDIF(NOT IS_DIRECTORY ${CMAKE_SOURCE_DIR}/.git)

# Set package version
IF(NOT SOURCE_PACKAGE)
  # Get the version from last git tag plus number of additional commits:
  FIND_PACKAGE(Git QUIET)

  # Workaround for broken git detection on Yocto Poky
  IF(NOT GIT_FOUND)
    set(GIT_EXECUTABLE "/usr/bin/git")
    set(GIT_FOUND true)
    MESSAGE("-- Path of git executable forced to /usr/bin/git.")
  ENDIF(NOT GIT_FOUND)

  IF(GIT_FOUND)
    EXEC_PROGRAM(git ${CMAKE_CURRENT_SOURCE_DIR} ARGS describe --tags HEAD OUTPUT_VARIABLE PEARY_LIB_VERSION)
    STRING(REGEX REPLACE "^release-" "" PEARY_LIB_VERSION ${PEARY_LIB_VERSION})
    STRING(REGEX REPLACE "([v0-9.]+)-([0-9]+)-([A-Za-z0-9]+)" "\\1+\\2~\\3" PEARY_LIB_VERSION ${PEARY_LIB_VERSION})
    EXEC_PROGRAM(git ARGS status --porcelain ${CMAKE_CURRENT_SOURCE_DIR}/core OUTPUT_VARIABLE PEARY_CORE_STATUS)
    IF(PEARY_CORE_STATUS STREQUAL "")
      MESSAGE("-- Git: corelib directory is clean.")
    ELSE(PEARY_CORE_STATUS STREQUAL "")
      MESSAGE("-- Git: corelib directory is dirty:\n ${PEARY_CORE_STATUS}.")
    ENDIF(PEARY_CORE_STATUS STREQUAL "")
  ELSE(GIT_FOUND)
    SET(PEARY_LIB_VERSION ${PEARY_VERSION})
    MESSAGE("-- Git repository present, but could not find git executable.")
  ENDIF(GIT_FOUND)
ELSE(NOT SOURCE_PACKAGE)
  # If we don't have git we take the hard-set version.
  SET(PEARY_LIB_VERSION ${PEARY_VERSION})
  MESSAGE("-- Source tarball build - no repository present.")
ENDIF(NOT SOURCE_PACKAGE)
MESSAGE("-- Determined pxar corelib API version ${PEARY_LIB_VERSION}.")

CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.h" "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")


###################################
# Load cpp format and check tools #
###################################

# Set the source files to clang-format
FILE(GLOB_RECURSE
     CHECK_CXX_SOURCE_FILES
     devices/*.[ch]pp devices/*.[ht]cc exec/*.[ch]pp exec/*.[ht]cc peary/*.[ch]pp peary/*.[ht]cc
     )
INCLUDE("cmake/clang-cpp-checks.cmake")


##################################
# Define build targets for peary #
##################################

# Include directories
INCLUDE_DIRECTORIES(peary/utils
  peary/device
  peary/carboard
  peary/interfaces)

# Always build main peary library;
ADD_SUBDIRECTORY(peary)

# Include optional device support
ADD_SUBDIRECTORY(devices)

# Add targets for Doxygen code reference and LaTeX User manual
ADD_SUBDIRECTORY(doc)


################################################
# Define build targets for sample applications #
################################################

# Build flag for the Timepix3 telescope DAQ server
OPTION(BUILD_server "Build server for receiving commands from Run Control?" OFF)
IF(BUILD_server)
  # add target
  ADD_EXECUTABLE(pearysrv "exec/pearysrv.cpp")
  ADD_EXECUTABLE(dummyclient "exec/dummyclient.cpp")
  ADD_EXECUTABLE(dummysrv "exec/dummysrv.cpp")
  TARGET_LINK_LIBRARIES(pearysrv ${PROJECT_NAME})
  INSTALL(TARGETS pearysrv dummyclient dummysrv
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
ENDIF(BUILD_server)

ADD_EXECUTABLE(peary_app "exec/sample_application.cpp")
TARGET_LINK_LIBRARIES(peary_app ${PROJECT_NAME})

ADD_SUBDIRECTORY(extern/cpp-readline)
ADD_EXECUTABLE(pearycli "exec/pearycli.cpp" "exec/clicommands.cpp")
TARGET_LINK_LIBRARIES(pearycli ${PROJECT_NAME} ${CPP-READLINE_LIB})

INCLUDE_DIRECTORIES(devices/example)
ADD_EXECUTABLE(peary_direct "exec/direct_device.cpp")
TARGET_LINK_LIBRARIES(peary_direct ${PROJECT_NAME} CaribouExampleDevice )

INSTALL(TARGETS peary_app peary_direct pearycli
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

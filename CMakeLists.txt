########################################################
# CMake file for the peary caribou DAQ
CMAKE_MINIMUM_REQUIRED(VERSION 3.0 FATAL_ERROR)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW) # change linker path search behaviour
  CMAKE_POLICY(SET CMP0048 NEW) # set project version
ENDIF(COMMAND CMAKE_POLICY)
########################################################

# Set default version
SET(PEARY_VERSION "v0.9.2")

# Set default build type
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

# Overwrite with the version from git if found
INCLUDE("cmake/tools.cmake")
GET_VERSION(PEARY_VERSION)

# Print version
MESSAGE(STATUS "Building Peary version ${PEARY_VERSION}.")
MESSAGE(STATUS "Building \"${CMAKE_BUILD_TYPE}\"")

# Gather information about build time:
STRING(TIMESTAMP BUILD_TIME "%Y-%m-%d, %H:%M:%S UTC" UTC)

# Define the project with the simple version
STRING(REGEX MATCH "([0-9.]+)+" SIMPLE_VERSION "${PEARY_VERSION}")
# Set languages to NONE to allow the documentation to be built without CXX compiler:
PROJECT(peary VERSION ${SIMPLE_VERSION} LANGUAGES CXX)
# Access the project name (for install locations) in the source
ADD_DEFINITIONS(-DPEARY_PROJECT_NAME="${CMAKE_PROJECT_NAME}" -DPEARY_PROJECT_VERSION="${PEARY_VERSION}" -DPEARY_BUILD_TIME="${BUILD_TIME}")

# Include a generated configuration file
# FIXME: this should be combined with the ADD_DEFINITIONS
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.h" "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

###############################
# Setup the build environment #
###############################

# lock dir option must be available globally
SET(PEARY_LOCK_DIR "/var/lock" CACHE PATH "Path to lockfile directory")

# Include peary cmake functions
INCLUDE("cmake/peary.cmake")

# Additional packages to be searched for by cmake
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Configure the installation prefix to allow system-wide installation
SET(INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" CACHE PATH "Prefix prepended to install directories")
SET(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}" CACHE INTERNAL "Prefix prepended to install directories" FORCE)

# Set up the RPATH so executables find the libraries even when installed in non-default location
SET(CMAKE_MACOSX_RPATH 1)
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# Add the automatically determined parts of the RPATH which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")


#################################
# Define build flags for peary  #
#################################

ADD_DEFINITIONS(-std=c++14)
INCLUDE(cmake/Platform.cmake)

SET(CMAKE_THREAD_PREFER_PTHREAD TRUE)
FIND_PACKAGE(Threads REQUIRED)

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

# Always build main peary library;
INCLUDE_DIRECTORIES(peary)
ADD_SUBDIRECTORY(peary)
SET(PEARY_LIBRARIES ${PEARY_LIBRARIES} peary)

# Include individual device libraries:
INCLUDE_DIRECTORIES(devices)
ADD_SUBDIRECTORY(devices)

# Add targets for Doxygen code reference and LaTeX User manual
ADD_SUBDIRECTORY(doc)

################################################
# Define build targets for sample applications #
################################################

ADD_SUBDIRECTORY(exec)
ADD_SUBDIRECTORY(etc) # pearyd service files

###################
# Install headers #
###################
OPTION(INSTALL_HEADERS "Install header files?" OFF)
IF(INSTALL_HEADERS)
  INSTALL(DIRECTORY peary DESTINATION include/peary )
  INSTALL(DIRECTORY devices DESTINATION include/peary )
ENDIF()

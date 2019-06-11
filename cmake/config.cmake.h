#pragma once

// CMake uses config.cmake.h to generate config.h within the build folder.
#ifndef PEARY_CONFIG_H
#define PEARY_CONFIG_H

#define PACKAGE_NAME "@CMAKE_PROJECT_NAME@"
#define PACKAGE_VERSION "@PEARY_VERSION@"
#define PACKAGE_STRING PACKAGE_NAME " " PACKAGE_VERSION
#define PACKAGE_FIRMWARE "v@PEARY_FW_VERSION@"
#endif

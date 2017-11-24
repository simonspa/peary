# Peary Caribou DAQ Software

This is the user manual for the Peary Caribou software, a DAQ framework for the Caribou DAQ system. The source code is hosted on the CERN GitLab instance in the [peary repository](https://gitlab.cern.ch/Caribou/peary). All issues and questions concerning this software should be reported on the projects [issue tracker](https://gitlab.cern.ch/Caribou/peary/issues), issues concerning the [Caribou Linux distribution](https://gitlab.cern.ch/Caribou/meta-caribou) or the [Peary firmware](https://gitlab.cern.ch/Caribou/peary-firmware) should be reported to the corresponding projects.

The software name derives from the smallest of the North American caribou subspecies, found in the high Arctic islands of Canada ([Wikipedia](https://en.wikipedia.org/wiki/Peary_caribou)).

## Overview

This user manual aims at providing an overview of the software framework and facilitating the implementation of new devices.

## Compilation & Installation

When using the Caribou Linux distribution on the Caribou DAQ system, the latest version or `peary` is always pre-installed.

To compile the project on a development machine, the following steps have to be peformed:

```
# clone the repository:
$ git clone https://gitlab.cern.ch/Caribou/peary.git
$ cd peary
$ mkdir build && cd build
# Run CMake to prepare the Makefiles:
$ cmake ..
# Compile the project and install to the default location (source directory):
$ make install
```

### Compiling on machines without I2C and SPI

All interfaces can be switched to emulated mode to avoid having to install the packages for I2C and SPI support. This can be done by passing the corresponding flags to the CMake executable, e.g. for the I2C interface:

```
$ cmake -DINTERFACE_I2C=OFF ..
```

A list of available interfaces can be found in the section [Interfaces](framework.md#hardware-interfaces).

## Compiling only certain devices

The compilation of any supported device can be turned on or off since some devices come with special dependencies which not every user might be able or willing to satisfy. Some devices are *not built by default* and need to be explicitly activated using the appropriate CMake option:

```
$ cmake -DBUILD_devicename=ON/OFF ..
```

The device name is derived from the directory of the device's source code as described in Section [Devices](devices.md).

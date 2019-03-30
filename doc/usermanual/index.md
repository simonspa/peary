# Peary Caribou DAQ Software

This is the user manual for the Peary Caribou software, a DAQ framework for the Caribou DAQ system.
It aims to provide an overview of the software framework and facilitating the implementation of new devices.

## Introduction

The Peary DAQ software framework provides hardware abstraction for periphery components such as voltage regulators and simplifies direct detector configuration and data acquisition through a common interface.

A command line tool as well as a producer for EUDAQ2.0 integration is provided.

The software name derives from the smallest of the North American caribou subspecies, found in the high Arctic islands of Canada ([Wikipedia](https://en.wikipedia.org/wiki/Peary_caribou)).

### Source Code, Support and Reporting Issues
The source code is hosted on the CERN GitLab instance in the [peary repository](https://gitlab.cern.ch/Caribou/peary).
As for most of the software used within the high-energy particle physics community, only limited support on best-effort basis for this software can be offered.
The authors are, however, happy to receive feedback on potential improvements or problems arising.
Reports on issues, questions concerning the software as well as the documentation and suggestions for improvements are very much appreciated.
All issues and questions concerning this software should be reported on the projects [issue tracker](https://gitlab.cern.ch/Caribou/peary/issues), issues concerning the [Caribou Linux distribution](https://gitlab.cern.ch/Caribou/meta-caribou) or the [Peary firmware](https://gitlab.cern.ch/Caribou/peary-firmware) should be reported to the corresponding projects.

### Contributing Code
Peary is a community project that benefits from active participation in the development and code contributions from users.
Users and prospective developers are encouraged to discuss their needs either via the issue tracker of the repository to receive ideas and guidance on how to implement a specific feature.
Getting in touch with other developers early in the development cycle avoids spending time on features which already exist or are currently under development by other users.

## Installation

This section provides details and instructions on how to build and install Peary.
An overview of possible build configurations is given.
Depending on the requirements on supported hardware as well as the host system, different dependencies need to be installed and a set of options are available to customize the installation.
This chapter contains details on the standard installation process and information about custom build configurations.

> When using the Caribou Linux distribution on the Caribou DAQ system, the latest version or `peary` is always pre-installed.


### Downloading the source code
The latest version of Peary can be downloaded from the CERN Gitlab [repository](https://gitlab.cern.ch/Caribou/peary).
For production environments it is recommended to only download and use tagged software versions, as many of the available git branches are considered development versions and might exhibit unexpected behavior.

For developers, it is recommended to always use the latest available version from the git `master` branch.
The software repository can be cloned as follows:

```bash
$ git clone https://gitlab.cern.ch/Caribou/peary.git
$ cd peary
```

### Configuration via CMake

Peary uses the CMake build system to configure, build and install the core framework as well as all device libraries.
An out-of-source build is recommended: this means CMake should not be directly executed in the source folder.
Instead, a `build` folder should be created, from which CMake should be run.
For a standard build without any additional flags this implies executing:

```bash
$ mkdir build
$ cd build
$ cmake ..
```

CMake can be run with several extra arguments to change the type of installation.
These options can be set with `-Doption=VALUE` (see the end of this section for an example).
Currently the following options are supported:

* `INSTALL_PREFIX`: The directory to use as a prefix for installing the binaries, libraries and data. Defaults to the source directory (where the folders `bin/` and `lib/` are added).
* `INSTALL_HEADERS`: Switch to select whether development headers should be installe din the target installation directory or not. This option is used by the Yocto build system to provide the device interface headers to other programs linking against Peary. For development machines, this is not necessary and defaults to `OFF`.
* `CMAKE_BUILD_TYPE`: Type of build to install, defaults to `RelWithDebInfo` (compiles with optimizations and debug symbols). Other possible options are `Debug` (for compiling with no optimizations, but with debug symbols) and `Release` (for compiling with full optimizations and no debug symbols).
* `BUILD_DeviceName`: If the specific device `DeviceName` should be compiled or not.
Defaults to `ON` for most devices, however some devices with additional dependencies which not every user might be able or willing to satisfy. These devices are *not built by default* and need to be explicitly activated. The device name is derived from the directory of the device's source code as described in Section [Devices](devices.md).
* `BUILD_ALL_DEVICES`: Build all included devices, defaulting to `OFF`. This overwrites any selection using the parameters described above.
* `INTERFACE_interface`: Individual hardware interfaces can be switched to emulation mode for development purposes. This avoids having to install the dependencies e.g. for I2C and SPI support on the development system. By default, all interfaces are switched `ON`. A list of available interfaces can be found in the section [Interfaces](framework.md#hardware-interfaces).
* `BUILD_server`/`BUILD_ATPserver`: Build servers which listen on TCP ports and forward commands to the device manager. Default to `OFF`.

An example of a custom debug build, without the `CLICpix2` device and with installation to a custom directory is shown below:

```bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_INSTALL_PREFIX=../install/ \
        -DCMAKE_BUILD_TYPE=DEBUG \
        -DBUILD_CLICpix2=OFF ..
```

### Compilation and installation

Compiling the framework is now a single command in the build folder created earlier (replacing `<number_of_cores>`} with the number of cores to use for compilation):
```bash
$ make -j<number_of_cores>
```
The compiled (non-installed) executables can be found at `src/exec/` in the \dir{build} folder.
To install the library to the selected installation location (defaulting to the source directory of the repository) requires the following command:
```bash
$ make install
```

The binaries are now available in the `bin/` folder of the installation directory.

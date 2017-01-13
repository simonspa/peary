# Peary Caribou

DAQ framework for the Carioub DAQ System

<https://en.wikipedia.org/wiki/Peary_caribou>

## User manual

The user manual for the Peary DAQ software can be found [here](doc/usermanual/manual.md)

## How to compile / install?

The Peary library is compiled by simply running

```
$ mkdir build && cd build/
$ cmake ..
$ make
$ make install
```

In order to compile dive libraries for specific detectors, the building of the library has to be activated through CMake:

```
$ cmake -DBUILD_<device> ..
$ make
$ make install
```

where `<device>` is the device considered. For convenience, there is an example provided which can be activated with `-DBUILD_example`.


## Design and Implementation Guidelines

### Repository & Continuous Integration

* The software uses git for version control and is currently hosted at CERN gitlab. The repository contains a license, readme and clang format file which should be obeyed.

* A continuous integration is set up using the CERN Gitlab CI services. The software package is compiled after every push to the repository. In the future, this could be extended with additional unit tests for functionality.

  There are two compilation targets set up:

  * `peary` uses the latest Ubuntu LTS image from Dockerhub, installs the build dependencies and builds the software package
  * `peary-zynq` uses Gitlab CI runners with the `zynq` tag attached which will execute the job on one of our configured Gitlab CI instances running on the ZC706 Zynq boards with Poky Linux. This job ensures, the project builds fine under the target architecture `armv7l` and on the Poky distribution used.
 

### General Considerations

* The first effort should go into designing the caribouDevice API with  as much detail as possible. This allows to work independently on the  user-code and the library without the need for reworking code in case of internal design changes.

* After the definition of the IP core interfaces for the communication protocols, the HAL elements should be implemented.

* All code written should be well-documented, especially the function definitions in header files should always be accompanied by an explanation of what this function does, what the parameters are and what return is expected. Possible exceptions should be indicated.

### Device Configuration

Configuration is done via simple but flexible text files containing key-value pairs. The value can also be a comma-separated vector of any type.

Configuration files are provided by the user code via the constructor of the device classes. Configuration can then be accessed e.g. via

```
uint64_t myvar = _config.Get("key_of_myvar",1234);
```

where the second parameter is a default value used in case the key is not present in the provided file. In device implementations, all default configuration parameters should be collected in a header file as demonstrated for [the example device](devices/example/example_defaults.h). This avoids scattering hard-coded values across the code while still always providing sane defaults whenever the device is operated.
# The Framework

This section describes the basic structore of the PEary Caribou DAQ framework.

## Structure of the Source Tree

The code of the framework is structured in the following folders:

* The `peary/` directory contains the central components of the framework. This comprises both the device library and hardware abstraction layer as well as additional utilities for logging. The individual sub-folders and their content will be described in more detail in this chapter.
* The `devices/` directory hosts all implemented Caribou devices. Each device is placed in a separate folder which contains the necessary code and CMake commands to build a device library which can be loaded at run-time.
* The source code for central executables of the framework, such as the command line interface can be found in the `exec/` directory.
* The folder `extern/` contains additional code from third parties used in this repository such as C++ bindings for the `readline` library used for the command line interface.
* The `doc/` directory contains the configuration files for generating the Doxygen source reference as well as this user manual.
* Additional macros used for the generation of the build files can be found in the `cmake/` folder.

## Caribou Device API

The central device API for every device is defined in `peary/device/Device.hpp`. The API calls exposed there can be called from any exectuable built for Peary. In general, the framework distinguished between two different types of devices:

### Caribou Devices

Caribou devices are devices which access components residing on the CaR board, such as voltage regulators or ADCs.
These devices have full access to the hardware abstraction layer of the CaR board and can register periphery components such as voltage supplies as described below.
When instantiating, the device requests a specific interface as their slow-control interface for e.g. accessing device registers.
The CaribouDevice class of Peary then provides additional functionality such as a register dictionary and convenient member functions to set and retrieve register values to and from the device.

#### Registering periphery components

The Caribou device class provides a dictionary for periphery components found on the CaR board.
Devices inheriting from this class can register their required components via the dictionary's `add()` member function by providing a human-readable name and the CaR schematics name of the respective component, e.g.

```cpp
_periphery.add("vddd", PWR_OUT_1);
```

After this, the voltage regulator available at `PWR_OUT_1` of the CaR board is known under the name `vddd` and can be controlled via the device API e.g. as

```cpp
switchOn("vddd");
setVoltage("vddd", 1.2);
```

Which API calls are available for which periphery component depends on its functionality. A full list of available components from the CaR board can be obtained from the file `peary/carboard/Carboard.hpp` in the repository.
The names and pin-outs of the respective components can be found in the CaR board schematics.


#### Device registers

In order to simplify programming of devices, Peary implements a register dictionary which allows devices to list their registers with name, address, width.
More information

### Auxiliary Devices

Auxiliary devices do not use any of the infrastructure provided on the CaR board. They may only register a single interface and communicate over this. The communication protocol has to be implemented by the device itself and no additional functionality is provided within the Peary framework.

An example for such a device could be an oscilloscope connected to the same network as the Caribou system.
By implementing the communication protocol with the oscilloscope as auxiliary device, commands can be sent to the device directly from Peary, e.g. from the command line.
This simplifies e.g. scans of device parameters via Peary scripts.

### The Command Dispatcher

## Device Manager

While devices can be instantiated and used directly by user code, only one single device can be used in this way at the time.
If several devices, such as an active sensor with programming interface and the main readout chip, should be operated in parallel, the device manager is required in order to manage the shared resources on the CaR board.

The device manager keeps track of all devices currently online and is also capable of constructing and destructing device objects.

## Locking

When a device or device manager is running, a lock file is created on the system to prevent further instances from running and from overwriting e.g. voltage levels.
By default, these lock files are placed in the system lock directory at `/var/lock`, but the destination can be changed at build time using the CMake parameter `PEARY_LOCK_DIR`.

Two lock files distinguish whether an individual device is currently running and blocking the execution of further devices (`pearydev.lock`) or whether a device manager is already running (`pearydevmgr.lock`).

## Car Board Hardware Abstraction Layer

## Hardware Interfaces

### Interface Manager

### Interface Emulators

#### I2C

#### SPI

##### Non-Standard SPI Interface for CLICpix2

### IPSocket

### Loopback

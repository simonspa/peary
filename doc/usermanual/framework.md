# The Framework

This section describes the basic structure of the Peary Caribou DAQ framework.

## Structure of the Source Tree

The code of the framework is structured in the following folders:

* The `peary/` directory contains the central components of the framework. This comprises both the device library and hardware abstraction layer as well as additional utilities for logging. The individual sub-folders and their content will be described in more detail in this chapter.
* The `devices/` directory hosts all implemented Caribou devices. Each device is placed in a separate folder which contains the necessary code and CMake commands to build a device library which can be loaded at run-time.
* The source code for central executables of the framework, such as the command line interface can be found in the `exec/` directory.
* The folder `extern/` contains additional code from third parties used in this repository such as C++ bindings for the `readline` library used for the command line interface.
* The `doc/` directory contains the configuration files for generating the Doxygen source reference as well as this user manual.
* Additional macros used for the generation of the build files can be found in the `cmake/` folder.

## Caribou Device API

The central device API for every device is defined in `peary/device/Device.hpp`. The API calls exposed there can be called from any exectuable built for Peary. In general, the framework distinguished between two different types of devices, Caribou devices and Auxiliary devices, which will be detailed in the following sections.

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
More information on the possible configurations of individual registers in the dictionary can be found in the respective paragraph of the [Utilities chapter](utils.md#device-registers).

##### Adding registers to the dictionary

Registers can be added to the dictionary via its `add()` member function, e.g.

```cpp
register_t<> my_register(0x0A)
_registers.add("threshold", my_register);
```

However, it is customary to define all available registers together in a C++ initializer list and adding them to the dictionary together, i.e.

```cpp
#define MY_REGISTERS	               \
{									   \
    {"reset",     register_t<>(0x02)}, \
    {"threshold", register_t<>(0x04)}  \
}

_registers.add(MY_REGISTERS);
```
in the constructor of the device.

The device class then provides a set of functions to the device to easily set and retrieve register values:

* `setRegister(name, value)` looks up the respective register address from the dictionary, ensures that the register is marked as writable, shifts the value to the correct position if a mask is specified, and writes to the configured address via the standard device interface. If the given register only occupies part of a larger register (i.e. only a single bit from a an 8-bit register), the other bits are preserved by first reading the register, altering the respective bit, and writing the full new value to the device register.
* `getRegister(name)` looks up the register address from the dictionary, ensures that the respective register is readable, retrieves the value from the device and possibly shifts it back from its internal position. This way, a single bit set in a register with an offset of e.g. 4 bits can still be set and read as 0 and 1 without the necessity to manually shift it to the correct position.
* `getRegisters()` goes through all registers known to the device, for each register it checks whether it is readable, retrieves the value from the device amd returns a vector with the respective names and values to the caller.
* `listRegisters()` simply returns a vector of all registers known to this device. This can be used e.g. to automate things in command line scripts.

##### Special registers

Some registers behave in a special way. In order to accommodate for them, they can be marked as "special registers".
Devices can overload the `setSepcialRegister` and `getSpecialRegister` functions in order to treat them according to their needs.
If such a register is encountered by the `setRegister` or `getRegister` functions described above, the respective overloaded device member is called with the name and the value provided through the interface.

This functionality allows to e.g. treat two 8-bit registers as a linear 16-bit register without the need to alter the device API - all calls are correctly routed through the respective common register functions, while the device itself can internally take the value and split it for the two separate registers manually:

```cpp
void MyDevice::setSpecialRegister(std::string name, uint32_t value) {
  if(name == "threshold") {
    // 9-bit register, just linearly add:
    uint8_t lsb = value & 0x00FF;
    uint8_t msb = (value >> 8) & 0x01;
    // Set the two values:
    this->setRegister(name + "_msb", msb);
    this->setRegister(name + "_lsb", lsb);
  }
}
```

#### Access to FPGA Memory

Registers in the FPGA firmware are accessed via the AXI interface provided by the system.
In order to ease the access and to avoid having to deal with raw pointer arithmetics, this access is implemented as an independent hardware interface for memory access. Similar to the periphery components, devices can register memory pages in a memory access dictionary in order to read and write values from and to the FPGA registers.

```cpp
#define MY_MEMORY                     \
{                                     \
    {"readout",                       \
     memory_map(READOUT_BASE_ADDRESS, \
                READOUT_OFFSET,       \
                READOUT_MAP_SIZE,     \
                READOUT_MAP_MASK,     \
                PROT_READ)}           \
}

// Add memory pages to the dictionary:
_memory.add(MY_MEMORY);
```

A detailed description of the structure of the `memory_map` data type can be found in the [Utilities section](utils.md#memory-maps). After registering the memory pages, they can be accessed using member functions of the Caribou device class as follows:

```cpp
// Retrieving a value from the memory page named "frame_size"
unsigned int frameSize = getMemory("frame_size");

// Writing into a memory page called "control":
setMemory("control", 0x12);

// Writing into a memory page called "events" at different offsets:
for(size_t reg = 0; reg < patterns.size(); reg++) {
  setMemory("events", (reg << 2), patterns.at(reg));
}
```

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

## CaR Board Hardware Abstraction Layer

## Hardware Interfaces

### Interface Manager

### Interface Emulators

### Interfaces

#### Memory

#### I2C

#### SPI

##### Non-Standard SPI Interface for CLICpix2

### IPSocket

### Loopback

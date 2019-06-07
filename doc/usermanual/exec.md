# Peary Caribou Executables

## The `pearycli` Executable

The `pearycli` executable functions as interactive command line interface between the user and the framework.
It provides a shell to the user through which they can add or remove devices and interact with them through the device API.

The executable handles the following arguments:

* `-c <file>`: Specifies the configuration file to be used, relative to the current directory. If this argument is omitted, no configuration file is loaded.
* `-l <file>`: Specify an additional location to forward log output to, besides standard output printed on screen.
* `-v <level>`: Sets the global log verbosity level, overwriting the value specified in the configuration file described in the [Utilities section](utils.md#logging).
Possible values are `FATAL`, `STATUS`, `ERROR`, `WARNING`, `INFO`, `DEBUG` and `TRACE`, where all options are case-insensitive.
* `--version`: Prints the version and build time of the executable and terminates the program.

All other arguments are interpreted as devices to be instantiated.

### Termination Signals
Signals can be send using keyboard shortcuts to terminate the program, either gracefully or with force.
The executable understand the following signals:

* `CTRL+D`: The equivalent to typing `exit` in the command line. This properly terminates all running processes, powers down the devices, removes the device manager and exits.
* SIGINT (`CTRL+C`): Request a graceful shutdown of the program. This means that all devices are removed from the device manager and therefore powered down properly. After removing all devices, the device manager is terminated.
* SIGTERM: Same as SIGINT, request a graceful shutdown of the program. This signal is emitted e.g. by the `kill` command.
* SIGQUIT (`CTRL+\`): Forcefully terminates the program. It is not recommended to use this signal since it leaves the devices in an undefined state, most likely still powered. This signal should only be used when graceful termination is for any reason not possible.

## The EUDAQ2 Producer



## Example Standalone Application

## Other User Interfaces

### Peary Client and Server

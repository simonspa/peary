# Peary client python library

A pure python library to interact with the Caribou/Peary data
acquisition library. The library connects to a running `pearyd`
instance, forwards calls to the underlying devices, and does some output
processing. In addition, a shell-like client to interact with the peary
system based on the library is provided that can be started as

    pearyc <host>

## Installation

The software requires Python >= 3.4 and should be installed using pip. It
is recommended to install it privately either into a
[Python virtual environment][venv] or into a user-specific
directory. The following command installs it into the per-user location

    pip install --user <path/to/this/directory>

There is also the possibility to make the software available
but allow local modifications to the propagated immediately
directly (without having the reinstall it again) using

    pip install --user --editable <path/to/this/directory>

## Example

After starting a pearyd instance locally (e.g. compiled with interface
emulation) using

    pearyd -v DEBUG

the following python code connects to the local instance creates an
`ExampleCaribou` device for testing and calls some device commands

```python
from peary import PearyClient

with PearyClient(host='localhost') as client:

    device = client.add_device('ExampleCaribou')
    print(client.list_devices())

    # common device functionality
    print(device.list_registers())
    device.set_register('vdd', 12)
    print(device.get_register('vdd'))

    # ExampleCaribou-specific commands
    print(device.frobicate(123))
```

A more detailed example can be found in `peary/example.py`.


[venv]: https://docs.python.org/3/library/venv.html

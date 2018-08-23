#!/usr/bin/env python
# coding: utf-8
"""
Example script to show the usage of the peary client library
"""

from __future__ import print_function

from . import PearyClient

def main(host='localhost'):
    with PearyClient(host=host) as pc:
        pc.keep_alive()
        # ensure device allows multiple connections to the same
        # pearyd instance w/o adding a device every time.
        dev1 = pc.ensure_device('ExampleCaribou')
        print('devices:')
        for device in pc.list_devices():
            print('  {}'.format(device))
        print(dev1.frobicate(123))
        print(dev1.unfrobicate(543))
        dev1.set_register('reg0', 12)
        print('registers:')
        for reg in dev1.list_registers():
            print('  {}: {:#x}'.format(reg, dev1.get_register(reg)))
        print(dev1.get_voltage('vdd'))
        print(dev1.get_current('vdd'))
        pc.keep_alive()

if __name__ == '__main__':
    import sys
    main(*sys.argv[1:])

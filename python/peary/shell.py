# coding: utf-8
"""
Shell-like command line interface to interact with a pearyd instance.
"""

from __future__ import print_function

import argparse
import cmd
import functools

from . import PearyClient

# cmd.Cmd natively passes all arguments for a command as a single string
# and does not parse them automatically. This decorator parses the argument
# string and tries to convert each argument to an appropriate type.

# TODO inspect number of required arguments
# TODO inspect argument names to automatically create the help message

def parse_arguments(f):
    @functools.wraps(f)
    def f_with_parsing(self, arg):
        args = [ensure_number(_) for _ in arg.split()]
        return f(self, *args)
    return f_with_parsing
def ensure_number(x):
    """
    Convert the argument to an appropriate number type if possible.
    """
    try:
        return int(x)
    except ValueError:
        pass
    try:
        return float(x)
    except ValueError:
        pass
    # probably not a number; retain as-is
    return x

class PearyShell(cmd.Cmd):
    intro = '\n'.join([
        'Welcome to peary',
        '',
        'Type help or ? to list commands',
        'Type quit or CTRL-D to exit',
        '',
    ])
    prompt = '(peary) '

    def __init__(self, client, **kw):
        super(PearyShell, self).__init__(**kw)
        self.client = client
        self.intro += '\nConnected to {}\n'.format(client.peername)

    def default(self, line):
        if line == 'EOF':
            return True
        return super(PearyCli, self).default(line)

    @parse_arguments
    def do_quit(self):
        """Close the connection and exit the client."""
        return True

    @parse_arguments
    def do_list_devices(self):
        """List available devices."""
        for device in self.client.list_devices():
            print('{:d}: {}'.format(device.index, device))
    @parse_arguments
    def do_add_device(self, name):
        """Add new device. Arguments: <name>"""
        print(self.client.add_device(name))
    @parse_arguments
    def do_ensure_device(self, name):
        """Ensure one device of the given type exists. Arguments: <name>"""
        print(self.client.ensure_device(name))

def main():
    p = argparse.ArgumentParser(description='Peary client shell')
    p.add_argument('host', nargs='?', default='localhost',
                   help='host address (default=%(default)s)')
    args = p.parse_args()

    with PearyClient(host=args.host) as client:
        sh = PearyShell(client)
        sh.cmdloop()

if __name__ == '__main__':
    main()

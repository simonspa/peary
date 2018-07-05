#!/usr/bin/env python
# encoding = utf-8

from __future__ import print_function, unicode_literals

import functools
import random
import socket
import struct

MSG_HEADER = struct.Struct('!LL')

class SequenceMissmatch(Exception):
    def __init__(self, request, reply):
        msg = ('Sequence number missmatch'
               'request={:} reply{:d}'.format(request, reply))
        super(SequenceMissmatchError, self).__init__(msg)
class InvalidReply(Exception):
    pass
class FailedCommand(Exception):
    def __init__(self, cmd, reason):
        super(FailedCommand, self).__init__(
            'Command \'{}\' failed with error \'{}\''.format(cmd, reason))

class PearyClient(object):
    def __init__(self, host, port=12345):
        super(PearyClient, self).__init__()
        self.host = host
        self.port = port
        self._socket = socket.create_connection((self.host, self.port))
        self._sequence = random.Random()
        self._sequence.seed()

    def _command(self, cmd):
        req_seq = self._sequence.randint(0, 2**32 - 1)
        req_payload = cmd.encode('utf-8')
        req_header = MSG_HEADER.pack(req_seq, len(req_payload))
        self._socket.send(req_header)
        self._socket.send(req_payload)
        rep_header = self._socket.recv(8)
        rep_seq, rep_len = MSG_HEADER.unpack(rep_header)
        if rep_seq != req_seq:
            raise SequenceMissmatch(req_seq, rep_seq)
        rep_payload = self._socket.recv(rep_len)
        if rep_payload.startswith(b'ok'):
            return rep_payload[3:]
        elif rep_payload.startswith(b'error'):
            raise FailedCommand(cmd, rep_payload[6:].decode('ascii'))
        else:
            raise InvalidReply(rep_payload)

    def add_device(self, name):
        index = self._command('add_device {}'.format(name))
        index = int(index)
        return Device(self, index)
    def get_device(self, index):
        return Device(self, index)

class Device(object):
    def __init__(self, client, index):
        super(Device, self).__init__()
        self._client = client
        self._prefix = 'device {:d}'.format(index)
    def _call(self, name, *args):
        parts = [self._prefix, name]
        parts.extend(str(_) for _ in args)
        return self._client._command(' '.join(parts))
    def __getattr__(self, name):
        func = functools.partial(self._call, name)
        self.__dict__[name] = func
        return func

if __name__ == '__main__':
    pc = PearyClient(host='localhost')
    print(pc._command('hello'))
    print(pc._command('list_devices'))
    # print(pc._command('device 1 get_name'))
    dev0 = pc.get_device(0)
    print(dev0.get_name())

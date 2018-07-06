#!/usr/bin/env python
# encoding = utf-8

from __future__ import print_function, unicode_literals

import functools
import random
import socket
import struct

PROTOCOL_VERSION = b'1'

# total length, sequence number
REQUEST_HEADER = struct.Struct('!LL')
# total length, sequence number, status code
REPLY_HEADER = struct.Struct('!LLB')

class UnsupportedProtocol(Exception):
    pass
class InvalidReply(Exception):
    pass
class FailedCommand(Exception):
    def __init__(self, cmd, code, reason):
        msg = 'Command \'{}\' failed with error {:d} \'{}\''
        super(FailedCommand, self).__init__(msg.format(cmd, code, reason))

class PearyClient(object):
    """
    Connect to a pearyd instance running somewhere else.
    """
    def __init__(self, host, port=12345):
        super(PearyClient, self).__init__()
        self.host = host
        self.port = port
        self._sequence = random.Random()
        self._sequence.seed()
        self._socket = socket.create_connection((self.host, self.port))
        # check connection and protocol
        version = self._request('hello')
        if version != PROTOCOL_VERSION:
            raise UnsupportedProtocol(version)

    def _request(self, cmd):
        # randint limits are inclusive
        req_seq = self._sequence.randint(0, 2**32 - 1)
        req_payload = cmd.encode('utf-8')
        req_header = REQUEST_HEADER.pack(4 + len(req_payload), req_seq)
        self._socket.send(req_header)
        self._socket.send(req_payload)
        rep_header = self._socket.recv(9)
        rep_len, rep_seq, rep_err = REPLY_HEADER.unpack(rep_header)
        if rep_len < 5:
            raise InvalidReply('Length is too small')
        if rep_seq != req_seq:
            raise InvalidReply('Sequence number missmatch', req_seq, rep_seq)
        rep_payload = self._socket.recv(rep_len - 5)
        if rep_err:
            raise FailedCommand(cmd, rep_err, rep_payload.decode('utf-8'))
        return rep_payload

    def keep_alive(self):
        self._request('')

    def list_devices(self):
        indices = self._request('list_devices')
        indices = [int(_) for _ in indices.split()]
        return [Device(_) for _ in indices]
    def add_device(self, name):
        index = self._request('add_device {}'.format(name))
        index = int(index)
        return Device(self, index)

class Device(object):
    """
    A Peary device.

    This acts as a proxy object that just forwards all function calls via the
    network.
    """
    def __init__(self, client, index):
        super(Device, self).__init__()
        self._client = client
        self._prefix = 'device.{:d}.'.format(index)
    def _call(self, name, *args):
        parts = [self._prefix, name]
        parts.extend(str(_) for _ in args)
        return self._client._request(' '.join(parts))
    def __getattr__(self, name):
        func = functools.partial(self._call, name)
        self.__dict__[name] = func
        return func

if __name__ == '__main__':
    pc = PearyClient(host='localhost')
    pc.keep_alive()
    print(pc.list_devices())
    dev1 = pc.add_device('Example')
    print(dev1)

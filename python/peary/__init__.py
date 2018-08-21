# coding: utf-8

import functools
import random
import socket
import struct

# supported protocol version
PROTOCOL_VERSION = b'1'
# message length
LENGTH = struct.Struct('!L')
# sequence number, status code
HEADER = struct.Struct('!HH')

class UnsupportedProtocol(Exception):
    pass
class InvalidReply(Exception):
    pass
class Failure(Exception):
    def __init__(self, cmd, code, reason):
        self.cmd = cmd
        self.code = code
        self.readon = reason
        msg = 'Command \'{}\' failed with code {:d} \'{}\''
        super(Failure, self).__init__(msg.format(cmd, code, reason))

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
        version = self._request('protocol_version')
        if version != PROTOCOL_VERSION:
            raise UnsupportedProtocol(version)
    def __del__(self):
        self.close()
    # support with statements
    def __enter__(self):
        return self
    def __exit__(self, *unused):
        self.close()

    def close(self):
        """
        Close the connection.
        """
        # is there a better way to allow double-close?
        if self._socket.fileno() != -1:
            # hard shutdown, no more sending or receiving
            self._socket.shutdown(socket.SHUT_RDWR)
            self._socket.close()
    @property
    def peername(self):
        return self._socket.getpeername()

    def _request(self, cmd):
        # randint limits are inclusive
        req_seq = self._sequence.randint(0, 2**16 - 1)
        # constructive message content and header
        req_payload = cmd.encode('utf-8')
        req_fixed = bytearray(8)
        # message length for framing: header + payload
        LENGTH.pack_into(req_fixed, 0, 4 + len(req_payload))
        HEADER.pack_into(req_fixed, 4, req_seq, 0)
        self._socket.send(req_fixed)
        self._socket.send(req_payload)
        rep_length, = LENGTH.unpack(self._socket.recv(4))
        if rep_length < 4:
            raise InvalidReply('Length too small')
        rep_msg = self._socket.recv(rep_length)
        rep_seq, rep_status = HEADER.unpack(rep_msg[:4])
        rep_payload = rep_msg[4:]
        if rep_status:
            raise Failure(cmd, rep_status, rep_payload.decode('utf-8'))
        if rep_seq != req_seq:
            raise InvalidReply('Sequence number missmatch', req_seq, rep_seq)
        return rep_payload

    def keep_alive(self):
        self._request('')

    def list_devices(self):
        indices = self._request('list_devices')
        indices = [int(_) for _ in indices.split()]
        return [Device(self, _) for _ in indices]
    def add_device(self, name):
        index = self._request('add_device {}'.format(name))
        index = int(index)
        return Device(self, index)
    def ensure_device(self, name):
        """
        Ensure at least one device with the given name exists and return it.

        If there are multiple devices with the same name, the first one
        is returned.
        """
        devices = self.list_devices()
        devices = filter(lambda _: _.name == name, devices)
        devices = sorted(devices, key=lambda _: _.index)
        if devices:
            return devices[0]
        else:
            return self.add_device(name)

class Device(object):
    """
    A Peary device.

    This object acts as a proxy that forwards all function calls to the
    device specified by the device index using the client object.
    """
    def __init__(self, client, index):
        super(Device, self).__init__()
        self._client = client
        self.index = index
        # internal name is actualy <name>Device, but we only use <name>
        # to generate it. remove the suffix for consistency
        self.name = self._call('name').decode('utf-8')
        if self.name.endswith('Device'):
            self.name = self.name[:-6]
    def __repr__(self):
        return 'Device(index={:d}, name={})'.format(self.index, self.name)
    def _call(self, cmd, *args):
        parts = ['device.{}'.format(cmd), str(self.index)]
        parts.extend(str(_) for _ in args)
        return self._client._request(' '.join(parts))
    # unknown attributes are interpreted as dynamic functions
    # and are forwarded to the pearyd instance
    def __getattr__(self, name):
        func = functools.partial(self._call, name)
        self.__dict__[name] = func
        return func
    # fixed functions with additional return value decoding
    def list_registers(self):
        return self._call('list_registers').decode('utf-8').split()
    def get_register(self, name):
        return int(self._call('get_register', name))
    def get_current(self, name):
        return float(self._call('get_current', name))
    def get_voltage(self, name):
        return float(self._call('get_voltage', name))

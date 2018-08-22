# coding: utf-8

import functools
import socket
import struct

# supported protocol version
PROTOCOL_VERSION = b'1'
# named message status values
STATUS_OK = 0

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

    The peary client supports the context manager protocol and should be
    used in a with statement for automatic connection closing on errors, i.e.

        with PearyClient(host='localhost') as client:
            # do something with the client

    """
    def __init__(self, host, port=12345):
        super(PearyClient, self).__init__()
        self.host = host
        self.port = port
        # Cache of available device objects to avoid recreating them
        self._devices = {}
        self._sequence_number = 0
        self._socket = socket.create_connection((self.host, self.port))
        # check connection and protocol
        version = self._request('protocol_version')
        if version != PROTOCOL_VERSION:
            raise UnsupportedProtocol(version)
    def __del__(self):
        self._close()
    # support with statements
    def __enter__(self):
        return self
    def __exit__(self, *unused):
        self._close()
    def _close(self):
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

    def _request(self, cmd, *args):
        """
        Send a command to the host and return the reply payload.
        """
        # 1. encode request
        # encode command and its arguments into message payload
        req_payload = [cmd,]
        req_payload.extend(str(_) for _ in args)
        req_payload = ' '.join(req_payload).encode('utf-8')
        # encode message header
        self._sequence_number += 1
        req_header = HEADER.pack(self._sequence_number, STATUS_OK)
        # encode message length for framing
        req_length = LENGTH.pack(len(req_header) + len(req_payload))
        # 2. send request
        self._socket.send(req_length)
        self._socket.send(req_header)
        self._socket.send(req_payload)
        # 3. wait for reply and unpack in opposite order
        rep_length, = LENGTH.unpack(self._socket.recv(4))
        if rep_length < 4:
            raise InvalidReply('Length too small')
        rep_msg = self._socket.recv(rep_length)
        rep_seq, rep_status = HEADER.unpack(rep_msg[:4])
        rep_payload = rep_msg[4:]
        if rep_status != STATUS_OK:
            raise Failure(cmd, rep_status, rep_payload.decode('utf-8'))
        if rep_seq != self._sequence_number:
            raise InvalidReply('Sequence number missmatch', req_seq, rep_seq)
        return rep_payload

    def keep_alive(self):
        self._request('')

    def list_devices(self):
        indices = self._request('list_devices')
        indices = [int(_) for _ in indices.split()]
        return [self.get_device(_) for _ in indices]
    def get_device(self, index):
        device = self._devices.get(index)
        if not device:
            device = self._devices.setdefault(index, Device(self, index))
        return device
    def add_device(self, device_type):
        """
        Add a new device of the given type.
        """
        index = self._request('add_device', device_type)
        index = int(index)
        return self.get_device(index)
    def ensure_device(self, device_type):
        """
        Ensure at least one device of the given type exists and return it.

        If there are multiple devices with the same name, the first one
        is returned.
        """
        devices = self.list_devices()
        devices = filter(lambda _: _.device_type == device_type, devices)
        devices = sorted(devices, key=lambda _: _.index)
        if devices:
            return devices[0]
        else:
            return self.add_device(device_type)

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
        self.device_type = self._request('name').decode('utf-8')
        if self.device_type.endswith('Device'):
            self.device_type = self.device_type[:-6]
    def __repr__(self):
        return '{}Device(index={:d})'.format(self.device_type, self.index)
    def _request(self, cmd, *args):
        """
        Send a per-device command to the host and return the reply payload.
        """
        return self._client._request('device.{}'.format(cmd), self.index, *args)

    # fixed device functionality is added explicitely with
    # additional return value decoding where appropriate

    def power_on(self):
        self._request('power_on')
    def power_off(self):
        self._request('power_off')
    def reset(self):
        self._request('reset')
    def configure(self):
        self._request('configure')
    def daq_start(self):
        self._request('daq_start')
    def daq_stop(self):
        self._request('daq_stop')

    def list_registers(self):
        return self._request('list_registers').decode('utf-8').split()
    def get_register(self, name):
        return int(self._request('get_register', name))
    def set_register(self, name, value):
        self._request('set_register', name, value)

    def get_current(self, name):
        return float(self._request('get_current', name))
    def set_current(self, name, value):
        self._request('set_current', name, value)

    def get_voltage(self, name):
        return float(self._request('get_voltage', name))
    def set_voltage(self, name, value):
        self._request('set_voltage', name, value)

    def switch_on(self, name):
        self._request('switch_on', name)
    def switch_off(self, name):
        self._request('switch_off', name)

    # unknown attributes are interpreted as dynamic functions
    # and are forwarded as-is to the pearyd instance
    def __getattr__(self, name):
        func = functools.partial(self._request, name)
        self.__dict__[name] = func
        return func

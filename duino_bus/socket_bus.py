#!/usr/bin/env python3
"""This module implements the SocketBus, which basically implements
   a serial like interface using a socket server.
"""

from fcntl import fcntl, F_GETFL, F_SETFL
import logging
import os
import select
import socket
import time
from typing import Tuple, Union

from duino_bus.bus import IBus
from duino_bus.packet import ErrorCode, Packet

LOGGER = logging.getLogger(__name__)


class SocketBus(IBus):
    """Implements a bus which utilizes TCP/IP sockets."""

    DEFAULT_PORT = '8888'

    def __init__(self):
        super().__init__()
        self.socket = None

    def print_addr_info(self, label: str, info: Tuple) -> None:
        """Prints information from a single info entry returned by getaddrinfo."""
        # The info tuple looks like (family, type, proto, canonname, sockaddr)
        family, _, _, _, sockaddr = info

        family_str = '?'
        if family == socket.AF_INET:
            family_str = '4'
        elif family == socket.AF_INET6:
            family_str = '6'

        LOGGER.info('%s IPv%s [%s]:%d', label, family_str, sockaddr[0], sockaddr[1])

    def make_socket_non_blocking(self, skt: socket) -> ErrorCode:
        """Makes the socket non-blocking."""
        flags = fcntl(skt, F_GETFL) | os.O_NONBLOCK
        try:
            fcntl(skt, F_SETFL, flags)
        except OSError as err:
            LOGGER.error('fcntl failed to make socket non-blocking: %s', err)
            return ErrorCode.OS
        return ErrorCode.NONE

    def connect_to_server(self, host: str, port: str) -> ErrorCode:
        """Tries to connect to the indicated server."""
        addrinfo = socket.getaddrinfo(host, port, family=socket.AF_UNSPEC, type=socket.SOCK_STREAM)
        connected = False
        info = None
        for info in addrinfo:
            self.print_addr_info('Trying', info)
            family, skt_type, _, _, sockaddr = info
            server_skt = socket.socket(family, skt_type)
            server_skt.settimeout(0.5)
            try:
                server_skt.connect(sockaddr)
                connected = True
                break
            except socket.timeout:
                # We didn't connect, try the next entry
                server_skt.close()
                continue
        if not connected:
            LOGGER.error("No IP Address found for connecting")
            return ErrorCode.OS

        # Make the socket non-blocking
        err = self.make_socket_non_blocking(server_skt)
        if err != ErrorCode.NONE:
            return err

        self.print_addr_info('Connected to', info)
        self.socket = server_skt
        return ErrorCode.NONE

    def enable_keepalive(self):
        """Enables keep alive packets so we get notified quicker when the other end goes away."""
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        # Set the amount of idle time before a keep alive is sent
        self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPIDLE, 1)
        # Interval between keep alives
        self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPINTVL, 1)
        # Closes the socket after 3 failed  pings
        self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPCNT, 3)

    def is_data_available(self) -> bool:
        """Returns True if data is available, False otherwise."""
        poll = select.poll()
        poll.register(self.socket, select.POLLIN)
        events = poll.poll(0)
        return len(events) > 0

    def is_space_available(self) -> bool:
        """Returns Trus if space is available to write another byte, False otherwise."""
        poll = select.poll()
        poll.register(self.socket, select.POLLOUT)
        events = poll.poll(0)
        return len(events) > 0

    def read_byte(self) -> Union[int, None]:
        """Reads a byte from the bus. This function is non-blocking.
           Returns None if no character was available to be read, or the character.
        """
        data = self.socket.recv(1)
        if data:
            return data[0]
        return None

    def write_byte(self, byte: int) -> None:
        """Writes a byte to the bus."""
        self.socket.send(byte.to_bytes(1, 'little'))


# pylint: disable=duplicate-code
if __name__ == '__main__':
    # To invoke this use:
    # python -m duino_bus.socket_bus
    logging.basicConfig(level=logging.DEBUG)
    bus = SocketBus()
    bus.set_debug(True)
    bus.connect_to_server('localhost', SocketBus.DEFAULT_PORT)
    pkt = Packet(1)
    pkt.set_data(b'This is a test')
    bus.write_packet(pkt)  # PING
    while bus.process_byte() == ErrorCode.NOT_DONE:
        time.sleep(0.001)

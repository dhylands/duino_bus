#!/usr/bin/env python3
"""This module implements the SerialBus, which basically implements
   a serial like interface over a serial port.
"""

import logging
import select
import time
from typing import Union

import serial

from duino_bus.bus import IBus
from duino_bus.packet import ErrorCode, Packet

LOGGER = logging.getLogger(__name__)


class SerialBus(IBus):
    """Implements a bus which utilizes a serial port."""

    def __init__(self):
        super().__init__()
        self.serial = None
        self.baud = 0
        self.rx_buf_len = 0

    def open(self, *args, **kwargs) -> ErrorCode:
        """Tries to open the indicated serial port."""
        # Ensure that a reasonable timeout is set
        timeout = kwargs.get('timeout', 0.1)
        timeout = max(timeout, 0.05)
        kwargs['timeout'] = timeout
        kwargs['bytesize'] = serial.EIGHTBITS
        kwargs['parity'] = serial.PARITY_NONE
        kwargs['stopbits'] = serial.STOPBITS_ONE
        kwargs['xonxoff'] = False
        kwargs['rtscts'] = False
        kwargs['dsrdtr'] = False
        self.serial = serial.Serial(*args, **kwargs)
        return ErrorCode.NONE

    def is_data_available(self) -> bool:
        """Returns True if data is available, False otherwise."""
        poll = select.poll()
        poll.register(self.serial, select.POLLIN)
        events = poll.poll(0)
        return len(events) > 0

    def is_space_available(self) -> bool:
        """Returns Trus if space is available to write another byte, False otherwise."""
        poll = select.poll()
        poll.register(self.serial, select.POLLOUT)
        events = poll.poll(0)
        return len(events) > 0

    def read_byte(self) -> Union[int, None]:
        """Reads a byte from the bus. This function is non-blocking.
           Returns None if no character was available to be read, or the character.
        """
        data = self.serial.read(1)
        if data:
            return data[0]
        return None

    def write_byte(self, byte: int) -> None:
        """Writes a byte to the bus."""
        self.serial.write(byte.to_bytes(1, 'little'))


if __name__ == '__main__':
    # To invoke this use:
    # python -m duino_bus.serial_bus
    logging.basicConfig(level=logging.DEBUG)
    bus = SerialBus()
    bus.set_debug(True)
    bus.open('/dev/ttyUSB0', baudrate=115200)
    pkt = Packet(1)
    pkt.set_data(b'This is a test')
    bus.write_packet(pkt)  # PING
    while bus.process_byte() == ErrorCode.NOT_DONE:
        time.sleep(0.001)

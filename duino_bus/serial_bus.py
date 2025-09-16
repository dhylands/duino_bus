#!/usr/bin/env python3
"""This module implements the SerialBus, which basically implements
   a serial like interface over a serial port.
"""

import logging
import select
import time
from typing import Union

import serial
import serial.serialutil

from duino_bus.bus import IBus
from duino_bus.packet import ErrorCode, Packet

LOGGER = logging.getLogger(__name__)


class SerialBus(IBus):
    """Implements a bus which utilizes a serial port."""

    def __init__(self):
        super().__init__()
        self.serial = serial.Serial()
        self.baud = 0
        self.rx_buf_len = 0

    def is_open(self) -> bool:
        """Returns true if the serial bus is open."""
        return self.serial is not None

    def open(self, *args, **kwargs) -> int:
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
        kwargs['write_timeout'] = 0.1
        self.serial = serial.Serial(*args, **kwargs)
        self.bus_opened(self.serial.fileno())
        return ErrorCode.NONE

    def close(self) -> None:
        """Closes a previously opened serial port."""
        if self.serial is not None:
            self.serial.reset_input_buffer()
            self.serial.reset_output_buffer()
            self.serial.close()
            self.serial = None
            self.bus_closed()

    def is_data_available(self) -> bool:
        """Returns True if data is available, False otherwise."""
        if self.serial is None:
            return False
        poll = select.poll()
        poll.register(self.serial, select.POLLIN)
        events = poll.poll(0)
        return len(events) > 0

    def is_space_available(self) -> bool:
        """Returns Trus if space is available to write another byte, False otherwise."""
        if self.serial is None:
            return False
        poll = select.poll()
        poll.register(self.serial, select.POLLOUT)
        events = poll.poll(0)
        return len(events) > 0

    def read_byte(self) -> Union[int, None]:
        """Reads a byte from the bus. This function is non-blocking.
           Returns None if no character was available to be read, or the character.
        """
        if self.serial is None:
            return None
        data = self.serial.read(1)
        if data:
            return data[0]
        return None

    def write_byte(self, byte: int) -> bool:
        """Writes a byte to the bus."""
        if self.serial is None:
            return False
        try:
            self.serial.write(byte.to_bytes(1, 'little'))
        except serial.serialutil.SerialTimeoutException:
            return False
        return True


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

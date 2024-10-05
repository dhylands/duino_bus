"""This module implements a serial bus class which talks to bioloid
devices through a serial port.

"""
import logging
import select
import time

import serial

from duino_bus.bus import IBus
from duino_bus.packet import ErrorCode, Packet


class SerialBus(IBus):
    """Implements a BioloidBus which sends commands to a bioloid device
    via a BioloidSerialPort.
    """

    def __init__(self, port, baud=115200):
        super().__init__()
        self.serial_port = serial.Serial(
                port=port,
                baudrate=baud,
                timeout=0.1,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                xonxoff=False,
                rtscts=False,
                dsrdtr=False
        )

    def is_data_available(self) -> bool:
        """Returns True if data is available, False otherwise."""
        poll = select.poll()
        poll.register(self.serial_port, select.POLLIN)
        events = poll.poll(0)
        return len(events) > 0

    def is_space_available(self) -> bool:
        """Returns Trus if space is available to write another byte, False otherwise."""
        poll = select.poll()
        poll.register(self.serial_port, select.POLLOUT)
        events = poll.poll(0)
        return len(events) > 0

    def read_byte(self):
        """Reads a byte from the bus. This function will return None if
        no character was read within the designated timeout.

        The max Return Delay time is 254 x 2 usec = 508 usec (the
        default is 500 usec). This represents the minimum time between
        receiving a packet and sending a response.

        """
        data = self.serial_port.read()
        if data:
            return data[0]
        return None

    def write_byte(self, byte: int) -> None:
        """Writes a byte to the bus."""
        self.serial_port.write(byte.to_bytes(1, 'little'))


# pylint: disable=duplicate-code
if __name__ == '__main__':
    # To invoke this use:
    # python -m duino_bus.linux_serial_bus
    logging.basicConfig(level=logging.DEBUG)
    bus = SerialBus('/dev/ttyUSB0')
    bus.set_debug(True)
    pkt = Packet(1)
    pkt.set_data(b'This is a test')
    bus.write_packet(pkt)  # PING
    while bus.process_byte() == ErrorCode.NOT_DONE:
        time.sleep(0.001)

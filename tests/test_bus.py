#!/usr/bin/env python3
"""
Tests for bus.py
"""

import binascii
from typing import Union
import unittest

from duino_bus.bus import IBus
from duino_bus.packet import ErrorCode, Packet


# Note: If the class name starts with Test then pytest thinks it's a test class
#       which is why we use `TstBus` instead.
class TstBus(IBus):

    def __init__(self, data_str: str) -> None:
        super().__init__()
        self.cmd_idx = 0
        self.cmd_data = binascii.unhexlify(data_str.replace(' ', ''))
        self.rsp_data = bytearray(0)

    def read_byte(self) -> Union[int, None]:
        """Reads a byte from the bus. This function is non-blocking.
           Returns None if no character was available to be read, or the character.
        """
        if self.cmd_idx < len(self.cmd_data):
            byte = self.cmd_data[self.cmd_idx]
            self.cmd_idx += 1
            return byte
        return None

    def write_byte(self, byte: int) -> None:
        """Writes a byte to the bus."""
        self.rsp_data.append(byte)

    def as_str(self, data) -> str:
        return binascii.hexlify(data, ' ').decode('utf-8')


class TestBus(unittest.TestCase):

    def test_is_data_available(self):
        bus = IBus()
        with self.assertRaises(NotImplementedError):
            bus.is_data_available()

    def test_read_byte(self):
        bus = IBus()
        with self.assertRaises(NotImplementedError):
            bus.read_byte()

    def test_is_space_available(self):
        bus = IBus()
        with self.assertRaises(NotImplementedError):
            bus.is_space_available()

    def test_write_byte(self):
        bus = IBus()
        with self.assertRaises(NotImplementedError):
            bus.write_byte(0)

    def test_process_byte(self):
        bus = TstBus('c0 01 07 c0')
        err = ErrorCode.NONE
        for i in range(len(bus.cmd_data) - 1):
            self.assertEqual(bus.process_byte(), ErrorCode.NOT_DONE)
        self.assertEqual(bus.process_byte(), ErrorCode.NONE)
        self.assertEqual(bus.packet.get_command(), 1)
        self.assertEqual(bus.packet.get_data(), bytearray(0))
        self.assertEqual(bus.packet.get_crc(), 0x07)

    def test_process_byte_no_data(self):
        bus = TstBus('')
        self.assertEqual(bus.process_byte(), ErrorCode.NOT_DONE)

    def test_write_packet(self):
        bus = TstBus('')
        packet = Packet(1)
        packet.set_data(bytearray([2, 3, 4]))
        bus.write_packet(packet)
        self.assertEqual(bus.as_str(bus.rsp_data), 'c0 01 02 03 04 e3 c0')

    def test_set_debug(self):
        # If you run `python -m pytest -s tests/test_bus.py` you should see the packet printed out twice
        bus = TstBus('c0 01 02 03 04 e3 c0')
        bus.set_debug(True)
        print('')
        print('----- Start debug -----')
        for i in range(len(bus.cmd_data) - 1):
            self.assertEqual(bus.process_byte(), ErrorCode.NOT_DONE)
        self.assertEqual(bus.process_byte(), ErrorCode.NONE)
        bus.write_packet(bus.packet)
        print('----- End debug -----')

    def test_send_command_get_response(self):
        bus = TstBus('c0 01 02 03 04 e3 c0')
        bus.set_debug(True)
        print('')
        print('----- Start debug -----')
        packet = Packet(1)
        err, pkt = bus.send_command_get_response(packet)
        print('----- End debug -----')

    def test_send_command_get_response_timeout(self):
        bus = TstBus('')
        bus.set_debug(True)
        bus.encoder.state = 0xff
        print('')
        print('----- Start debug -----')
        packet = Packet(1)
        err, pkt = bus.send_command_get_response(packet)
        print('----- End debug -----')
        self.assertEqual(err, ErrorCode.TIMEOUT)
        self.assertEqual(pkt, None)
#!/usr/bin/env python3

# This file tests the packet parser

import unittest
import binascii

from duino_bus.dump_mem import dump_mem
from duino_bus.packet import ErrorCode, Packet
from duino_bus.packet_encoder import PacketEncoder


class TestPacket(unittest.TestCase):

    def setUp(self) -> None:
        self.encoder = PacketEncoder()

    def write_packet(self, pkt: Packet) -> None:
        self.encoder.encode_start(pkt)
        self.data = bytearray()
        while True:
            err, byte = self.encoder.encode_byte()
            self.data.append(byte)
            if err == ErrorCode.NONE:
                break

    def as_str(self) -> str:
        return binascii.hexlify(self.data, ' ').decode('utf-8')

    def test_write_no_data(self):
        pkt = Packet(1)
        self.write_packet(pkt)
        self.assertEqual(self.as_str(), 'c0 01 07 c0')

    def test_get_crc(self):
        pkt = Packet(1)
        self.write_packet(pkt)
        self.assertEqual(self.as_str(), 'c0 01 07 c0')
        self.assertEqual(pkt.get_crc(), 0x07)

    def test_write_1_byte(self):
        pkt = Packet(1, bytearray([2]))
        self.write_packet(pkt)
        self.assertEqual(self.as_str(), 'c0 01 02 1b c0')

    def test_append_data(self):
        pkt = Packet(1)
        pkt.append_data(bytearray([2]))
        self.write_packet(pkt)
        self.assertEqual(self.as_str(), 'c0 01 02 1b c0')

    def test_write_2_bytes(self):
        pkt = Packet(1, bytearray([2, 3]))
        self.write_packet(pkt)
        self.assertEqual(self.as_str(), 'c0 01 02 03 48 c0')

    def test_write_esc_end(self):
        pkt = Packet(0xc0, bytearray([2, 3]))
        self.write_packet(pkt)
        self.assertEqual(self.as_str(), 'c0 db dc 02 03 ae c0')

    def test_write_esc_esc(self):
        pkt = Packet(0xdb, bytearray([2, 3]))
        self.write_packet(pkt)
        self.assertEqual(self.as_str(), 'c0 db dd 02 03 e0 c0')

    def test_debug(self):
        self.encoder.set_debug(True)
        self.test_write_no_data()

    def test_bad_state(self):
        pkt = Packet(1)
        self.encoder.encode_start(pkt)
        self.encoder.state = 255
        err, _ = self.encoder.encode_byte()
        self.assertEqual(err, ErrorCode.BAD_STATE)


if __name__ == '__main__':
    unittest.main()

#!/usr/bin/env python3

# This file tests the packet parser

import unittest
import binascii

from DuinoBus.dump_mem import dump_mem
from DuinoBus.packet import Error, Packet


class TestPacket(unittest.TestCase):

    def parse_packet(self, data_str, expected_err=Error.NONE):
        data = binascii.unhexlify(data_str.replace(' ', ''))
        pkt = Packet()
        for i in range(len(data)):
            byte = data[i]
            err = pkt.parse_byte(byte)
            if i + 1 == len(data):
                self.assertEqual(err, expected_err)
                if err == Error.NONE:
                    dump_mem(pkt.data, 'Parsed')
            else:
                self.assertEqual(err, Error.NOT_DONE)
        return pkt

    def as_str(self) -> str:
        return binascii.hexlify(self.data, ' ').decode('utf-8')

    def write_packet(self, pkt: Packet) -> None:
        self.data = bytearray()
        pkt.write_packet(self.write_byte)

    def write_byte(self, byte: int) -> None:
        self.data.append(byte)

    def test_too_small(self):
        self.parse_packet('c0 01 c0', Error.TOO_SMALL)

    def test_pkt_no_data(self):
        pkt = self.parse_packet('c0 01 07 c0', Error.NONE)
        self.assertEqual(pkt.cmd, 1)
        self.assertEqual(len(pkt.data), 0)
        self.assertEqual(pkt.state, Packet.STATE_IDLE)

    def test_pkt_data_1_byte(self):
        pkt = self.parse_packet('c0 01 02 1b c0', Error.NONE)
        self.assertEqual(pkt.cmd, 1)
        self.assertEqual(len(pkt.data), 1)
        self.assertEqual(pkt.data, bytearray([2]))
        self.assertEqual(pkt.state, Packet.STATE_IDLE)

    def test_pkt_data_2_bytes(self):
        pkt = self.parse_packet('c0 01 02 03 48 c0', Error.NONE)
        self.assertEqual(pkt.cmd, 1)
        self.assertEqual(len(pkt.data), 2)
        self.assertEqual(pkt.data, bytearray([2, 3]))
        self.assertEqual(pkt.state, Packet.STATE_IDLE)

    def test_pkt_data_esc_end(self):
        pkt = self.parse_packet('c0 db dc 02 03 ae c0', Error.NONE)
        self.assertEqual(pkt.cmd, 0xc0)
        self.assertEqual(len(pkt.data), 2)
        self.assertEqual(pkt.data, bytearray([2, 3]))
        self.assertEqual(pkt.state, Packet.STATE_IDLE)

    def test_pkt_data_esc_esc(self):
        pkt = self.parse_packet('c0 db dd 02 03 e0 c0', Error.NONE)
        self.assertEqual(pkt.cmd, 0xdb)
        self.assertEqual(len(pkt.data), 2)
        self.assertEqual(pkt.data, bytearray([2, 3]))
        self.assertEqual(pkt.state, Packet.STATE_IDLE)

    def test_write_no_data(self):
        pkt = Packet(1)
        self.write_packet(pkt)
        self.assertEqual(self.as_str(), 'c0 01 07 c0')

    def test_write_1_byte(self):
        pkt = Packet(1, bytearray([2]))
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


if __name__ == '__main__':
    unittest.main()

#!/usr/bin/env python3

# This file tests the packet parser

import binascii
import unittest

from duino_bus.dump_mem import dump_mem
from duino_bus.packet import ErrorCode, Packet
from duino_bus.packet_decoder import PacketDecoder


class TestPacketDecoder(unittest.TestCase):

    def setUp(self):
        self.pkt = Packet()
        self.decoder = PacketDecoder(self.pkt)

    def parse_packet(self, data_str, expected_err=ErrorCode.NONE):
        data = binascii.unhexlify(data_str.replace(' ', ''))
        for i in range(len(data)):
            byte = data[i]
            err = self.decoder.decode_byte(byte)
            if i + 1 == len(data):
                self.assertEqual(err, expected_err)
                if err == ErrorCode.NONE:
                    dump_mem(self.pkt.data, 'Parsed')
            else:
                self.assertEqual(err, ErrorCode.NOT_DONE)
        return self.pkt

    def as_str(self) -> str:
        return binascii.hexlify(self.data, ' ').decode('utf-8')

    def test_null_packet(self):
        pkt = self.parse_packet('c0 c0 01 07 c0', ErrorCode.NONE)
        self.assertEqual(pkt.cmd, 1)
        self.assertEqual(len(pkt.data), 0)
        self.assertEqual(self.decoder.state, PacketDecoder.STATE_IDLE)

    def test_dump_no_data(self):
        pkt = self.parse_packet('c0 01 07 c0', ErrorCode.NONE)
        pkt.dump('Test')

    def test_set_debug(self):
        self.decoder.set_debug(True)
        pkt = self.parse_packet('c0 01 07 c0', ErrorCode.NONE)

    def test_dump_with_data(self):
        pkt = self.parse_packet('c0 01 02 1b c0', ErrorCode.NONE)
        pkt.dump('Test')

    def test_get_data(self):
        pkt = self.parse_packet('c0 01 02 1b c0', ErrorCode.NONE)
        self.assertEqual(pkt.get_data(), bytearray([2]))

    def test_too_small(self):
        self.parse_packet('c0 01 c0', ErrorCode.TOO_SMALL)

    def test_pkt_no_data(self):
        pkt = self.parse_packet('c0 01 07 c0', ErrorCode.NONE)
        self.assertEqual(pkt.cmd, 1)
        self.assertEqual(len(pkt.data), 0)
        self.assertEqual(self.decoder.state, PacketDecoder.STATE_IDLE)

    def test_pkt_crc_error(self):
        self.decoder.set_debug(True)
        pkt = self.parse_packet('c0 01 08 c0', ErrorCode.CRC)
        self.assertEqual(pkt.cmd, 1)
        self.assertEqual(len(pkt.data), 0)
        self.assertEqual(self.decoder.state, PacketDecoder.STATE_IDLE)

    def test_pkt_no_data_esc_end(self):
        pkt = self.parse_packet('c0 db dc 4e c0', ErrorCode.NONE)
        self.assertEqual(pkt.cmd, 0xc0)
        self.assertEqual(len(pkt.data), 0)
        self.assertEqual(self.decoder.state, PacketDecoder.STATE_IDLE)

    def test_pkt_data_1_byte(self):
        pkt = self.parse_packet('c0 01 02 1b c0', ErrorCode.NONE)
        self.assertEqual(pkt.cmd, 1)
        self.assertEqual(len(pkt.data), 1)
        self.assertEqual(pkt.data, bytearray([2]))
        self.assertEqual(self.decoder.state, PacketDecoder.STATE_IDLE)

    def test_pkt_data_2_bytes(self):
        pkt = self.parse_packet('c0 01 02 03 48 c0', ErrorCode.NONE)
        self.assertEqual(pkt.cmd, 1)
        self.assertEqual(len(pkt.data), 2)
        self.assertEqual(pkt.data, bytearray([2, 3]))
        self.assertEqual(self.decoder.state, PacketDecoder.STATE_IDLE)

    def test_pkt_data_esc_end(self):
        pkt = self.parse_packet('c0 db dc 02 03 ae c0', ErrorCode.NONE)
        self.assertEqual(pkt.cmd, 0xc0)
        self.assertEqual(len(pkt.data), 2)
        self.assertEqual(pkt.data, bytearray([2, 3]))
        self.assertEqual(self.decoder.state, PacketDecoder.STATE_IDLE)

    def test_pkt_data_esc_esc(self):
        pkt = self.parse_packet('c0 db dd 02 03 e0 c0', ErrorCode.NONE)
        self.assertEqual(pkt.cmd, 0xdb)
        self.assertEqual(len(pkt.data), 2)
        self.assertEqual(pkt.data, bytearray([2, 3]))
        self.assertEqual(self.decoder.state, PacketDecoder.STATE_IDLE)

    def test_pkt_too_big(self):
        self.decoder.set_debug(True)
        save_max_data = Packet.MAX_DATA_LEN
        Packet.MAX_DATA_LEN = 2
        pkt = self.parse_packet('c0 aa 02 03 48', ErrorCode.TOO_MUCH_DATA)
        Packet.MAX_DATA_LEN = save_max_data

    def test_bad_state(self):
        self.decoder.state = 255
        pkt = self.parse_packet('c0', ErrorCode.BAD_STATE)


if __name__ == '__main__':
    unittest.main()

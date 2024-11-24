#!/usr/bin/env python3

# This file tests the packet parser

import unittest

from duino_bus.dump_mem import dump_mem
from duino_bus.packet import ERROR_STRS, ErrorCode, Packet


class TestPacket(unittest.TestCase):

    def test_too_small(self):
        pass

    def test_error_code_as_str(self):
        self.assertEqual(ErrorCode.as_str(ErrorCode.NONE), 'NONE')
        self.assertEqual(ErrorCode.as_str(ErrorCode.NOT_DONE), 'NOT_DONE')
        self.assertEqual(ErrorCode.as_str(ErrorCode.CRC), 'CRC')
        self.assertEqual(ErrorCode.as_str(ErrorCode.TIMEOUT), 'TIMEOUT')
        self.assertEqual(ErrorCode.as_str(ErrorCode.TOO_MUCH_DATA), 'TOO_MUCH_DATA')
        self.assertEqual(ErrorCode.as_str(ErrorCode.TOO_SMALL), 'TOO_SMALL')
        self.assertEqual(ErrorCode.as_str(ErrorCode.BAD_STATE), 'BAD_STATE')
        self.assertEqual(ErrorCode.as_str(ErrorCode.OS), 'OS')
        self.assertEqual(ErrorCode.as_str(255), '???')

        self.assertEqual(len(ERROR_STRS), 8)

    def test_set_data(self) -> None:
        pkt = Packet(0xcc)
        pkt.set_data(b'123')
        self.assertEqual(pkt.get_command(), 0xcc)
        self.assertEqual(pkt.get_data(), b'123')


if __name__ == '__main__':
    unittest.main()

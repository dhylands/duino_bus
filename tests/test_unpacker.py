import unittest

from duino_bus.unpacker import Unpacker

class TestUnPacker(unittest.TestCase):

    def test_unpack_u8(self):
        unpacker = Unpacker(b'\x11')
        self.assertEqual(unpacker.unpack_u8(), 0x11)

    def test_unpack_u16(self):
        unpacker = Unpacker(b'\x22\x11')
        self.assertEqual(unpacker.unpack_u16(), 0x1122)

    def test_unpack_u32(self):
        unpacker = Unpacker(b'\x44\x33\x22\x11')
        self.assertEqual(unpacker.unpack_u32(), 0x11223344)

    def test_unpack_str(self):
        unpacker = Unpacker(b'\x04ABC\x00')
        self.assertEqual(unpacker.unpack_str(), 'ABC')

    def test_unpack_data(self):
        unpacker = Unpacker(b'ABC')
        self.assertEqual(unpacker.unpack_data(3), b'ABC')

    def test_unpack_multiple(self):
        unpacker = Unpacker(b'\x11\x22\x11\x44\x33\x22\x11\x04ABC\x00')
        self.assertEqual(unpacker.unpack_u8(), 0x11)
        self.assertEqual(unpacker.unpack_u16(), 0x1122)
        self.assertEqual(unpacker.unpack_u32(), 0x11223344)
        self.assertEqual(unpacker.unpack_str(), 'ABC')

    def test_unpack_more_data(self) -> None:
        unpacker = Unpacker(b'A')
        self.assertTrue(unpacker.more_data())
        self.assertEqual(unpacker.unpack_u8(), 0x41)
        self.assertFalse(unpacker.more_data())

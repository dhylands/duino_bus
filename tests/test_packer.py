import unittest

from duino_bus.packer import Packer
from duino_bus.packet import Packet

class TestPacker(unittest.TestCase):

    def test_pack_u8(self):
        packer = Packer()
        packer.pack_u8(0x11)
        self.assertEqual(packer.data, b'\x11')

    def test_pack_u16(self):
        packer = Packer()
        packer.pack_u16(0x1122)
        self.assertEqual(packer.data, b'\x22\x11')

    def test_pack_u32(self):
        packer = Packer()
        packer.pack_u32(0x11223344)
        self.assertEqual(packer.data, b'\x44\x33\x22\x11')

    def test_pack_bytestr(self):
        packer = Packer()
        packer.pack_str(b'ABC')
        self.assertEqual(packer.data, b'\x04ABC\x00')

    def test_pack_str(self):
        packer = Packer()
        packer.pack_str('ABC')
        self.assertEqual(packer.data, b'\x04ABC\x00')

    def test_pack_data(self):
        packer = Packer()
        packer.pack_data(b'ABC')
        self.assertEqual(packer.data, b'ABC')

    def test_pack_multiple(self):
        packer = Packer()
        packer.pack_u8(0x11)
        packer.pack_u16(0x1122)
        packer.pack_u32(0x11223344)
        packer.pack_str(b'ABC')
        self.assertEqual(packer.data, b'\x11\x22\x11\x44\x33\x22\x11\x04ABC\x00')

    def test_pack_with_data(self):
        data = bytearray(0)
        pkt = Packet(1, data)
        packer = Packer(pkt)
        packer.pack_u16(0x1122)
        self.assertEqual(data, b'\x22\x11')

    def test_pack_with_data2(self):
        pkt = Packet(1, b'AB')
        packer = Packer(pkt)
        packer.pack_u16(0x1122)
        self.assertEqual(pkt.get_data(), b'\x41\x42\x22\x11')

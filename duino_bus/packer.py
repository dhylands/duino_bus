"""
This modules implements a binary packer.
"""

from typing import Union

from duino_bus.packet import Packet


class Packer:
    """Implements a binary data packer."""

    def __init__(self, pkt: Union[Packet, None] = None) -> None:
        if pkt is None:
            self.data = bytearray(0)
        else:
            self.data = pkt.data

    def pack_u8(self, data: int) -> None:
        """Packs a uint8 into the binary data"""
        self.data.append(data & 0xff)

    def pack_u16(self, data: int) -> None:
        """Packs a uint16 into the binary data"""
        self.data.append(data & 0xff)
        self.data.append((data >> 8) & 0xff)

    def pack_u32(self, data: int) -> None:
        """Packs a uint8 into the binary data"""
        self.data.append(data & 0xff)
        self.data.append((data >> 8) & 0xff)
        self.data.append((data >> 16) & 0xff)
        self.data.append((data >> 24) & 0xff)

    def pack_data(self, s: Union[bytes, bytearray]) -> None:
        """Packs arbitrary data into the binary data."""
        self.data.extend(s)

    def pack_str(self, s: Union[str, bytes, bytearray]) -> None:
        """Packs a string into the binary data."""
        if isinstance(s, str):
            s = bytes(s, 'utf-8')
        self.pack_u8(len(s) + 1)
        self.pack_data(s)
        self.pack_u8(0)

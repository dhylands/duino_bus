"""
This modules implements a binary unpacker.
"""

from typing import Union


class Unpacker:
    """Implements a binary data unpacker."""

    def __init__(self, data: Union[bytes, bytearray]) -> None:
        self.data = data
        self.idx = 0

    def more_data(self) -> bool:
        """Returns true if there is more data available to unpack."""
        return self.idx < len(self.data)

    def unpack_u8(self) -> int:
        """Packs a uint8 into the binary data"""
        val = self.data[self.idx]
        self.idx += 1
        return val

    def unpack_u16(self) -> int:
        """Packs a uint16 into the binary data"""
        val = int.from_bytes(self.data[self.idx:self.idx + 2], 'little')
        self.idx += 2
        return val

    def unpack_u32(self) -> int:
        """Packs a uint8 into the binary data"""
        val = int.from_bytes(self.data[self.idx:self.idx + 4], 'little')
        self.idx += 4
        return val

    def unpack_data(self, num_bytes: int) -> Union[bytes, bytearray]:
        """Packs arbitrary data into the binary data."""
        val = self.data[self.idx:self.idx + num_bytes]
        self.idx += num_bytes
        return val

    def unpack_str(self) -> str:
        """Packs a string into the binary data."""
        str_len = self.unpack_u8()
        c_str = self.unpack_data(str_len)
        if len(c_str) > 1:
            c_str = c_str[:-1]
        return str(c_str, 'utf-8')

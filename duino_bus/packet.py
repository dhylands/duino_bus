"""
This module defines Packet class.
"""

from typing import ByteString, Union
import crcmod

from duino_bus.dump_mem import dump_mem

ERROR_STRS = ['NONE', 'NOT_DONE', 'CRC', 'TIMEOUT', 'TOO_MUCH_DATA', 'TOO_SMALL', 'BAD_STATE', 'OS']


# pylint: disable=too-few-public-methods
class ErrorCode:
    """Constants for Errors returns by the parsing funtions."""
    NONE = 0  # No Error
    NOT_DONE = 1  # Indicates that parsing is not complete
    CRC = 2  # CRC error occurred during parsing
    TIMEOUT = 3  # Indicates that a timeout occurred while waiting for a reply.
    TOO_MUCH_DATA = 4  # Packet storage isn't big enough
    TOO_SMALL = 5  # Not enough data for a packet
    BAD_STATE = 6  # Bad parsing state
    OS = 7  # OS error

    @staticmethod
    def as_str(err: int) -> str:
        """Returns a string representation of an ErrorCode."""
        if err < 0 or err > len(ERROR_STRS):
            return '???'
        return ERROR_STRS[err]


class Packet:
    """
    Encapsulates the packets sent between devices.
    Packets are SLIP encoded, and the length is inferred from the decoded packet.
    The first byte of each packet is the command.
    The last byte of the packet is an 8-bit CRC (crcmod.predefined.mkCrcFun('crc-8'))
    Each packet has data bytes between the command and the CRC.
    """

    END = 0xC0  # Start/End of Frame
    ESC = 0xDB  # Next char is escaped
    ESC_END = 0xDC  # Escape an END character
    ESC_ESC = 0xDD  # Escape an ESC character

    STATE_IDLE = 0  # Haven't started parsing a packet yet.
    STATE_PACKET = 1  # Parsing a packet
    STATE_ESCAPE = 2  # Parsing an escape

    MAX_DATA_LEN = 256

    CRC_FN = crcmod.predefined.mkCrcFun('crc-8')

    def __init__(self, cmd: Union[int, None] = None, data: Union[ByteString, None] = None) -> None:
        """Constructs a packet from a buffer, if provided."""
        if cmd is None:
            cmd = 0
        self.cmd = cmd
        if isinstance(data, bytearray):
            self.data = data
        else:
            self.data: bytearray = bytearray()
            if data is not None:
                self.data.extend(data)
        self.crc = 0

    def dump(self, label: str) -> None:
        """
        Dumps the contents of a packet.
        """
        print(
                f'{label} Command: 0x{self.cmd:02x} ({str(self.cmd)}) Len: {len(self.data)} '
                f'CRC: 0x{self.crc:02x}'
        )
        if len(self.data) > 0:
            dump_mem(self.data, label)

    def get_command(self) -> Union[int, None]:
        """Returns the command from the packet."""
        return self.cmd

    def set_command(self, cmd: int) -> None:
        """Sets the command in the packet."""
        self.cmd = cmd

    def get_data_len(self) -> int:
        """Returns the length of the data portion of the packet."""
        return len(self.data)

    def get_data(self) -> bytearray:
        """Returns the data portion of the packet."""
        return bytearray(self.data)

    def get_data_byte(self, idx: int) -> int:
        """Returns one byte of the packet data."""
        return self.data[idx]

    def set_data(self, data: Union[bytes, bytearray]) -> None:
        """Sets the packet data."""
        if isinstance(data, bytearray):
            self.data = data
        else:
            self.data = bytearray(data)

    def append_byte(self, byte: int) -> None:
        """Appends a byte to the packet data."""
        self.data.append(byte)

    def append_data(self, data: bytearray) -> None:
        """Appends data to the packet data."""
        self.data.extend(data)

    def get_crc(self) -> int:
        """Returns the CRC from the packet."""
        return self.crc

    def calc_crc(self) -> int:
        """Calculates and returns the CRC using the command/packet data."""
        crc = Packet.CRC_FN(self.cmd.to_bytes(1, 'little'), 0)
        return Packet.CRC_FN(self.data, crc)

    def calc_and_store_crc(self) -> None:
        """Calculates the CRC of the data and saves it in the packet."""
        self.crc = self.calc_crc()

    def extract_crc(self) -> int:
        """Used by the packet decoder, this function extracts the CRC from the
           end of the data and stores it in the CRC.
        """
        self.crc = self.data[-1]
        self.data = self.data[:-1]
        return self.crc

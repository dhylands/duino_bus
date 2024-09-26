"""
This module defines Packet class.
"""

from typing import ByteString, Callable, Union
import crcmod

from DuinoBus.dump_mem import dump_mem


# pylint: disable=too-few-public-methods
class Error:
    """Constants for Errors returns by the parsing funtions."""
    NONE = 0  # No Error
    NOT_DONE = 1  # Indicates that parsing is not complete
    CRC = 2  # CRC error occurred during parsing
    TIMEOUT = 3  # Indicates that a timeout occurred while waiting for a reply.
    TOO_MUCH_DATA = 4  # Packet storage isn't big enough
    TOO_SMALL = 5  # Not enough data for a packet
    BAD_STATE = 6  # Bad parsing state


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

    def __init__(self, cmd: Union[int, None] = None, data: Union[ByteString, None] = None) -> None:
        """Constructs a packet from a buffer, if provided."""
        self.cmd = cmd
        if data is None:
            self.data = bytearray()
        else:
            self.data = data
        self.crc = crcmod.predefined.mkCrcFun('crc-8')
        self.state = Packet.STATE_IDLE

    def dump(self, prefix: str) -> None:
        """
        Dumps the contents of a packet.
        """
        print(f'{prefix} Cmd: 0x{self.cmd:02x}')
        if len(self.data) > 0:
            dump_mem(self.data, prefix)

    def write_packet(self, write_fn: Callable) -> None:
        """
        Writes a packet sending each byte thru `write_fn()`
        """
        self.write_raw_byte(Packet.END, write_fn)
        self.write_escaped_byte(self.cmd, write_fn)
        crc = self.crc(bytes([self.cmd]))
        if len(self.data) > 0:
            for byte in self.data:
                self.write_escaped_byte(byte, write_fn)
            crc = self.crc(self.data, crc)
        self.write_escaped_byte(crc, write_fn)
        self.write_raw_byte(Packet.END, write_fn)

    def write_escaped_byte(self, byte: int, write_fn: Callable) -> None:
        """
        Writes a byte thru `write_fn()` escaping as necessary.
        """
        if byte == Packet.END:
            self.write_raw_byte(Packet.ESC, write_fn)
            self.write_raw_byte(Packet.ESC_END, write_fn)
        elif byte == Packet.ESC:
            self.write_raw_byte(Packet.ESC, write_fn)
            self.write_raw_byte(Packet.ESC_ESC, write_fn)
        else:
            self.write_raw_byte(byte, write_fn)

    def write_raw_byte(self, byte: int, write_fn: Callable) -> None:
        """
        Writes a byte thru `write_fn()` with no escaping.
        """
        write_fn(byte)

    def parse_byte(self, byte: int) -> int:
        """
        Runs a single byte through the packet parsing state machine.

        Returns Error.NOT_DONE if the packet is incomplete,
        Error.NONE if the packet was received successfully, and
        Error.CRC if a checksum error is detected.
        """
        if self.state == Packet.STATE_IDLE:
            return self.parse_byte_state_idle(byte)
        if self.state == Packet.STATE_PACKET:
            return self.parse_byte_state_packet(byte)
        if self.state == Packet.STATE_ESCAPE:
            return self.parse_byte_state_escape(byte)
        return self.parse_byte_state_invalid(byte)

    def parse_byte_state_idle(self, byte: int) -> int:
        """
        Runs a single byte through the packet parsing state IDLE.
        """
        if byte == Packet.END:
            self.state = Packet.STATE_PACKET
            self.data = bytearray()
        return Error.NOT_DONE

    def parse_byte_state_packet(self, byte: int) -> int:
        """
        Runs a single byte through the packet parsing state PACKET.
        """
        if len(self.data) >= Packet.MAX_DATA_LEN:
            self.state = Packet.STATE_IDLE
            return Error.TOO_MUCH_DATA
        if byte == Packet.ESC:
            self.state = Packet.STATE_ESCAPE
        elif byte == Packet.END:
            if len(self.data) == 0:
                # We ignore empty packets
                return Error.NOT_DONE
            if len(self.data) == 1:
                # Mimimum packet requires a Cmd and a CRC
                return Error.TOO_SMALL
            dump_mem(self.data[:-1], 'CRC')
            rcvd_crc = self.data[-1]
            calc_crc = self.crc(self.data[:-1])
            if rcvd_crc == calc_crc:
                self.cmd = self.data[0]
                self.data = self.data[1:-1]  # Strip off cmd and CRC
                self.state = Packet.STATE_IDLE
                return Error.NONE
            print(f'CRC Error: Received 0x{rcvd_crc:02x} Expected: 0x{calc_crc:02x}')
            return Error.CRC
        else:
            self.data.append(byte)
        return Error.NOT_DONE

    def parse_byte_state_escape(self, byte: int) -> int:
        """
        Runs a single byte through the packet parsing state ESCAPE.
        """
        if byte == Packet.ESC_END:
            self.data.append(Packet.END)
        elif byte == Packet.ESC_ESC:
            self.data.append(Packet.ESC)
        else:
            self.data.append(byte)
        self.state = Packet.STATE_PACKET
        return Error.NOT_DONE

    def parse_byte_state_invalid(self, _: int) -> int:
        """
        Runs a single byte through the packet parsing state INVALID.
        """
        self.state = Packet.STATE_IDLE
        return Error.BAD_STATE

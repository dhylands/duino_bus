"""
This module defines the PacketEncoder class.
"""
from typing import Tuple

from duino_bus.packet import ErrorCode, Packet


class PacketEncoder:
    """Class which implements a packet encoder.
       The packet encoder converts a packet into it's "over the wire" format.
    """

    STATE_IDLE = 0
    STATE_COMMAND = 1
    STATE_DATA = 2
    STATE_ESCAPE = 3

    def __init__(self) -> None:
        self.packet = Packet()
        self.state = PacketEncoder.STATE_IDLE
        self.encode_idx = 0
        self.escape_char = 0
        self.debug = False

    def set_debug(self, debug: bool) -> None:
        """Sets the debug flag which controls whether decoded packets get dumped."""
        self.debug = debug

    def encode_start(self, pkt: Packet) -> None:
        """Resets the encoder to start encoding a packet."""
        self.packet = pkt
        self.packet.calc_and_store_crc()
        if self.debug:
            self.packet.dump('Sent')
        self.state = PacketEncoder.STATE_IDLE

    def handle_escape(self, byte: int) -> Tuple[int, int]:
        """Helper function for encoding escape sequences."""
        if byte == Packet.END:
            self.escape_char = Packet.ESC_END
            return (PacketEncoder.STATE_ESCAPE, Packet.ESC)
        if byte == Packet.ESC:
            self.escape_char = Packet.ESC_ESC
            return (PacketEncoder.STATE_ESCAPE, Packet.ESC)
        return (PacketEncoder.STATE_DATA, byte)

    def encode_byte(self) -> Tuple[ErrorCode, int]:
        """Encodes the next byte of the packet."""
        if self.state == PacketEncoder.STATE_IDLE:
            return self.encode_byte_idle()

        if self.state == PacketEncoder.STATE_COMMAND:
            return self.encode_byte_command()

        if self.state == PacketEncoder.STATE_DATA:
            return self.encode_byte_data()

        if self.state == PacketEncoder.STATE_ESCAPE:
            return self.encode_byte_escape()

        return (ErrorCode.BAD_STATE, 0)

    def encode_byte_idle(self) -> Tuple[ErrorCode, int]:
        """Handles the IDLE state of the encoder."""
        self.state = PacketEncoder.STATE_COMMAND
        return (ErrorCode.NOT_DONE, Packet.END)

    def encode_byte_command(self) -> Tuple[ErrorCode, int]:
        """Handles the COMMAND state of the encoder."""
        self.state, byte = self.handle_escape(self.packet.get_command())
        self.encode_idx = 0
        return (ErrorCode.NOT_DONE, byte)

    def encode_byte_data(self) -> Tuple[ErrorCode, int]:
        """Handles the DATA state of the encoder."""
        if self.encode_idx < self.packet.get_data_len():
            self.state, byte = self.handle_escape(self.packet.get_data_byte(self.encode_idx))
            self.encode_idx += 1
            return (ErrorCode.NOT_DONE, byte)
        if self.encode_idx == self.packet.get_data_len():
            self.state, byte = self.handle_escape(self.packet.calc_crc())
            self.encode_idx += 1
            return (ErrorCode.NOT_DONE, byte)
        self.state = PacketEncoder.STATE_IDLE
        return (ErrorCode.NONE, Packet.END)

    def encode_byte_escape(self) -> Tuple[ErrorCode, int]:
        """Handles the ESCAPE state of the encoder."""
        self.state = PacketEncoder.STATE_DATA
        return (ErrorCode.NOT_DONE, self.escape_char)

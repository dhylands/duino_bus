"""
This module provides the IBus class which describes an API used for talking with
serial like devices.
"""

import logging
import time
from typing import Tuple, Union

from duino_bus.packet import ErrorCode, Packet
from duino_bus.packet_decoder import PacketDecoder
from duino_bus.packet_encoder import PacketEncoder

LOGGER = logging.getLogger(__name__)

RESPONSE_TIMEOUT_SEC = 1  # Time to wait for responses.


class IBus:
    """Base class for abstracting access to various serial type devices."""

    def __init__(self) -> None:
        self.packet = Packet()
        self.decoder = PacketDecoder(self.packet)
        self.encoder = PacketEncoder()

    def is_data_available(self) -> bool:
        """Returns True if data is available, False otherwise."""
        raise NotImplementedError

    def read_byte(self) -> Union[int, None]:
        """Reads a byte from the bus. This function is non-blocking.
           Returns None if no character was available to be read, or the character.
        """
        raise NotImplementedError

    def is_space_available(self) -> bool:
        """Returns Trus if space is available to write another byte, False otherwise."""
        raise NotImplementedError

    def write_byte(self, byte: int) -> None:
        """Writes a byte to the bus."""
        raise NotImplementedError

    def process_byte(self) -> ErrorCode:
        """Reads a byte from the bus, and runs it through the packet parser."""
        byte = self.read_byte()
        if byte is None:
            return ErrorCode.NOT_DONE
        return self.decoder.decode_byte(byte)

    def write_packet(self, pkt: Packet) -> ErrorCode:
        """Writes a packet to the bus."""
        self.encoder.encode_start(pkt)
        err = ErrorCode.NOT_DONE
        while err == ErrorCode.NOT_DONE:
            err, byte = self.encoder.encode_byte()
            self.write_byte(byte)
        return err

    def set_debug(self, debug: bool) -> None:
        """Sets the debug flag, which controls whether packets get logged."""
        self.decoder.set_debug(debug)
        self.encoder.set_debug(debug)

    def send_command_get_response(
            self,
            cmd_pkt: Packet,
            timeout: int = RESPONSE_TIMEOUT_SEC
    ) -> Tuple[int,
               Union[Packet,
                     None]]:
        """Sends a command and waits for a response."""
        self.write_packet(cmd_pkt)
        end_time = time.monotonic() + timeout
        while self.process_byte() == ErrorCode.NOT_DONE:
            if time.monotonic() > end_time:
                LOGGER.error('Timeout sending packet')
                cmd_pkt.dump('Timeout')
                return (ErrorCode.TIMEOUT, None)
        return (ErrorCode.NONE, self.decoder.packet)

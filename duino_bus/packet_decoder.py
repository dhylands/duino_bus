"""
This module defines the PacketDecoder class.
"""

import logging

from duino_bus.packet import ErrorCode, Packet

LOGGER = logging.getLogger(__name__)


class PacketDecoder:
    """Class which implements a packet decoder.
       The packet decoder converts data from the "over the wire" format into a packet.
    """

    STATE_IDLE = 0
    STATE_COMMAND = 1
    STATE_DATA = 2

    def __init__(self, pkt: Packet) -> None:
        self.packet = pkt
        self.state = PacketDecoder.STATE_IDLE
        self.escape = False
        self.debug = False

    def set_debug(self, debug: bool) -> None:
        """Sets the debug flag which controls whether decoded packets get dumped."""
        self.debug = debug

    def decode_byte(self, byte: int) -> ErrorCode:
        """Runs a single byte through the packet decoder state machine."""
        # Since we need to escape for multiple states, it's easier to put
        # the escape logic here.
        if self.state != PacketDecoder.STATE_IDLE:
            if self.escape:
                if byte == Packet.ESC_END:
                    byte = Packet.END
                elif byte == Packet.ESC_ESC:
                    byte = Packet.ESC
                # We deliberately don't clear escape here so we can tell whether
                # we're processing an escaped END or a regular END.
            else:
                if byte == Packet.ESC:
                    self.escape = True
                    return ErrorCode.NOT_DONE

        if self.state == PacketDecoder.STATE_IDLE:
            return self.decode_byte_idle(byte)

        if self.state == PacketDecoder.STATE_COMMAND:
            return self.decode_byte_command(byte)

        if self.state == PacketDecoder.STATE_DATA:
            return self.decode_byte_data(byte)

        return ErrorCode.BAD_STATE

    def decode_byte_idle(self, byte: int) -> ErrorCode:
        """Handles the IDLE state."""
        if byte == Packet.END:
            self.state = PacketDecoder.STATE_COMMAND
        return ErrorCode.NOT_DONE

    def decode_byte_command(self, byte: int) -> ErrorCode:
        """Handles the COMMAND state."""
        if byte == Packet.END and not self.escape:
            # A regular END marks the beginning/end of a packet
            # An END now means that we got back-to-back ENDs.
            # In this situation we ignore the first END
            return ErrorCode.NOT_DONE
        self.escape = False
        self.packet.set_command(byte)
        self.packet.set_data(bytearray(0))
        self.state = PacketDecoder.STATE_DATA
        return ErrorCode.NOT_DONE

    def decode_byte_data(self, byte: int) -> ErrorCode:
        """Handles the DATA state."""
        if byte == Packet.END and not self.escape:
            self.state = PacketDecoder.STATE_IDLE
            if self.packet.get_data_len() == 0:
                # This means we got END CMD END which is too short
                # since the minimum packet needs to include a CMD and
                # a CRC.
                self.state = PacketDecoder.STATE_COMMAND
                return ErrorCode.TOO_SMALL

            # This END is a real END which marks the end of the packet.
            self.state = PacketDecoder.STATE_IDLE
            rcvd_crc = self.packet.extract_crc()
            expected_crc = self.packet.calc_crc()
            if rcvd_crc == expected_crc:
                if self.debug:
                    self.packet.dump('Rcvd')
                return ErrorCode.NONE
            LOGGER.error('CRC Error: Received 0x%02x Expected 0x%02x', rcvd_crc, expected_crc)
            if self.debug:
                self.packet.dump('CRC ')
            return ErrorCode.CRC

        self.escape = False
        if self.packet.get_data_len() >= Packet.MAX_DATA_LEN:
            if self.debug:
                self.packet.dump('2Big')
            print('Returning TOO_MUCH_DATA')
            return ErrorCode.TOO_MUCH_DATA
        self.packet.append_byte(byte)
        return ErrorCode.NOT_DONE

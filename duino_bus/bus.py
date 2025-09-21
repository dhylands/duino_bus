"""
This module provides the IBus class which describes an API used for talking with
serial like devices.
"""

import logging
import queue
import select
import threading
from typing import Tuple, Union

from duino_bus.packet import ErrorCode, Packet
from duino_bus.packet_decoder import PacketDecoder
from duino_bus.packet_encoder import PacketEncoder
from duino_bus.unpacker import Unpacker

LOGGER = logging.getLogger(__name__)

RESPONSE_TIMEOUT_SEC = 1  # Time to wait for responses.

LOG = 0x03  # Log message
EVENT = 0x04  # Event message

# Logging levels (from duino_log/src/duino_log/Log.h)
LOG_LEVEL_NONE = 0
LOG_LEVEL_FATAL = 1
LOG_LEVEL_ERROR = 2
LOG_LEVEL_WARNING = 3
LOG_LEVEL_INFO = 4
LOG_LEVEL_DEBUG = 5

PYTHON_LOG_LEVEL = [
    logging.NOTSET,
    logging.CRITICAL,
    logging.ERROR,
    logging.WARNING,
    logging.INFO,
    logging.DEBUG,
]


class IBus:
    """Base class for abstracting access to various serial type devices."""

    def __init__(self) -> None:
        self.packet = Packet()
        self.decoder = PacketDecoder(self.packet)
        self.encoder = PacketEncoder()
        self.poll = select.poll()
        self.fileno = -1
        self.thread = None
        self.recv_queue = queue.Queue()

    def close(self) -> None:
        """Closed a previously opened bus."""

    def bus_opened(self, fileno: int) -> None:
        """Called by the derived class when the bus object is opened.
        This function starts a thread which processes asynchronous incoming data
        (i.e. from log messages or event)
        """
        self.fileno = fileno
        self.thread = threading.Thread(target=self.incoming_data_thread, daemon=True)
        self.thread.start()

    def bus_closed(self) -> None:
        """Called by the derived class when the bus object is closed."""
        if self.thread is not None:
            LOGGER.debug('Waiting for incoming data thread to quit')
            self.thread.join()
        self.fileno = -1

    def is_open(self) -> bool:
        """Returns True if the bus has been opened."""
        return False

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

    def write_byte(self, byte: int) -> bool:
        """Writes a byte to the bus."""
        raise NotImplementedError

    def process_byte(self) -> int:
        """Reads a byte from the bus, and runs it through the packet parser."""
        if not self.is_data_available():
            return ErrorCode.NOT_DONE
        byte = self.read_byte()
        if byte is None:
            return ErrorCode.NOT_DONE
        return self.decoder.decode_byte(byte)

    def write_packet(self, pkt: Packet) -> int:
        """Writes a packet to the bus."""
        self.encoder.encode_start(pkt)
        err = ErrorCode.NOT_DONE
        while err == ErrorCode.NOT_DONE:
            err, byte = self.encoder.encode_byte()
            if not self.write_byte(byte):
                return ErrorCode.OS
        return err

    def set_debug(self, debug: bool) -> None:
        """Sets the debug flag, which controls whether packets get logged."""
        self.decoder.set_debug(debug)
        self.encoder.set_debug(debug)

    def send_command_get_response(
            self,
            cmd_pkt: Packet,
            timeout: int = RESPONSE_TIMEOUT_SEC) -> Tuple[int, Union[Packet, None]]:
        """Sends a command and waits for a response."""
        if not self.is_open():
            return (ErrorCode.NOT_OPEN, None)
        err = self.write_packet(cmd_pkt)
        if err != ErrorCode.NONE:
            LOGGER.error('Error writing command')
            cmd_pkt.dump('Error')
            return (err, None)
        try:
            rsp_pkt = self.recv_queue.get(block=True, timeout=timeout)
        except queue.Empty:
            LOGGER.error('Timeout waiting for response')
            cmd_pkt.dump('Timeout')
            return (ErrorCode.TIMEOUT, None)
        return (ErrorCode.NONE, rsp_pkt)

    def incoming_data_thread(self) -> None:
        """Process incoming data, automatically handling event messages."""
        poll = select.poll()
        poll.register(self.fileno, select.POLLIN)
        while True:
            if not self.is_open():
                LOGGER.debug('incoming_data_thread: bus is closed: exiting')
                break
            events = poll.poll(500)
            for fd, event in events:
                if fd == self.fileno:
                    if event & select.POLLIN:
                        if self.process_byte() == ErrorCode.NOT_DONE:
                            continue
                        self.process_packet(self.decoder.packet)
                    elif event & select.POLLERR:
                        LOGGER.error('Error reading serial port')
                        return
        LOGGER.debug('incoming_data_thread: exiting')

    def process_packet(self, pkt: Packet) -> None:
        """Processes a received packet."""
        cmd = pkt.get_command()
        if cmd == LOG:
            unpacker = Unpacker(pkt.get_data())
            level = unpacker.unpack_u8()
            msg = unpacker.unpack_str()
            if level >= len(PYTHON_LOG_LEVEL):
                logging_level = logging.ERROR
            else:
                logging_level = PYTHON_LOG_LEVEL[level]
            LOGGER.log(logging_level, msg)
        else:
            self.recv_queue.put(pkt)

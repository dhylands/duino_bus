/****************************************************************************
 *
 *   @copyright Copyright (c) 2024 Dave Hylands     <dhylands@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the MIT License version as described in the
 *   LICENSE file in the root of this repository.
 *
 ****************************************************************************/
/**
 *   @file   PacketDecoder.cpp
 *
 *   @brief  Decodes packets from their "over the wire" format.
 *
 ****************************************************************************/

#include "PacketDecoder.h"

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <memory>

#include "Crc8.h"
#include "DumpMem.h"
#include "Log.h"

PacketDecoder::PacketDecoder(Packet* pkt) : m_packet{pkt} {}

Packet::Error PacketDecoder::decodeByte(uint8_t byte) {
    // Since we need to escape for multiple states, it's easier to put
    // the escape logic here.
    if (this->m_state != State::IDLE) {
        if (this->m_escape) {
            if (byte == Packet::ESC_END) {
                byte = Packet::END;
            } else if (byte == Packet::ESC_ESC) {
                byte = Packet::ESC;
            }
            // We deliberately don't clear m_escape here so that we can tell
            // if we're processing an escaped character later.
        } else {
            if (byte == Packet::ESC) {
                this->m_escape = true;
                return Packet::Error::NOT_DONE;
            }
        }
    }

    switch (this->m_state) {
        case State::IDLE: {  // We're waiting for the beginning of the packet (0xC0)
            if (byte == Packet::END) {
                this->m_state = State::COMMAND;
            }
            return Packet::Error::NOT_DONE;
        }

        case State::COMMAND: {
            if (byte == Packet::END && !this->m_escape) {
                // This happens when we get 2 END characters in a row, which is
                // a totally empty packet, which we ignore.
                return Packet::Error::NOT_DONE;
            }
            this->m_escape = false;
            this->m_packet->setCommand(byte);
            this->m_packet->setData(0, nullptr);
            this->m_state = State::DATA;
            return Packet::Error::NOT_DONE;
        }

        case State::DATA: {
            if (byte == Packet::END && !this->m_escape) {
                // A regular END marks the beginning/end of a packet.
                if (this->m_packet->getDataLength() == 0) {
                    // Minimum packet requires a Cmd and CRC
                    return Packet::Error::TOO_SMALL;
                }

                uint8_t rcvdCrc = this->m_packet->extractCrc();
                uint8_t expectedCrc = this->m_packet->calcCrc();
                if (rcvdCrc == expectedCrc) {
                    this->m_state = State::IDLE;
                    if (this->m_debug) {
                        this->m_packet->dump("Rcvd");
                    }
                    return Packet::Error::NONE;
                }
                Log::error(
                    "CRC Error: Received 0x%02" PRIx8 " Expected 0x%02" PRIx8, rcvdCrc,
                    expectedCrc);
                if (this->m_debug) {
                    this->m_packet->dump("CRC ");
                }
                return Packet::Error::CRC;
            }
            this->m_escape = false;

            if (this->m_packet->getDataLength() >= this->m_packet->getMaxDataLength()) {
                // Not enough room to store any more bytes.
                if (this->m_debug) {
                    this->m_packet->dump("2Big");
                }
                return Packet::Error::TOO_MUCH_DATA;
            }
            this->m_packet->appendByte(byte);
            return Packet::Error::NOT_DONE;
        }
    }
    return Packet::Error::BAD_STATE;
}

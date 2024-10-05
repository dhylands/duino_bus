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
 *   @file   PacketEncoder.cpp
 *
 *   @brief  Encodes packets into "over the wire" format.
 *
 ****************************************************************************/

#include "Packet.h"

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <memory>

#include "Crc8.h"
#include "DumpMem.h"
#include "Log.h"
#include "PacketEncoder.h"

void PacketEncoder::encodeStart(Packet* packet) {
    packet->calcAndStoreCrc();
    this->m_packet = packet;

    if (this->m_debug) {
        this->m_packet->dump("Sent");
    }

    this->m_state = State::IDLE;
}

PacketEncoder::State PacketEncoder::handleEscape(uint8_t* byte) {
    if (*byte == Packet::END) {
        this->m_escapeChar = Packet::ESC_END;
        *byte = Packet::ESC;
        return State::ESCAPE;
    }
    if (*byte == Packet::ESC) {
        this->m_escapeChar = Packet::ESC_ESC;
        *byte = Packet::ESC;
        return State::ESCAPE;
    }
    return State::DATA;
}

Packet::Error PacketEncoder::encodeByte(uint8_t* byte) {
    switch (this->m_state) {
        case State::IDLE: {
            *byte = Packet::END;
            this->m_state = State::COMMAND;
            return Packet::Error::NOT_DONE;
        }

        case State::COMMAND: {
            *byte = this->m_packet->getCommand();
            this->m_state = this->handleEscape(byte);
            this->m_encodeIdx = 0;
            return Packet::Error::NOT_DONE;
        }

        case State::DATA: {
            if (this->m_encodeIdx < this->m_packet->getDataLength()) {
                *byte = this->m_packet->getData()[this->m_encodeIdx++];
                this->m_state = this->handleEscape(byte);
                return Packet::Error::NOT_DONE;
            }
            if (this->m_encodeIdx == this->m_packet->getDataLength()) {
                *byte = this->m_packet->calcCrc();
                this->m_state = this->handleEscape(byte);
                this->m_encodeIdx++;
                return Packet::Error::NOT_DONE;
            }
            *byte = Packet::END;
            this->m_state = State::IDLE;
            return Packet::Error::NONE;
        }

        case State::ESCAPE: {
            *byte = this->m_escapeChar;
            this->m_state = State::DATA;
            return Packet::Error::NOT_DONE;
        }
    }
    return Packet::Error::BAD_STATE;
}

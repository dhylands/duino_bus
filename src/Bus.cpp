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
 *   @file   Bus.cpp
 *
 *   @brief  Abstract base class for a bus.
 *
 ****************************************************************************/

#include "Bus.h"
#include "Log.h"
#include "PacketHandler.h"

IBus::IBus(
    Packet* cmdPacket,  //!< [mod] Place to store the command packet.
    Packet* rspPacket   //!< [mod] Place to store the response packet.
    )
    : m_cmdPacket{cmdPacket}, m_rspPacket{rspPacket}, m_decoder{cmdPacket} {};

Packet::Error IBus::processByte() {
    uint8_t byte;
    if (!this->readByte(&byte)) {
        return Error::NOT_DONE;
    }
    return this->m_decoder.decodeByte(byte);
}

Packet::Error IBus::writePacket(Packet* packet) {
    this->m_encoder.encodeStart(packet);
    uint8_t byte;
    Error err = Error::NOT_DONE;
    while (err == Error::NOT_DONE) {
        err = this->m_encoder.encodeByte(&byte);
        this->writeByte(byte);
    }
    return err;
}

void IBus::add(IPacketHandler& handler) {
    this->m_handlers.push_back(&handler);
}

bool IBus::handlePacket() {
    this->m_rspPacket->setCommand(0);
    this->m_rspPacket->setData(0, nullptr);
    for (auto handler : this->m_handlers) {
        if (handler->handlePacket(*this->m_cmdPacket, this->m_rspPacket)) {
            if (this->m_rspPacket->getCommand() != 0) {
                this->writePacket(this->m_rspPacket);
            }
            return true;
        }
    }
    Log::error("Unhandled command: 0x%02" PRIx8, this->m_cmdPacket->getCommand());
    return false;
}

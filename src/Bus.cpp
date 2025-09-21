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

#include "duino_bus/Bus.h"
#include "duino_bus/PacketHandler.h"
#include "duino_log/Log.h"

IBus::IBus(Packet* cmdPacket, Packet* rspPacket, Packet* logPacket, Packet* evtPacket)
    : m_cmdPacket{cmdPacket},
      m_rspPacket{rspPacket},
      m_logPacket{logPacket},
      m_evtPacket{evtPacket},
      m_decoder{this, cmdPacket},
      m_encoder{this} {};

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
    this->flush();
    return err;
}

void IBus::add(IPacketHandler& handler) {
    handler.setBus(this);
    this->m_handlers.push_back(&handler);
}

bool IBus::handlePacket() {
    this->m_rspPacket->setCommand(0);
    this->m_rspPacket->setData(0, nullptr);
    for (auto handler : this->m_handlers) {
        if (handler->handlePacket(*this->m_cmdPacket, this->m_rspPacket)) {
            if (this->m_rspPacket->getCommand() != 0) {
                this->writePacket(this->m_rspPacket);
            } else if (this->m_rspPacket->getDataLength() > 0) {
                Log::error("Packet data set, but no command");
            }
            return true;
        }
    }
    Log::error("Unhandled command: 0x%02" PRIx8, this->m_cmdPacket->getCommand());
    return false;
}

char const* IBus::as_str(Packet::Command::Type cmd) const {
    for (auto handler : this->m_handlers) {
        auto str = handler->as_str(cmd);
        if (str != nullptr && *str != '?') {
            return str;
        }
    }
    return "???";
}

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
 *   @file   CorePacketHandler.cpp
 *
 *   @brief  Handles packets for core commands.
 *
 ****************************************************************************/

#include "duino_bus/CorePacketHandler.h"
#include "duino_bus/Unpacker.h"

char const* CorePacketHandler::as_str(Packet::Command::Type cmd) const {
    switch (cmd) {
        case Command::PING:
            return "PING";
        case Command::DEBUG:
            return "DEBUG";
        case Command::LOG:
            return "LOG";
    }
    return "???";
}

bool CorePacketHandler::handlePacket(Packet const& cmd, Packet* rsp) {
    switch (cmd.getCommand()) {
        case Command::PING: {
            this->handlePing(cmd, rsp);
            return true;
        }
        case Command::DEBUG: {
            this->handleDebug(cmd, rsp);
            return true;
        }
    }
    return false;
}

void CorePacketHandler::handleDebug(Packet const& cmd, Packet* rsp) {
    Unpacker unpacker(cmd);
    DebugFlags flags;
    unpacker.unpack(&flags);

    this->m_bus->setDebug((flags & 0x01) != 0);

    rsp->setCommand(Command::DEBUG);
    rsp->append(flags);
}

void CorePacketHandler::handlePing(Packet const& cmd, Packet* rsp) {
    rsp->setCommand(Command::PING);
    // We echo back any data which is included in the PING command.
    rsp->setData(cmd.getDataLength(), cmd.getData());
}

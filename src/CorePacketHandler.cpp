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

char const* CorePacketHandler::as_str(Packet::Command::Type cmd) const {
    switch (cmd) {
        case Command::PING:
            return "PING";
    }
    return "???";
}

bool CorePacketHandler::handlePacket(Packet const& cmd, Packet* rsp) {
    switch (cmd.getCommand()) {
        case Command::PING: {
            this->handlePing(cmd, rsp);
            return true;
        }
    }
    return false;
}

void CorePacketHandler::handlePing(Packet const& cmd, Packet* rsp) {
    rsp->setCommand(Command::PING);
    // We echo back any data which is included in the PING command.
    rsp->setData(cmd.getDataLength(), cmd.getData());
}

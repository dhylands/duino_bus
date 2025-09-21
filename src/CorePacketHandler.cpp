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

#include <malloc.h>

#include "duino_bus/Unpacker.h"
#include "duino_util/HeapMonitor.h"
#include "duino_util/StackMonitor.h"

char const* CorePacketHandler::as_str(Packet::Command::Type cmd) const {
    switch (cmd) {
        case Command::PING:
            return "PING";
        case Command::DEBUG:
            return "DEBUG";
        case Command::LOG:
            return "LOG";
        case Command::STACK_INFO:
            return "STACK_INFO";
        case Command::HEAP_INFO:
            return "HEAP_INFO";
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
        case Command::STACK_INFO: {
            this->handleStackInfo(cmd, rsp);
            return true;
        }
        case Command::HEAP_INFO: {
            this->handleHeapInfo(cmd, rsp);
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

void CorePacketHandler::handleHeapInfo(Packet const&, Packet* rsp) {
    rsp->setCommand(Command::HEAP_INFO);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    struct mallinfo info = mallinfo();
#pragma GCC diagnostic pop
    // unittest compiles this file.
#if defined(__ARM_ARCH)
    auto growthPotential = getHeapGrowthPotential();
#else
    size_t growthPotential = 0;
#endif

    rsp->append(static_cast<uint32_t>(info.arena));
    rsp->append(static_cast<uint32_t>(info.uordblks));
    rsp->append(static_cast<uint32_t>(info.fordblks));
    rsp->append(static_cast<uint32_t>(info.ordblks));
    rsp->append(static_cast<uint32_t>(growthPotential));
}

void CorePacketHandler::handlePing(Packet const& cmd, Packet* rsp) {
    rsp->setCommand(Command::PING);
    // We echo back any data which is included in the PING command.
    rsp->setData(cmd.getDataLength(), cmd.getData());
}

void CorePacketHandler::handleStackInfo(Packet const&, Packet* rsp) {
    rsp->setCommand(Command::STACK_INFO);

#if defined(__ARM_ARCH)
    rsp->append(static_cast<uint32_t>(getStackSize()));
    rsp->append(static_cast<uint32_t>(getUsedStackSpace()));
    rsp->append(static_cast<uint32_t>(getUnusedStackSpace()));
#else
    rsp->append(static_cast<uint32_t>(0));
    rsp->append(static_cast<uint32_t>(0));
    rsp->append(static_cast<uint32_t>(0));
#endif
}

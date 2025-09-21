/****************************************************************************
 *
 *   @copyright Copyright (c) 2025 Dave Hylands     <dhylands@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the MIT License version as described in the
 *   LICENSE file in the root of this repository.
 *
 ****************************************************************************/
/**
 *   @file   BusLog.h
 *
 *   @brief  Logger which sends log packets over the bus.
 *
 ****************************************************************************/

#include "duino_bus/BusLog.h"

#include "duino_bus/CorePacketHandler.h"
#include "duino_log/ConsoleColor.h"
#include "duino_util/Util.h"

using Command = CorePacketHandler::Command;

//! Structure used for accumulating the log message into a packet.
struct LogParam {
    Packet* pkt;           //!< Packet to store the log message into.
    size_t bytes_written;  //!< Number of bytes that have been stored so far.
};

size_t BusLog::log_char_to_packet(void* outParam, char ch) {
    LogParam* param = reinterpret_cast<LogParam*>(outParam);
    if (param->pkt->getSpaceRemaining() > 1) {
        param->pkt->appendByte(ch);
        param->bytes_written++;
        return 1;
    }
    return 0;
}

void BusLog::do_log(
    Level level,      //!< Logging level associated with this message.
    const char* fmt,  //!< Printf style format string
    va_list args      //!< Arguments associated with format string.
) {
    Packet* log = this->m_bus->getLogPacket();
    if (log == nullptr) {
        // User didn't provide a log packet to the Bus constructor.
        return;
    }

    log->setCommand(Command::LOG);
    log->setData(0, nullptr);
    log->appendByte(to_underlying(level));

    uint8_t* strLen = log->getWriteData(1);

    LogParam param = {
        .pkt = log,
        .bytes_written = 0,
    };
    size_t bytes_written = vStrXPrintf(BusLog::log_char_to_packet, &param, fmt, args);
    log->appendByte(0);

    *strLen = static_cast<uint8_t>(param.bytes_written + 1);

    this->m_bus->writePacket(log);
}

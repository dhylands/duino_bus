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
 *   @brief  Class which sends log messages over a Bus.
 *
 ****************************************************************************/

#pragma once

#include "duino_bus/Bus.h"
#include "duino_log/Log.h"

//! Implements logging by sending logging messages over the bus,
class BusLog : public Log {
 public:
    //! Constructor.
    explicit BusLog(
        IBus* bus  //!< [in] Bus to send messages over.
        )
        : Log(), m_bus{bus} {}

 protected:
    //! Implements the actual logging function.
    void do_log(
        Level level,      //!< Logging level associated with this message.
        const char* fmt,  //!< Printf style format string
        va_list args      //!< Arguments associated with format string.
        ) override;

 private:
    //! Function called from vStrXPrintf which appends a single character to the log packet.
    //! @returns 1 if the character was logged successzfully, 0 otherwise.
    static size_t log_char_to_packet(
        void* outParam,  //!< Pointer to ArduinoSerialLogger object.
        char ch          //!< Character to output.
    );

    IBus* m_bus;  //!< Bus to send the log packets on.
};

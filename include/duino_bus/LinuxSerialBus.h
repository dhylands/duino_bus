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
 *   @file   LinuxSerialBus.h
 *
 *   @brief  Implements a bus for sending packets using a linux serial port.
 *
 ****************************************************************************/

#pragma once

#if !defined(ARDUINO)

#include <cinttypes>

#include "Bus.h"

//! Implements a bus using an Arduino Serial port.
class LinuxSerialBus : public IBus {
 public:
    //! Constructor.
    LinuxSerialBus(
        Packet* cmdPacket,  //!< [in] Place to store command packet
        Packet* rspPacket   //!< [in] Place to store response packet
    );

    //! Destructor.
    ~LinuxSerialBus() override;

    //! Opens the named serial port.
    //! @returns Error::NONE if everything went ok, or an error code otherwise.
    Error open(
        char const* portName,  //!< [in] Name of serial port to open.
        uint32_t baudRate      //!< [in] Baud rate to use.
    );

    //! @returns the file descriptor associated with the serial port.
    int serial() { return this->m_serial; }

    bool isDataAvailable() const override;
    bool readByte(uint8_t* byte) override;
    bool isSpaceAvailable() const override;
    void writeByte(uint8_t byte) override;

 private:
    char const* m_portName = nullptr;  //!< Name of serial port to use.
    int m_serial = -1;                 //!< Serial port file descriptor.
};

#endif  // !defined(ARDUINO)

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
 *   @file   ArduinoSerialBus.h
 *
 *   @brief  Implements a bus for sending packets via an Arduino serial port.
 *
 ****************************************************************************/

#pragma once

#include <Arduino.h>

#include <cinttypes>

#include "duino_bus/Bus.h"

//! Implements a bus using an Arduino Serial port.
class ArduinoSerialBus : public IBus {
 public:
    //! Constructor.
    ArduinoSerialBus(
        HardwareSerial* serial,  //!< [in] Serial port to use.
        Packet* cmdPacket,       //!< [in] Place to store command packet
        Packet* rspPacket        //!< [in] Place to store response packet
    );

    //! Destructor.
    ~ArduinoSerialBus() override;

    bool isDataAvailable() const override;
    bool readByte(uint8_t* byte) override;
    bool isSpaceAvailable() const override;
    void writeByte(uint8_t byte) override;

 private:
    HardwareSerial* const m_serial;  //!< Serial port to use.
};

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
 *   @file   ArduinoSerialBus.cpp
 *
 *   @brief  Implements a bus for sending packets via an Arduino serial port.
 *
 ****************************************************************************/

#include "ArduinoSerialBus.h"

ArduinoSerialBus::ArduinoSerialBus(HardwareSerial* serial, Packet* cmdPacket, Packet* rspPacket)
    : IBus{cmdPacket, rspPacket}, m_serial{serial} {}

ArduinoSerialBus::~ArduinoSerialBus() {}

bool ArduinoSerialBus::isDataAvailable() const {
    return this->m_serial->available() > 0;
}

bool ArduinoSerialBus::readByte(uint8_t* byte) {
    int byteRead = this->m_serial->read();
    if (byteRead < 0) {
        return false;
    }
    *byte = static_cast<uint8_t>(byteRead);
    return true;
}

bool ArduinoSerialBus::isSpaceAvailable() const {
    return this->m_serial->availableForWrite() > 0;
}

void ArduinoSerialBus::writeByte(uint8_t byte) {
    this->m_serial->write(byte);
}

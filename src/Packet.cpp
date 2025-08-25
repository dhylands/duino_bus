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
 *   @file   Packet.cpp
 *
 *   @brief  Container for a packet of data.
 *
 ****************************************************************************/

#include "duino_bus/Packet.h"

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <memory>

#include "duino_bus/Bus.h"
#include "duino_log/Log.h"
#include "duino_log/DumpMem.h"
#include "duino_util/Crc8.h"

char const* as_str(Packet::Error err) {
    switch (err) {
        case Packet::Error::NONE:
            return "NONE";
        case Packet::Error::NOT_DONE:
            return "NOT_DONE";
        case Packet::Error::CRC:
            return "CRC";
        case Packet::Error::TIMEOUT:
            return "TIMEOUT";
        case Packet::Error::TOO_MUCH_DATA:
            return "TOO_MUCH_DATA";
        case Packet::Error::TOO_SMALL:
            return "TOO_SMALL";
        case Packet::Error::BAD_STATE:
            return "BAD_STATE";
        case Packet::Error::OS:
            return "OS";
    }
    return "???";
}

Packet::Packet(size_t maxDataLen, void* data)
    : m_command{0}, m_maxDataLen(maxDataLen), m_data{reinterpret_cast<uint8_t*>(data)} {}

void Packet::dump(char const* label, IBus const* bus) const {
    Command cmd{this->getCommand()};
    char const* cmd_str = (bus == nullptr) ? "???" : bus->as_str(cmd.value);
    Log::info(
        "%s: Command: 0x%02" PRIx8 " (%s) Len: %zu CRC: 0x%02" PRIx8, label, cmd.value, cmd_str,
        this->getDataLength(), this->getCrc());
    DumpMem(label, 0, this->m_data, this->getDataLength());
}

void Packet::setData(size_t dataLen, void const* data) {
    this->m_dataLen = 0;
    this->appendData(dataLen, data);
}

void Packet::appendByte(uint8_t byte) {
    assert(this->getDataLength() < this->getMaxDataLength());
    this->m_data[this->m_dataLen++] = byte;
}

void Packet::appendData(size_t dataLen, void const* data) {
    assert(this->getDataLength() + dataLen <= this->getMaxDataLength());
    if (dataLen > 0) {
        memcpy(&this->m_data[this->m_dataLen], data, dataLen);
        this->m_dataLen += dataLen;
    }
}

void Packet::append(char const* str) {
    // Get the length (including the terminating null)
    uint8_t strLength = strnlen(str, 256) + 1;

    // Store the length
    this->append(strLength);

    // Copy the string including the terminating null character.
    this->appendData(strLength, str);
}

uint8_t Packet::getCrc() const {
    return this->m_crc;
}

uint8_t Packet::calcCrc() const {
    uint8_t expectedCrc = Crc8(0, this->m_command.value);
    expectedCrc = Crc8(expectedCrc, this->getDataLength(), this->getData());
    return expectedCrc;
}

uint8_t Packet::extractCrc() {
    assert(this->m_dataLen >= 1);
    // When receiving data we don't know the length ahead of time, so the
    // CRC will be the last byte of the data. We remove it from the data
    // and store it in the crc field.
    this->m_dataLen--;
    this->m_crc = this->m_data[this->m_dataLen];
    return this->m_crc;
}

void Packet::calcAndStoreCrc() {
    this->m_crc = this->calcCrc();
}

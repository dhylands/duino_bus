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
 *   @file   Unpacker.cpp
 *
 *   @brief  Utility class for unpacking variable length data.
 *
 ****************************************************************************/

#include <algorithm>

#include "Unpacker.h"

Unpacker::Unpacker() {}

Unpacker::Unpacker(Packet const& packet)
    : m_data{packet.getData()}, m_dataRemaining{packet.getDataLength()} {}

void Unpacker::setData(Packet const& packet) {
    this->m_data = packet.getData();
    this->m_dataRemaining = packet.getDataLength();
}

bool Unpacker::unpack(size_t numBytes, uint8_t const** data) {
    if (this->m_dataRemaining < numBytes) {
        return false;
    }
    *data = this->m_data;
    this->m_data += numBytes;
    this->m_dataRemaining -= numBytes;
    return true;
}

bool Unpacker::unpack(char const** str) {
    // Get the length
    uint8_t strLength;
    if (!this->unpack(&strLength)) {
        return false;
    }

    if (this->m_dataRemaining < strLength) {
        return false;
    }
    *str = reinterpret_cast<char const*>(this->m_data);

    this->m_data += strLength;
    this->m_dataRemaining -= strLength;

    return true;
}

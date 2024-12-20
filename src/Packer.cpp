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
 *   @file   Packer.cpp
 *
 *   @brief  Utility class for packing variable length data.
 *
 ****************************************************************************/

#include <algorithm>

#include "Packer.h"

Packer::Packer(Packet* packet) : m_packet(packet) {}

bool Packer::pack(char const* str) {
    // Get the length (including the terminating null)
    uint8_t strLength = strnlen(str, 256) + 1;

    // Store the length
    if (!this->pack(strLength)) {
        return false;
    }

    if (this->m_packet->getSpaceRemaining() < strLength) {
        return false;
    }

    // Copy the string including the terminating null character.
    this->m_packet->appendData(strLength, str);

    return true;
}

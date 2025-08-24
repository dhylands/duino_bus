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
 *   @file   Packer.h
 *
 *   @brief  Utility class for packing variable length data.
 *
 ****************************************************************************/

#pragma once

#include <cassert>
#include <cstring>

#include "Packet.h"

//! Class for packing variable length data from a packet.
class Packer {
 public:
    //! Constructor.
    explicit Packer(
        Packet* packet  //!< [mod] Packet to pack into.
    );

    //! Pack a simple type.
    //! @tparam T type of value to pack.
    //! @returns true if the data was packed successfully, false otherwise.
    template <typename T>
    bool pack(
        T const& data  //!< [in] Data to pack.
    ) {
        // Produce an error if somebody tries to pass a pointer.
        static_assert(!std::is_pointer_v<T>);

        if (this->m_packet->getSpaceRemaining() < sizeof(data)) {
            return false;
        }
        this->m_packet->appendData(sizeof(data), &data);
        return true;
    }

    //! Packs a string.
    //! @returns true if the string was packed successfully, false otherwise.
    bool pack(
        char const* str  //!< [in] String to append.
    );

 private:
    Packet* m_packet;
};

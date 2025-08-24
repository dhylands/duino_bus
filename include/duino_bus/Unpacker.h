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
 *   @file   Unpacker.h
 *
 *   @brief  Utility class for unpacking variable length data.
 *
 ****************************************************************************/

#pragma once

#include <cstring>

#include "Packet.h"

//! Class for unpacking variable length data from a packet.
class Unpacker {
 public:
    //! Default Constructor.
    Unpacker();

    //! Constructor.
    explicit Unpacker(Packet const& packet  //!< [in] Packet to unpack.
    );

    //! Sets the data from a packet.
    void setData(Packet const& packet  //!< [in] Packet data to unpack.
    );

    //! Unpack a simple type.
    //! @tparam T type of value to unpack.
    //! @returns true if the data was unpacked successfully, false otherwise.
    template <typename T>
    bool unpack(T* data  //!< [out] Place to store extracted data.
    ) {
        // Produce an error if somebody tries to pass a pointer.
        static_assert(!std::is_pointer_v<T>);

        if (this->m_dataRemaining < sizeof(*data)) {
            return false;
        }
        memcpy(data, this->m_data, sizeof(*data));
        this->m_data += sizeof(*data);
        this->m_dataRemaining -= sizeof(*data);
        return true;
    }

    //! Unpacks arbitrary data.
    //! This function will populate data to point to the data inside the packet.
    //! The caller should copy the data out, if required.
    //! @returns true if the string was unpacked successfully, false otherwise.
    bool unpack(
        size_t numBytes,      //!< [in] Number of bytes to unpack.
        uint8_t const** data  //!< [out] Place to store pointer to data.
    );

    //! Unpacks a string.
    //! This function wil populate str to point to the string inside the packet.
    //! The caller should copy the data out, if required.
    //! @returns true if the string was unpacked successfully, false otherwise.
    bool unpack(char const** str  //!< [out] Place to store extracted string.
    );

 private:
    uint8_t const* m_data = nullptr;
    size_t m_dataRemaining = 0;
};

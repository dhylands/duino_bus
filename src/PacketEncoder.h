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
 *   @file   PacketEncoder.h
 *
 *   @brief  Encodes packets into "over the wire" format.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>
#include <initializer_list>

#include "Packet.h"
#include "Util.h"

//! Forward declaration.
//@{
class PacketEncoderTest_BadStateTest_Test;
//@}

//! Code for encoding a packet into its raw byte stream.
class PacketEncoder {
 public:
    //! Resets the encoder to start encoding a packet.
    void encodeStart(Packet* packet  //!< [in/out] Packet to encode (CRC is modified)
    );

    //! Encodes the next byte of the packet.
    //! @returns Error::NONE if the packet encoding has been completed.
    //! @returns Error::NOT_DONE if packet encoding is incomplete.
    Packet::Error encodeByte(uint8_t* byte  //!< [out] Place to store the next encoded byte.
    );

    //! Sets the debug flag which controls whether decoded packets get dumped.
    void setDebug(bool debug  //!< [in] Value to set debug flag to.
    ) {
        this->m_debug = debug;
    }

 private:
    //! This allows the TEST(PacketTest, BadState) function to access m_state
    friend class ::PacketEncoderTest_BadStateTest_Test;

    enum class State {
        IDLE,     //!< Haven't started parsing a packet yet.
        COMMAND,  //!< Encoding the command.
        DATA,     //!< Encoding the data portion of the packet.
        ESCAPE,   //!< Encoding an escape character.
    };

    State handleEscape(uint8_t* byte);

    Packet const* m_packet = nullptr;  //!< Packet being encoded.
    State m_state = State::IDLE;       //!< State of the parser.
    size_t m_encodeIdx = 0;            //!< Data byte being encoded.
    uint8_t m_escapeChar;              //!< Character being escaped.
    bool m_debug = false;              //!< Print packets encoded?
};

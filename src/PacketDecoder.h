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
 *   @file   PacketDecoder.h
 *
 *   @brief  Decodes packets from their "over the wire" format.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>
#include <initializer_list>

#include "duino_util.h"
#include "Packet.h"

//! Forward declaration.
//@{
class PacketDecoderTest_BadStateTest_Test;
//@}

//! Code for decoding a raw byte stream into a packet.
class PacketDecoder {
 public:
    //! Constructor
    explicit PacketDecoder(Packet* pkt  //!< [out] Place to store decoded packet.
    );

    //! Runs a single byte through the packet decoder state machine.
    //! @returns Error::NONE if the packet was parsed successfully.
    //! @returns Error::NOT_DONE if the packet is incomplete.
    //! @returns Error::CRC if a CRC error was encountered.
    //! @returns Error::TOO_MUCH_DATA if the data doesn't fit into the packet data.
    Packet::Error decodeByte(uint8_t byte  //!< [in] Byte to parse.
    );

    //! Sets the debug flag which controls whether decoded packets get dumped.
    void setDebug(bool debug  //!< [in] Value to set debug flag to.
    ) {
        this->m_debug = debug;
    }

 private:
    //! This allows the TEST(PacketTest, BadState) function to access m_state
    friend class ::PacketDecoderTest_BadStateTest_Test;

    enum class State {
        IDLE,     //!< Haven't started parsing a packet yet.
        COMMAND,  //!< Parsing the command.
        DATA,     //!< Parsing the data.
    };

    Packet* m_packet;             //!< Packet being decoded.
    State m_state = State::IDLE;  //!< State of the parser.
    bool m_escape = false;        //!< Are we escaping a byte?
    bool m_debug = false;         //!< Print packets decoded?
};

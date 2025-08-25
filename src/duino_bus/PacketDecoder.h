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

#include "duino_bus/Packet.h"
#include "duino_util/Util.h"

//! Forward declaration.
//@{
class IBus;
class PacketDecoderTest_BadStateTest_Test;
//@}

//! Code for decoding a raw byte stream into a packet.
class PacketDecoder {
 public:
    //! Constructor
    PacketDecoder(
        IBus const* bus,  //!< [in] Bus this decoder is associated with.
        Packet* pkt       //!< [out] Place to store decoded packet.
    );

    //! Runs a single byte through the packet decoder state machine.
    //! @returns Error::NONE if the packet was parsed successfully.
    //! @returns Error::NOT_DONE if the packet is incomplete.
    //! @returns Error::CRC if a CRC error was encountered.
    //! @returns Error::TOO_MUCH_DATA if the data doesn't fit into the packet data.
    Packet::Error decodeByte(
        uint8_t byte  //!< [in] Byte to parse.
    );

    //! Sets the debug flag which controls whether decoded packets get dumped.
    void setDebug(
        bool debug  //!< [in] Value to set debug flag to.
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

    IBus const* m_bus;            //!< Bus this packet decoder is associated wiith
    Packet* m_packet;             //!< Packet being decoded.
    State m_state = State::IDLE;  //!< State of the parser.
    bool m_escape = false;        //!< Are we escaping a byte?
    bool m_debug = false;         //!< Print packets decoded?
};

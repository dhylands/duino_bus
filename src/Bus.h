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
 *   @file   Bus.h
 *
 *   @brief  Abstract base class for a bus.
 *
 ****************************************************************************/

#pragma once

#include <cinttypes>
#include <vector>

#include "Packet.h"
#include "PacketDecoder.h"
#include "PacketEncoder.h"

class IPacketHandler;  //!< Forward reference

//! Abstract base class for a bus.
//! A bus is used for interfacing with the underlying hardware, (TCP/IP socket, serial, etc).
class IBus {
 public:
    using Error = Packet::Error;  //!< Convenience alias.

    //! Constructor.
    explicit IBus(
        Packet* cmdPacket,  //!< [mod] Place to store the command packet.
        Packet* rspPacket   //!< [mod] Place to store the response packet.
    );

    //! Destructor.
    virtual ~IBus() = default;

    //! @returns true if data is available to be received, false otherwise.
    virtual bool isDataAvailable() const = 0;

    //! Reads a byte from the bus (or other virtual device).
    //! This function is non-blocking.
    //! @returns true if a byte was read, false otherwise.
    virtual bool readByte(uint8_t* byte  //!< [out] Place to store byte that was read.
                          ) = 0;

    //! @returns true if space is available to write another byte, false otherwise.
    virtual bool isSpaceAvailable() const = 0;

    //! Writes a byte to the bus.
    virtual void writeByte(uint8_t byte  //!< [in] byte to write.
                           ) = 0;

    //! Reads a byte from the bus, and runs it through the packet parser.
    //! @returns Error::NONE if the packet was parsed successfully.
    //! @returns Error::NOT_DONE if the packet is incomplete.
    //! @returns Error::CRC if a CRC error was encountered.
    //! @returns Error::TOO_MUCH_DATA if the data doesn't fit into the packet data.
    Error processByte();

    //! Writes a packet on this bus.
    //! @returns Error::NONE if the packet was written successfully, or an error code otherwise.
    Error writePacket(Packet* packet  //!< [in] Packet to write.
    );

    //! Sets the debug flag which controls whether decoded packets get dumped.
    void setDebug(bool debug  //!< [in] Value to set debug flag to.
    ) {
        this->m_decoder.setDebug(debug);
        this->m_encoder.setDebug(debug);
    }

    //! Adds a packet handler.
    void add(IPacketHandler& handler  //!< Packet handler to add.
    );

    //! Runs the received packet through the registered handlers.
    //! @returns true if the packet was handled, false otherwise.
    bool handlePacket();

 protected:
    Packet* m_cmdPacket;                      //!< Place to store the incoming command packet.
    Packet* m_rspPacket;                      //!< Place to store the outcoming response packet.
    PacketDecoder m_decoder;                  //!< Used to decode incoming packets.
    PacketEncoder m_encoder;                  //!< Used to encode outgoing packets.
    std::vector<IPacketHandler*> m_handlers;  //!< Registered packet handlers.
};

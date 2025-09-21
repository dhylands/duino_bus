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
 *   @file   CorePacketHandler.h
 *
 *   @brief  Handles packets for core commands.
 *
 ****************************************************************************/

#pragma once

#include "PacketHandler.h"

//! Packet handler for dealing with core commands.
class CorePacketHandler : public IPacketHandler {
 public:
    //! Commands accepted by the Core packet handler.
    struct Command : public Packet::Command {
        static constexpr Type PING = 0x01;        //!< Check to see if the board is aliave.
        static constexpr Type DEBUG = 0x02;       //!< Sets debug setting
        static constexpr Type LOG = 0x03;         //!< Log message (to host)
        static constexpr Type STACK_INFO = 0x04;  //!< Returns stack information.
        static constexpr Type HEAP_INFO = 0x05;   //!< Returns heap information.
    };

    //! Flags passed to DEBUG message.
    //! Currently just a 0/1 but could become a bit mask.
    using DebugFlags = uint32_t;

    bool handlePacket(Packet const& cmd, Packet* rsp) override;

    char const* as_str(Packet::Command::Type cmd) const override;

 protected:
    //! Handles DEBUG command.
    void handleDebug(
        Packet const& cmd,  //!< [in] Ping packet.
        Packet* rsp         //!< [in] Place to store ping response.
    );

    //! Handles the HEAP_INFO command.
    //! Command:
    //!     No Data
    //! Response:
    //!     uint32_t heapSize
    //!     uint32_t heapAllocated
    //!     uint32_t heapFree
    //!     uint32_t heapLargestFree
    //!     uint32_t heapGrowthPotential
    void handleHeapInfo(
        Packet const& cmd,  //!< [in] Ping packet.
        Packet* rsp         //!< [in] Place to store ping response.
    );

    //! Handles PING command.
    void handlePing(
        Packet const& cmd,  //!< [in] Ping packet.
        Packet* rsp         //!< [in] Place to store ping response.
    );

    //! Handles the STACK_INFO command.
    //! Command:
    //!     No Data
    //! Response:
    //!     uint32_t stackSize
    //!     uint32_t stackUsed
    //!     uint32_t stackUnused
    void handleStackInfo(
        Packet const& cmd,  //!< [in] Ping packet.
        Packet* rsp         //!< [in] Place to store ping response.
    );
};

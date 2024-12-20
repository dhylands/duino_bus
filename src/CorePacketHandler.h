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
        static constexpr Type PING = 0x01;  //!< Check to see if the board is aliave.
    };

    bool handlePacket(Packet const& cmd, Packet* rsp) override;

    char const* as_str(Packet::Command::Type cmd) const override;

    //! Handles PING command
    void handlePing(
        Packet const& cmd,  //!< [in] Ping packet.
        Packet* rsp         //!< [in] Place to store ping response.
    );
};

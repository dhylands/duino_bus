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
 *   @file   CorePacketHandlerTest.cpp
 *
 *   @brief  Tests for the PacketEncoder class.
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include "duino_bus/CorePacketHandler.h"
#include "duino_bus/Packet.h"

using Command = Packet::Command;  //!< Convenience alias
using Error = Packet::Error;      //!< Convenience alias

//! Test data used by individual tests.
struct TestData {
    //! Constructor.
    TestData() : m_cmdPacket{LEN(m_cmdData), m_cmdData}, m_rspPacket{LEN(m_rspData), m_rspData} {}

    uint8_t m_cmdData[16];        //!< Storage for command packet data.
    uint8_t m_rspData[16];        //!< Storage for response packet data.
    Packet m_cmdPacket;           //!< Command packet.
    Packet m_rspPacket;           //!< Response packet.
    CorePacketHandler m_handler;  //!< Packet handler.
};

TEST(CorePacketHandlerTest, PingTest) {
    TestData test;
    test.m_cmdPacket.setCommand(Command::PING);
    EXPECT_EQ(test.m_handler.handlePacket(test.m_cmdPacket, &test.m_rspPacket), true);
    EXPECT_EQ(test.m_rspPacket.getCommand(), Command::PING);
    EXPECT_EQ(test.m_rspPacket.getDataLength(), 0);
}

TEST(CorePacketHandlerTest, UnhandledTest) {
    TestData test;
    test.m_cmdPacket.setCommand(0xff);
    EXPECT_EQ(test.m_handler.handlePacket(test.m_cmdPacket, &test.m_rspPacket), false);
}

TEST(CorePacketHandlerTest, as_strTest) {
    CorePacketHandler handler;

    EXPECT_STREQ(handler.as_str(CorePacketHandler::Command::PING), "PING");
    EXPECT_STREQ(handler.as_str(0), "???");
}

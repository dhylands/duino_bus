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
 *   @file   PacketTest.cpp
 *
 *   @brief  Tests for the Packet class.
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include "AsciiHex.h"
#include "Crc8.h"
#include "DumpMem.h"
#include "PacketDecoder.h"
#include "Util.h"

//! Convenience alias.
//!@{
using Command = Packet::Command;
using Error = Packet::Error;
//!@}

TEST(PacketTest, AsStrTest) {
    EXPECT_STREQ(as_str(Error::NONE), "NONE");
    EXPECT_STREQ(as_str(Error::NOT_DONE), "NOT_DONE");
    EXPECT_STREQ(as_str(Error::CRC), "CRC");
    EXPECT_STREQ(as_str(Error::TIMEOUT), "TIMEOUT");
    EXPECT_STREQ(as_str(Error::TOO_MUCH_DATA), "TOO_MUCH_DATA");
    EXPECT_STREQ(as_str(Error::TOO_SMALL), "TOO_SMALL");
    EXPECT_STREQ(as_str(Error::BAD_STATE), "BAD_STATE");
    EXPECT_STREQ(as_str(Error::OS), "OS");
    EXPECT_STREQ(as_str(static_cast<Error>(0xff)), "???");
}

TEST(PacketTest, DumpTest) {
    // This is really for coverage
    uint8_t pktData[16];
    Packet pkt(LEN(pktData), pktData);
    pkt.setCommand(Command::PING);
    uint8_t data[] = {0x11, 0x22, 0x33};
    pkt.setData(LEN(data), data);
    pkt.dump("Test");
}

TEST(PacketTest, UnrecognizedCommandTest) {
    // This is really for coverage
    uint8_t pktData[16];
    Packet pkt(LEN(pktData), pktData);
    pkt.setCommand(static_cast<Packet::Command::Type>(0xff));
    uint8_t data[] = {0x11, 0x22, 0x33};
    pkt.setData(LEN(data), data);
    pkt.dump("Test");
}

TEST(PacketDeathTest, AppendTooManyBytesTest) {
    uint8_t pktData[4];
    Packet pkt(LEN(pktData), pktData);

    pkt.appendByte('1');
    pkt.appendByte('2');
    pkt.appendByte('3');
    pkt.appendByte('4');
    ASSERT_DEATH(
        { pkt.appendByte('5'); },
        "Assertion `this->getDataLength\\(\\) < this->getMaxDataLength\\(\\)'");
}

TEST(PacketDeathTest, ExtractCrcTest) {
    uint8_t pktData[4];
    Packet pkt(LEN(pktData), pktData);

    ASSERT_DEATH({ pkt.extractCrc(); }, "Assert");
}

TEST(PacketTest, AppendStrTest) {
    uint8_t pktData[16];
    Packet pkt(LEN(pktData), pktData);

    char const* str = "Data";
    ByteBuffer strVec = {0x05, 'D', 'a', 't', 'a', 0x00};

    pkt.append(str);

    EXPECT_EQ(pkt.getDataLength(), 6);

    ByteBuffer pktDataVec(pkt.getData(), pkt.getData() + pkt.getDataLength());

    EXPECT_EQ(strVec, pktDataVec);
}

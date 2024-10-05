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
 *   @file   UnpackerTest.cpp
 *
 *   @brief  Tests for Unpacker class.
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include "AsciiHex.h"
#include "Packet.h"
#include "Unpacker.h"
#include "Util.h"

//! Helper class for Unpacker tests.
class UnpackerTest {
 public:
    //! Constructor.
    explicit UnpackerTest(char const* str  //!< [in] ASCII Hex data to store in packet.
                          )
        : m_hexData{AsciiHexToBinary(str)}, m_packet(LEN(this->m_packetData), this->m_packetData) {
        this->m_packet.appendData(this->m_hexData.size(), this->m_hexData.data());
        this->m_unpacker.setData(this->m_packet);
    }

    ByteBuffer m_hexData;      //!< ASCII Hex version of packet data.
    uint8_t m_packetData[64];  //!< Storage for the packet data.
    Packet m_packet;           //!< Packet we're unpacking data from.
    Unpacker m_unpacker;       //!< Unpacket which unpacks the data.
};

TEST(UnpackerTest, Unpack1Test) {
    UnpackerTest test("11");

    uint8_t data;

    EXPECT_TRUE(test.m_unpacker.unpack(&data));
    EXPECT_EQ(data, 0x11);
    EXPECT_FALSE(test.m_unpacker.unpack(&data));
}

TEST(UnpackerTest, Unpack2Test) {
    UnpackerTest test("11 22");

    uint16_t data;

    EXPECT_TRUE(test.m_unpacker.unpack(&data));
    EXPECT_EQ(data, 0x2211);
    EXPECT_FALSE(test.m_unpacker.unpack(&data));
}

TEST(UnpackerTest, Unpack4Test) {
    UnpackerTest test("11 22 33 44");

    uint32_t data;

    EXPECT_TRUE(test.m_unpacker.unpack(&data));
    EXPECT_EQ(data, 0x44332211);
    EXPECT_FALSE(test.m_unpacker.unpack(&data));
}

TEST(UnpackerTest, UnpackStrTest) {
    UnpackerTest test("04 41 42 43 00");

    char const* str;

    EXPECT_TRUE(test.m_unpacker.unpack(&str));
    EXPECT_STREQ(str, "ABC");

    uint8_t data;
    EXPECT_FALSE(test.m_unpacker.unpack(&data));
}

TEST(UnpackerTest, UnpackNoStrTest) {
    UnpackerTest test("11");

    uint8_t data;

    EXPECT_TRUE(test.m_unpacker.unpack(&data));

    char const* str;

    EXPECT_FALSE(test.m_unpacker.unpack(&str));
}

TEST(UnpackerTest, UnpackMultiTest) {
    UnpackerTest test("11 04 41 42 43 00 44 55");

    uint8_t data8;
    uint16_t data16;
    char const* str;

    EXPECT_TRUE(test.m_unpacker.unpack(&data8));
    EXPECT_EQ(data8, 0x11);
    EXPECT_TRUE(test.m_unpacker.unpack(&str));
    EXPECT_STREQ(str, "ABC");
    EXPECT_TRUE(test.m_unpacker.unpack(&data16));
    EXPECT_EQ(data16, 0x5544);
    EXPECT_FALSE(test.m_unpacker.unpack(&data8));
}

TEST(UnpackerTest, UnpackBadStrTest) {
    UnpackerTest test("04 41 42 43");

    char const* str;

    EXPECT_FALSE(test.m_unpacker.unpack(&str));
}

TEST(UnpackerTest, PacketContstructorTest) {
    UnpackerTest test("11");
    Unpacker unpacker(test.m_packet);

    uint8_t data;

    EXPECT_TRUE(unpacker.unpack(&data));
    EXPECT_EQ(data, 0x11);
    EXPECT_FALSE(unpacker.unpack(&data));
}

TEST(UnpackerTest, UnpackDataTest) {
    UnpackerTest test("41 42 43 00");

    uint8_t const* data;

    debug();
    EXPECT_TRUE(test.m_unpacker.unpack(4, &data));
    EXPECT_EQ(data[0], 0x41);
    EXPECT_EQ(data[1], 0x42);
    EXPECT_EQ(data[2], 0x43);
    EXPECT_EQ(data[3], 0x00);

    uint8_t data8;
    EXPECT_FALSE(test.m_unpacker.unpack(&data8));
}

TEST(UnpackerTest, UnpackNoDataTest) {
    UnpackerTest test("11");

    uint8_t data8;

    EXPECT_TRUE(test.m_unpacker.unpack(&data8));

    uint8_t const* dataStr;
    uint16_t data16;
    uint32_t data32;

    EXPECT_FALSE(test.m_unpacker.unpack(1, &dataStr));
    EXPECT_FALSE(test.m_unpacker.unpack(&data8));
    EXPECT_FALSE(test.m_unpacker.unpack(&data16));
    EXPECT_FALSE(test.m_unpacker.unpack(&data32));
}

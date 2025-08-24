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
 *   @file   PackerTest.cpp
 *
 *   @brief  Tests for Packer class.
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include "duino_log/DumpMem.h"
#include "duino_bus/Packer.h"
#include "duino_bus/Packet.h"
#include "duino_util/AsciiHex.h"
#include "duino_util/Util.h"

//! Helper class for Packer tests.
class PackerTest {
 public:
    //! Constructor.
    PackerTest()
        : m_packet(LEN(this->m_packetData), this->m_packetData), m_packer(&this->m_packet) {}

    //! Tests if the packet data matches the given ascii hex data.
    //! @returns true if the packet data matches, false otherwise.
    bool matches(
        char const* expectedAsciiHexData  //!< [in] ASCII Hex data that should match the
                                          //!< encoded packet.
    ) {
        auto expectedData = AsciiHexToBinary(expectedAsciiHexData);
        if (expectedData.size() != this->m_packet.getDataLength() ||
            memcmp(expectedData.data(), this->m_packet.getData(), expectedData.size()) != 0) {
            DumpMem("Expecting", 0, expectedData.data(), expectedData.size());
            DumpMem("   Packed", 0, this->m_packet.getData(), this->m_packet.getDataLength());
            return false;
        }
        return true;
    }

    uint8_t m_packetData[16];  //!< Storage for packet data.
    Packet m_packet;           //!< Packet we're packing data into.
    Packer m_packer;           //!< Packer that does the packing.
};

TEST(PackerTest, Pack1Test) {
    PackerTest test;

    uint8_t data = 0x11;

    EXPECT_EQ(test.m_packer.pack(data), true);
    EXPECT_TRUE(test.matches("11"));
}

TEST(PackerTest, Pack2Test) {
    PackerTest test;

    uint16_t data = 0x2211;

    EXPECT_TRUE(test.m_packer.pack(data));
    EXPECT_TRUE(test.matches("11 22"));
}

TEST(PackerTest, Pack4Test) {
    PackerTest test;

    uint32_t data = 0x44332211;

    EXPECT_TRUE(test.m_packer.pack(data));
    EXPECT_TRUE(test.matches("11 22 33 44"));
}

TEST(PackerTest, PackStrTest) {
    PackerTest test;

    EXPECT_TRUE(test.m_packer.pack("ABC"));
    EXPECT_TRUE(test.matches("04 41 42 43 00"));
}

TEST(PackerTest, PackMultiTest) {
    PackerTest test;

    uint8_t data8 = 0x11;
    uint16_t data16 = 0x5544;
    uint32_t data32 = 0x66554433;

    EXPECT_TRUE(test.m_packer.pack(data8));
    EXPECT_TRUE(test.m_packer.pack("ABC"));
    EXPECT_TRUE(test.m_packer.pack(data16));
    EXPECT_TRUE(test.m_packer.pack(data32));

    EXPECT_TRUE(test.matches("11 04 41 42 43 00 44 55 33 44 55 66"));
}

TEST(PackerTest, PackStrTooLongTest) {
    PackerTest test;

    EXPECT_FALSE(test.m_packer.pack("123456789 123456"));
}

TEST(PackerTest, PackStrTooLong2Test) {
    PackerTest test;

    // Fill packet (so that storing the length of the string fails)
    uint32_t data = 0;

    EXPECT_TRUE(test.m_packer.pack(data));
    EXPECT_TRUE(test.m_packer.pack(data));
    EXPECT_TRUE(test.m_packer.pack(data));
    EXPECT_TRUE(test.m_packer.pack(data));

    EXPECT_FALSE(test.m_packer.pack("123456789 123456"));
}

TEST(PackerTest, PackDataTooLongTest) {
    PackerTest test;

    uint8_t data = 0;

    EXPECT_TRUE(test.m_packer.pack("123456789 1234"));
    EXPECT_FALSE(test.m_packer.pack(data));
}

TEST(PackerTest, PackData2TooLongTest) {
    PackerTest test;

    uint16_t data = 0;

    EXPECT_TRUE(test.m_packer.pack("123456789 1234"));
    EXPECT_FALSE(test.m_packer.pack(data));
}

TEST(PackerTest, PackData4TooLongTest) {
    PackerTest test;

    uint32_t data = 0;

    EXPECT_TRUE(test.m_packer.pack("123456789 1234"));
    EXPECT_FALSE(test.m_packer.pack(data));
}

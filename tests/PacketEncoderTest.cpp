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
 *   @file   PacketEncoder.cpp
 *
 *   @brief  Tests for the PacketEncoder class.
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include "duino_bus/CorePacketHandler.h"
#include "duino_log/DumpMem.h"
#include "duino_bus/PacketEncoder.h"
#include "duino_log/LinuxColorLog.h"
#include "duino_util/AsciiHex.h"
#include "duino_util/Crc8.h"
#include "duino_util/Util.h"

using Command = CorePacketHandler::Command;
using Error = Packet::Error;

//! Helper class for creating test cases.
class PacketEncoderTest {
 public:
    //! Constructor.
    PacketEncoderTest(
        Command::Type cmd,  //!< [in] Command to include in the packet.
        char const* str,    //!< [in] ASCII Hex version of packet data.
        bool debug = false  //!< [in] Enable debug?
        )
        : m_data(AsciiHexToBinary(str)),
          m_packet{LEN(this->m_packetData), this->m_packetData},
          m_encoder(nullptr) {
        this->m_packet.setCommand(cmd);
        this->m_packet.setData(this->m_data.size(), this->m_data.data());
        this->m_encodedData.clear();
        this->m_encoder.setDebug(debug);
        this->m_encoder.encodeStart(&this->m_packet);
        uint8_t nextByte;
        while (this->m_encoder.encodeByte(&nextByte) == Error::NOT_DONE) {
            m_encodedData.push_back(nextByte);
        }
        m_encodedData.push_back(nextByte);
    }

    //! Tests if the encoded packet matches the given ascii hex data.
    //! @returns true if the encoded packet matches, false otherwise.
    bool matches(
        char const* expectedAsciiHexData  //!< [in] ASCII Hex data that should match the
                                          //!< encoded packet.
    ) {
        auto expectedData = AsciiHexToBinary(expectedAsciiHexData);
        if (expectedData != this->m_encodedData) {
            DumpMem("Expecting", 0, expectedData.data(), expectedData.size());
            DumpMem("  Encoded", 0, this->m_encodedData.data(), this->m_encodedData.size());
        }
        return expectedData == this->m_encodedData;
    }

    ByteBuffer m_data;         //!< Binary data bytes for packet.
    uint8_t m_packetData[16];  //!< Storage for the packet data.
    Packet m_packet;           //!< Packet to encode.
    ByteBuffer m_encodedData;  //!< Encoded version of the packet.
    PacketEncoder m_encoder;   //!< Encoder used to encode the packet.
};

TEST(PacketEncoderTest, NoDataTest) {
    auto test = PacketEncoderTest(Command::PING, "");
    EXPECT_TRUE(test.matches("c0 01 07 c0"));
}

TEST(PacketEncoderTest, OneByteDataTest) {
    auto test = PacketEncoderTest(Command::PING, "02");
    EXPECT_TRUE(test.matches("c0 01 02 1b c0"));
}

TEST(PacketEncoderTest, OneByteDataDebugTest) {
    auto test = PacketEncoderTest(Command::PING, "02", true);
    EXPECT_TRUE(test.matches("c0 01 02 1b c0"));
}

TEST(PacketEncoderTest, TwoBytesDataTest) {
    auto test = PacketEncoderTest(Command::PING, "02 03");
    EXPECT_TRUE(test.matches("c0 01 02 03 48 c0"));
}

TEST(PacketEncoderTest, EscapeEndTest) {
    auto test = PacketEncoderTest(0xc0, "02 03");
    EXPECT_TRUE(test.matches("c0 db dc 02 03 ae c0"));
}

TEST(PacketEncoderTest, EscapeEscTest) {
    auto test = PacketEncoderTest(0xdb, "02 03");
    EXPECT_TRUE(test.matches("c0 db dd 02 03 e0 c0"));
}

TEST(PacketEncoderTest, BadStateTest) {
    auto test = PacketEncoderTest(Command::PING, "");

    uint8_t nextByte;
    test.m_encoder.encodeStart(&test.m_packet);
    test.m_encoder.m_state = static_cast<PacketEncoder::State>(0x80);
    EXPECT_EQ(test.m_encoder.encodeByte(&nextByte), Error::BAD_STATE);
}

TEST(PacketEncoderDeathTest, AppendDataFullTest) {
    uint8_t packetData[4];
    Packet packet(LEN(packetData), packetData);

    uint8_t data[] = {1, 2, 3, 4, 5};

    packet.setData(0, nullptr);

    ASSERT_DEATH(
        { packet.appendData(LEN(data), data); },
        "Assertion `this->getDataLength\\(\\) \\+ dataLen <= this->getMaxDataLength\\(\\)' "
        "failed.");
}

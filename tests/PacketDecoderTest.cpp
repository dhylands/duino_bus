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
 *   @file   PacketDecoderTest.cpp
 *
 *   @brief  Tests for the PacketDecoder class.
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

//! Helper class for creating test cases.
class PacketDecoderTest {
 public:
    //! Constructor.
    explicit PacketDecoderTest(
        char const* str  //!< [in] ASCII Hex string representing data to decode.
        )
        : m_data(AsciiHexToBinary(str)),
          m_packet{LEN(this->m_packetData), this->m_packetData},
          m_decoder(&this->m_packet) {}

    //! Parses all of the bytes from m_dataStream using the packet parser.
    //! @returns Error::NONE if a packet was parsed successfully.
    //! @returns Error::NOT_DONE if more bytes are needed to complete parsing the packet.
    //! @returns Error::TOO_MUCH_DATA if the packet has more parameters than we have storage for.
    //! @returns Error::CHECKSUM if a checksum error was encountered while parsing.
    Error decodeData() {
        for (uint8_t byte : this->m_data) {
            if (auto err = this->m_decoder.decodeByte(byte); err != Error::NOT_DONE) {
                return err;
            }
        }
        return Error::NOT_DONE;
    }

    ByteBuffer m_data;         //!< Data to decode.
    uint8_t m_packetData[15];  //!< Storage for packet data.
    Packet m_packet;           //!< Packet being decoded.
    PacketDecoder m_decoder;   //!< Packet Decoder.
};

TEST(PacketDecoderTest, EmptyPacketTest) {
    auto test = PacketDecoderTest("c0 c0");

    EXPECT_EQ(test.decodeData(), Error::NOT_DONE);
}

TEST(PacketDecoderTest, EmptyPacket2Test) {
    auto test = PacketDecoderTest("aa bb c0 c0 c0 c0");

    EXPECT_EQ(test.decodeData(), Error::NOT_DONE);
}

TEST(PacketDecoderTest, NoDataTest) {
    auto test = PacketDecoderTest("c0 01 07 c0");

    EXPECT_EQ(test.decodeData(), Error::NONE);
    EXPECT_EQ(test.m_packet.getCommand(), Command::PING);
}

TEST(PacketDecoderTest, CrcErrorTest) {
    auto test = PacketDecoderTest("c0 01 08 c0");

    EXPECT_EQ(test.decodeData(), Error::CRC);
}

TEST(PacketDecoderTest, CrcErrorDebugTest) {
    auto test = PacketDecoderTest("c0 01 08 c0");

    test.m_decoder.setDebug(true);
    EXPECT_EQ(test.decodeData(), Error::CRC);
}

TEST(PacketDecoderTest, TooSmallTest) {
    auto test = PacketDecoderTest("c0 01 c0");

    EXPECT_EQ(test.decodeData(), Error::TOO_SMALL);
}

TEST(PacketDecoderTest, OneByteDataTest) {
    auto test = PacketDecoderTest("c0 01 02 1b c0");

    EXPECT_EQ(test.decodeData(), Error::NONE);
    EXPECT_EQ(test.m_packet.getCommand(), Command::PING);
    EXPECT_EQ(test.m_packet.getDataLength(), 1);
    EXPECT_EQ(test.m_packet.getData()[0], 0x02);
    EXPECT_EQ(test.m_packet.getCrc(), 0x1b);
}

TEST(PacketDecoderTest, OneByteDataDebugTest) {
    auto test = PacketDecoderTest("c0 01 02 1b c0");

    test.m_decoder.setDebug(true);
    EXPECT_EQ(test.decodeData(), Error::NONE);
    EXPECT_EQ(test.m_packet.getCommand(), Command::PING);
    EXPECT_EQ(test.m_packet.getDataLength(), 1);
    EXPECT_EQ(test.m_packet.getData()[0], 0x02);
    EXPECT_EQ(test.m_packet.getCrc(), 0x1b);
}

TEST(PacketDecoderTest, TwoBytesDataTest) {
    auto test = PacketDecoderTest("c0 01 02 03 48 c0");

    EXPECT_EQ(test.decodeData(), Error::NONE);
    EXPECT_EQ(test.m_packet.getCommand(), Command::PING);
    EXPECT_EQ(test.m_packet.getDataLength(), 2);
    EXPECT_EQ(test.m_packet.getData()[0], 0x02);
    EXPECT_EQ(test.m_packet.getData()[1], 0x03);
    EXPECT_EQ(test.m_packet.getCrc(), 0x48);
}

TEST(PacketDecoderTest, EscapeEndCommandTest) {
    auto test = PacketDecoderTest("c0 db dc 02 03 ae c0");

    EXPECT_EQ(test.decodeData(), Error::NONE);
    EXPECT_EQ(test.m_packet.getCommand(), 0xc0);
    EXPECT_EQ(test.m_packet.getDataLength(), 2);
    EXPECT_EQ(test.m_packet.getData()[0], 0x02);
    EXPECT_EQ(test.m_packet.getData()[1], 0x03);
    EXPECT_EQ(test.m_packet.getCrc(), 0xae);
}

TEST(PacketDecoderTest, EscapeEndDataTest) {
    auto test = PacketDecoderTest("c0 01 db dc 03 8f c0");

    EXPECT_EQ(test.decodeData(), Error::NONE);
    EXPECT_EQ(test.m_packet.getCommand(), Command::PING);
    EXPECT_EQ(test.m_packet.getDataLength(), 2);
    EXPECT_EQ(test.m_packet.getData()[0], 0xc0);
    EXPECT_EQ(test.m_packet.getData()[1], 0x03);
    EXPECT_EQ(test.m_packet.getCrc(), 0x8f);
}

TEST(PacketDecoderTest, EscapeEscTest) {
    auto test = PacketDecoderTest("c0 db dd 02 03 e0 c0");

    EXPECT_EQ(test.decodeData(), Error::NONE);
    EXPECT_EQ(test.m_packet.getCommand(), 0xdb);
    EXPECT_EQ(test.m_packet.getDataLength(), 2);
    EXPECT_EQ(test.m_packet.getData()[0], 0x02);
    EXPECT_EQ(test.m_packet.getData()[1], 0x03);
    EXPECT_EQ(test.m_packet.getCrc(), 0xe0);
}

TEST(PacketDecoderTest, EscapeOtherTest) {
    auto test = PacketDecoderTest("c0 db 01 02 03 48 c0");

    EXPECT_EQ(test.decodeData(), Error::NONE);
    EXPECT_EQ(test.m_packet.getCommand(), Command::PING);
    EXPECT_EQ(test.m_packet.getDataLength(), 2);
    EXPECT_EQ(test.m_packet.getData()[0], 0x02);
    EXPECT_EQ(test.m_packet.getData()[1], 0x03);
    EXPECT_EQ(test.m_packet.getCrc(), 0x48);
}

TEST(PacketDecoderTest, FullDataTest) {
    auto test = PacketDecoderTest("c0 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 14 c0");

    EXPECT_EQ(test.decodeData(), Error::NONE);
}

TEST(PacketDecoderTest, TooMuchDataTest) {
    auto test = PacketDecoderTest("c0 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f e0 c0");

    EXPECT_EQ(test.decodeData(), Error::TOO_MUCH_DATA);
}

TEST(PacketDecoderTest, TooMuchDataDebugTest) {
    auto test = PacketDecoderTest("c0 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f e0 c0");

    test.m_decoder.setDebug(true);
    EXPECT_EQ(test.decodeData(), Error::TOO_MUCH_DATA);
}

TEST(PacketDecoderTest, BadStateTest) {
    auto test = PacketDecoderTest("c0 00 01 07 c0");
    test.m_decoder.m_state = static_cast<PacketDecoder::State>(0x80);
    EXPECT_EQ(test.decodeData(), Error::BAD_STATE);
}

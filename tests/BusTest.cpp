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
 *   @file   BusTest.cpp
 *
 *   @brief  Tests for functions in Bus.cpp
 *
 ****************************************************************************/

#include <gtest/gtest.h>

#include "duino_bus/Bus.h"
#include "duino_bus/Packet.h"
#include "duino_bus/PacketDecoder.h"
#include "duino_bus/PacketHandler.h"
#include "duino_log/DumpMem.h"
#include "duino_util/AsciiHex.h"
#include "duino_util/Crc8.h"
#include "duino_util/Util.h"

//! Convenience alias.
//!@{
using Command = Packet::Command;
using Error = Packet::Error;
//!@}

//! Implements an IBus which integrates with testing.
class TestBus : public IBus {
 public:
    //! Constructor.
    explicit TestBus(
        Packet* cmdPacket,  //!< [in] Place to store incoming packet.
        Packet* rspPacket   //!< [in] Place to store incoming packet.
        )
        : IBus{cmdPacket, rspPacket} {}

    bool isDataAvailable() const { return this->m_decodeIdx < m_dataToDecode.size(); }

    bool readByte(uint8_t* byte) {
        if (this->isDataAvailable()) {
            *byte = this->m_dataToDecode[m_decodeIdx++];
            return true;
        }
        return false;
    }

    bool isSpaceAvailable() const { return true; }

    void writeByte(uint8_t byte) { this->m_encodedData.push_back(byte); }

    size_t m_decodeIdx = 0;     //!< Index used to iterate thrugh the incoming data.
    ByteBuffer m_dataToDecode;  //!< Represents incoming data.
    ByteBuffer m_encodedData;   //!< Place to store outgoing data.
};

//! Test handler for testing handler functions.
class TestHandler : public IPacketHandler {
 public:
    bool handlePacket(Packet const& cmd, Packet* rsp) override {
        if (cmd.getCommand() == 0x01) {
            // We set the response packet to match the incoming packet.
            rsp->setCommand(cmd.getCommand());
            rsp->setData(cmd.getDataLength(), cmd.getData());
            return true;
        }
        if (cmd.getCommand() == 0x02) {
            // Simulate a command with no response
            return true;
        }
        // Simulate an unhandled command
        return false;
    }

    char const* as_str(Packet::Command::Type cmd) const override {
        (void)cmd;
        return "???";
    }
};

//! Helper class used for tests.
class BusTest {
 public:
    //! Constructor.
    BusTest()
        : m_cmdPacket{LEN(this->m_cmdPacketData), this->m_cmdPacketData},
          m_rspPacket{LEN(this->m_rspPacketData), this->m_rspPacketData},
          m_bus{&this->m_cmdPacket, &this->m_rspPacket} {}

    //! Runs the bytes in `str` through the packet decoder.
    void processBytes(
        char const* str,             //!< [in] ASCII Hex bytes representing over the wire data.
        Packet::Error expectedError  //!< [in] Expected error (other than NOT_DONE).
    ) {
        this->m_bus.m_dataToDecode = AsciiHexToBinary(str);
        for (size_t i = 0; i < this->m_bus.m_dataToDecode.size(); i++) {
            EXPECT_TRUE(this->m_bus.isDataAvailable());
            if (i + 1 == this->m_bus.m_dataToDecode.size()) {
                EXPECT_EQ(this->m_bus.processByte(), expectedError);
            } else {
                EXPECT_EQ(this->m_bus.processByte(), Error::NOT_DONE);
            }
        }
    }

    //! Exercises the IBus::writePacket function.
    //! Converts the string into a packet, writes it, and confirms it wrote the same packet.
    void writePacket(
        char const* str  //!< [in] ASCII Hex bytes representing the packet.
    ) {
        ByteBuffer expectedData = AsciiHexToBinary(str);
        this->processBytes(str, Error::NONE);
        this->m_bus.writePacket(&this->m_cmdPacket);
        EXPECT_EQ(expectedData, this->m_bus.m_encodedData);
    }

    uint8_t m_cmdPacketData[15];  //!< Storage for command packets.
    uint8_t m_rspPacketData[15];  //!< Storage for response packet.
    Packet m_cmdPacket;           //!< Command packet
    Packet m_rspPacket;           //!< Response packet.
    TestBus m_bus;                //!< Bus for testing.
};

TEST(BusTest, ProcessByteTest) {
    auto test = BusTest();

    test.processBytes("c0 01 07 c0", Error::NONE);
}

TEST(BusTest, ProcessByteNoDataTest) {
    auto test = BusTest();

    // Test that calling processByte when no data is available just returns NOT_DONE
    EXPECT_EQ(test.m_bus.processByte(), Error::NOT_DONE);
}

TEST(BusTest, WritePacketTest) {
    auto test = BusTest();
    test.writePacket("c0 01 07 c0");
}

TEST(BusTest, HandlerWithResponseTest) {
    auto test = BusTest();
    auto testHandler = TestHandler();
    test.m_bus.add(testHandler);
    test.processBytes("c0 01 02 1b c0", Error::NONE);
    EXPECT_EQ(test.m_bus.handlePacket(), true);
    EXPECT_EQ(test.m_rspPacket.getCommand(), 1);
    EXPECT_EQ(test.m_rspPacket.getDataLength(), 1);
    EXPECT_EQ(test.m_rspPacket.getData()[0], 2);
    EXPECT_EQ(test.m_rspPacket.getCrc(), 0x1b);
}

TEST(BusTest, HandlerNoResponseTest) {
    auto test = BusTest();
    auto testHandler = TestHandler();
    test.m_bus.add(testHandler);
    test.processBytes("c0 02 03 23 c0", Error::NONE);
    EXPECT_EQ(test.m_bus.handlePacket(), true);
    EXPECT_EQ(test.m_rspPacket.getCommand(), 0);
}

TEST(BusTest, HandlerUnhandledTest) {
    auto test = BusTest();
    auto testHandler = TestHandler();
    test.m_bus.add(testHandler);
    test.processBytes("c0 03 04 23 c0", Error::NONE);
    EXPECT_EQ(test.m_bus.handlePacket(), false);
}

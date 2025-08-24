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
 *   @file   Packet.h
 *
 *   @brief  Container for a packet of data.
 *
 ****************************************************************************/

#pragma once

#include <cstdint>
#include <initializer_list>

#include "duino_util/Util.h"

//! Encapsulates the packet sent to a devices.
class Packet {
 public:
    // The over the wire format looks like a SLIP encoded packet.
    // Packets are SLIP encoded, and the length is inferred from the decoded packet.
    // The first byte of each packet is the command.
    // The last byte of the packet is an 8-bit CRC (crcmod.predefined.mkCrcFun('crc-8'))
    // Each packet has data bytes between the command and the CRC.

    static constexpr uint8_t END = 0xC0;      //!< Start/End of Frame
    static constexpr uint8_t ESC = 0xDB;      //!< Next char is escaped
    static constexpr uint8_t ESC_END = 0xDC;  //!< Escape an END character
    static constexpr uint8_t ESC_ESC = 0xDD;  //!< Escape an ESC character

    //! Error code.
    enum class Error {
        NONE = 0,           //!< No Error.
        NOT_DONE = 1,       //!< Indicates that parsing is not complete.
        CRC = 2,            //!< CRC error occurred during parsing.
        TIMEOUT = 3,        //!< Indicates that a timeout occurred while waiting for a reply.
        TOO_MUCH_DATA = 4,  //!< Packet storage isn't big enough.
        TOO_SMALL = 5,      //!< Not enough data for a packet.
        BAD_STATE = 6,      //!< Not enough data for a packet.
        OS = 7,             //!< OS Error.
    };

    //! @brief Predefined commands.
    //! @details We use a struct rather than an enum so that a device can derive their own commands.
    struct Command : Bits<uint8_t> {
        //! Constructor.
        Command(
            Type cmd  //!< [in] Command.
            )
            : Bits(cmd) {}

        static constexpr Type PING = 0x01;  //!< Checks to see if the device is alive

        //! @brief Returns the string version of a command.
        //! @details By making this virtual, derived classes can add custom commands and also
        //!          return the string equivalents.
        //! @returns A pointer to a C string containing the string equivalent of the command.
        virtual const char* as_str() {
            switch (this->value) {
                case PING:
                    return "PING";
            }
            return "???";
        }
    };

    //! Constructor where the storage for parameter data is specified.
    Packet(
        size_t maxData,  //!< [in] Maximum number of data bytes in the packet.
        void* data       //!< [in] Place to store packet data.
    );

    //! Dumps the contents of a packet.
    void dump(
        char const* label  //!< [in] Label to print on each line.
    ) const;

    //! Returns the command from the command packet.
    //! @returns Command::Type containg the command found in the packet.
    Command::Type getCommand() const { return this->m_command.value; }

    //! Sets the command for the packet using a Command object.
    void setCommand(
        Command cmd  //!< [in] Command object to set command from..
    ) {
        this->m_command.value = cmd.value;
    }

    //! Sets the command for the packet using a CommandType.
    void setCommand(
        Command::Type cmd  //!< [in] Command to set command to.
    ) {
        this->m_command.value = cmd;
    }

    //! Returns the length of the data portion of the packet.
    //! Since we don't know the length of the packet ahead of time,
    //! we need maxDataLen to allow for a spot to store the CRC.
    //! @returns the maximum number of data bytes in the packet.
    size_t getMaxDataLength() const { return this->m_maxDataLen; }

    //! Returns the length of the data portion of the packet.
    //! @returns the number of data bytes in the packet.
    size_t getDataLength() const { return this->m_dataLen; }

    //! @returns the amount of space remaining in the packet.
    size_t getSpaceRemaining() const { return this->m_maxDataLen - this->m_dataLen; }

    //! @returns a mutable pointer to the data portion of the packet.
    uint8_t* getData() { return this->m_data; }

    //! @returns a const pointer to the data portion of the packet.
    uint8_t const* getData() const { return this->m_data; }

    //! @returns a mutable pointer to the first byte beyond the end of the data.
    uint8_t* getWriteData(
        size_t numBytes = 0  //!< [in] Number of bytes to advance the write pointer by.
    ) {
        uint8_t* data = &this->m_data[this->m_dataLen];
        this->m_dataLen += numBytes;
        return data;
    }

    //! Sets the packet data.
    //! To set the packet to be empty, pass in a zero dataLen.
    void setData(
        size_t dataLen,   //!< [in] Size of the packet data to copy in.
        void const* data  //!< [in] Packet data to copy in.
    );

    //! Appends data to the packet.
    void appendData(
        size_t dataLen,   //!< [in] Size of the packet data to copy in.
        void const* data  //!< [in] Packet data to copy in.
    );

    //! Appends a byte to the packet.
    void appendByte(
        uint8_t byte  //!< Byte to append.
    );

    //! Append a string to the packet.
    void append(
        char const* str  //!< [in] String to append.
    );

    //! Appends data to the packet.
    //! @tparam T type of the data to append.
    template <typename T>
    void append(
        T data  //!< [in] Data to append.
    ) {
        // Produce an error if somebody tries to pass a pointer.
        static_assert(!std::is_pointer_v<T>);

        this->appendData(sizeof(data), &data);
    }

    //! @returns the CRC contained in the packet.
    uint8_t getCrc() const;

    //! Calculates the CRC of the data.
    //! @returns the CRC over the command and the data.
    uint8_t calcCrc() const;

    //! Calculates the CRC of the data and saves it in the packet.
    void calcAndStoreCrc();

 private:
    friend class PacketDecoder;

    //! This allows the TEST(PacketTest, ExtractCrcTest) function to call extractCrc
    friend class PacketDeathTest_ExtractCrcTest_Test;

    //! Extracts the CRC from the last byte of the data.
    //! When receiving a packet we don't know the length ahead of time,
    //! so the CRC is stored as the last byte of the data. This function
    //! removes the CRC from that last byte of data and stores it in the
    //! m_crc field.
    uint8_t extractCrc();

    Command m_command;                //!< Command associated with this packet.
    size_t const m_maxDataLen = 0;    //!< Max number of bytes of packet data.
    size_t m_dataLen = 0;             //!< Length of data in the packet.
    uint8_t* const m_data = nullptr;  //!< Place to store packet data.
    uint8_t m_crc;                    //!< CRC associated with the data.
};

//! Returns a string version of the error code.
//! @returns A pointer to a literal string.
char const* as_str(
    Packet::Error err  //!< [in] Error code to convert to a string.
);

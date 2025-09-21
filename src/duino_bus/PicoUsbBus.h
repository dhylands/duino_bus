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
 *   @file   PicoUsbBus.h
 *
 *   @brief  Implements a bus for sending packets via an Pico USB port.
 *
 ****************************************************************************/

#pragma once

#include <cinttypes>
#include <vector>

#include "tusb.h"

#include "duino_bus/Bus.h"

//! Implements a bus using an Arduino Serial port.
class PicoUsbBus : public IBus {
 public:
    //! Constructor.
    PicoUsbBus(
        uint8_t intf,       //!< [in] USB Interface to use.
        Packet* cmdPacket,  //!< [in] Place to store command packet
        Packet* rspPacket,  //!< [in] Place to store response packet
        Packet* logPacket,  //!< [mod] Place to store outgoing log packet.
        Packet* evtPacket   //!< [mod] Place to store outgoinf event packet.
    );

    //! Destructor.
    ~PicoUsbBus() override;

    bool isDataAvailable() const override;
    bool readByte(uint8_t* byte) override;
    bool isSpaceAvailable() const override;
    void writeByte(uint8_t byte) override;
    void flush(void) override;
    bool isConnected(void) const override;

 private:
    friend void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts);

    uint8_t const m_intf;  //!< USB Interface to use.
};

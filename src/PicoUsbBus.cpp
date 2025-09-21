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
 *   @file   PicoUsbBus.cpp
 *
 *   @brief  Implements a bus for sending packets via an Arduino serial port.
 *
 ****************************************************************************/

#include "duino_bus/PicoUsbBus.h"

#include <cassert>

#include "tusb.h"

#include "duino_util/Util.h"

//! Array of connected flags, indexed by intf.
//! We use a simple array rather than a vector since we can't control the order
//! of initialization of global objects, and PicoUsbBus is declared globally.
static bool g_isConnected[4];

PicoUsbBus::PicoUsbBus(
    uint8_t intf,
    Packet* cmdPacket,
    Packet* rspPacket,
    Packet* logPacket,
    Packet* evtPacket)
    : IBus{cmdPacket, rspPacket, logPacket, evtPacket}, m_intf{intf} {}

PicoUsbBus::~PicoUsbBus() {}

bool PicoUsbBus::isConnected() const {
    if (this->m_intf < LEN(g_isConnected)) {
        return g_isConnected[m_intf];
    }
    return false;
}

bool PicoUsbBus::isDataAvailable() const {
    return tud_cdc_n_available(this->m_intf) > 0;
}

bool PicoUsbBus::readByte(uint8_t* byte) {
    uint32_t count = tud_cdc_n_read(this->m_intf, byte, 1);
    return (count >= 1);
}

bool PicoUsbBus::isSpaceAvailable() const {
    return tud_cdc_n_write_available(this->m_intf) > 0;
}

void PicoUsbBus::writeByte(uint8_t byte) {
    if (this->isConnected()) {
        tud_cdc_n_write(this->m_intf, &byte, 1);
    }
}

void PicoUsbBus::flush(void) {
    if (this->isConnected()) {
        tud_cdc_n_write_flush(this->m_intf);
    }
}

//! tud_cdc_line_state_cb is a weak function from TinyUSB.
//! It's called  in response to the SET_CONTROL_LINE_STATE CDC ACM message.
//! We use it to detect when the host side serial port is opened, since it
//! asserts the DTR line on open and deasserts on close.
void tud_cdc_line_state_cb(
    uint8_t intf,  //!< [in] Interface the SET_CONTROL_LINE_STATE msg is for.
    bool dtr,      //!< [in] true =- assert DTR, false = deassert DTR
    bool rts       //!< [in] true =- assert RTS, false = deassert RTS
) {
    (void)rts;
    if (intf >= LEN(g_isConnected)) {
        return;
    }
    g_isConnected[intf] = dtr;
}

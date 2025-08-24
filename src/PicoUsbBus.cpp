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

#include "tusb.h"

PicoUsbBus::PicoUsbBus(uint8_t intf, Packet* cmdPacket, Packet* rspPacket)
    : IBus{cmdPacket, rspPacket}, m_intf{intf} {}

PicoUsbBus::~PicoUsbBus() {}

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
    tud_cdc_n_write(this->m_intf, &byte, 1);
}

void PicoUsbBus::flush(void) {
    tud_cdc_n_write_flush(this->m_intf);
}

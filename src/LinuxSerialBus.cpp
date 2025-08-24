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
 *   @file   LinuxSerialBus.cpp
 *
 *   @brief  Implements a bus for sending packets using a linux serial port.
 *
 ****************************************************************************/

#if !defined(ARDUINO)

#include "LinuxSerialBus.h"

#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>

#include "duino_log/Log.h"

LinuxSerialBus::LinuxSerialBus(Packet* cmdPacket, Packet* rspPacket) : IBus{cmdPacket, rspPacket} {}

LinuxSerialBus::~LinuxSerialBus() {
    if (this->m_serial >= 0) {
        close(this->m_serial);
    }
}

IBus::Error LinuxSerialBus::open(char const* portName, uint32_t baudRate) {
    if ((this->m_serial = ::open(portName, O_RDWR | O_EXCL)) < 0) {
        Log::error("Unable to open serial port '%s': %s", portName, strerror(errno));
        return Error::OS;
    }

    struct termios attr;
    if (tcgetattr(this->m_serial, &attr) < 0) {
        Log::error("Call to tcgetattr failed: %s\n", strerror(errno));
        return Error::OS;
    }

    attr.c_iflag = 0;
    attr.c_oflag = 0;
    attr.c_cflag = CLOCAL | CREAD | CS8;
    attr.c_lflag = 0;
    attr.c_cc[VTIME] = 0;  // timeout in tenths of a second
    attr.c_cc[VMIN] = 1;

    cfsetispeed(&attr, baudRate);
    cfsetospeed(&attr, baudRate);

    if (tcsetattr(this->m_serial, TCSAFLUSH, &attr) < 0) {
        Log::error("Call to tcsetattr failed: %s\n", strerror(errno));
        return Error::OS;
    }
    return Error::NONE;
}

bool LinuxSerialBus::isDataAvailable() const {
    struct pollfd pfd = {
        .fd = this->m_serial,
        .events = POLLIN,
        .revents = 0,
    };

    return poll(&pfd, 1, 0) > 0;
}

bool LinuxSerialBus::readByte(uint8_t* byte) {
    printf("Reading ...");
    fflush(stdout);
    int bytesRead = ::read(this->m_serial, byte, 1);
    printf(" 0x%02" PRIx8 "\n", *byte);
    return bytesRead == 1;
}

bool LinuxSerialBus::isSpaceAvailable() const {
    struct pollfd pfd = {
        .fd = this->m_serial,
        .events = POLLOUT,
        .revents = 0,
    };

    return poll(&pfd, 1, 0) > 0;
}

void LinuxSerialBus::writeByte(uint8_t byte) {
    printf("Writing 0x%02" PRIx8 " ...", byte);
    ::write(this->m_serial, &byte, 1);
    printf("\n");
}

#endif  //  !defined(ARDUINO)

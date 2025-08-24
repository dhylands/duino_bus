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
 *   @file   SocketBus.h
 *
 *   @brief  Implements a bus for sending TCP packets.
 *
 ****************************************************************************/

#pragma once

#if !defined(ARDUINO)

#include <netinet/in.h>

#include <cinttypes>

#include "duino_bus/Bus.h"

//! Implements a bus using TCP/IP sockets.
class SocketBus : public IBus {
 public:
    using Socket = int;     //!< Type to store the socket in.
    using Port = uint16_t;  //!< Type to hold a port number.

    static constexpr Socket INVALID_SOCKET = -1;             //!< Invalid socket.
    static constexpr char const* DEFAULT_PORT_STR = "8888";  //!< Default port to use.

    //! Holds either an IPv4 or IPv6 socket address.
    union Address {
        struct sockaddr_in6 sa6;  //!< IPv6 Address
        struct sockaddr_in sa4;   //!< IPv4 Address
    };
    static_assert(offsetof(sockaddr_in6, sin6_family) == offsetof(sockaddr_in, sin_family));
    static_assert(offsetof(sockaddr_in6, sin6_port) == offsetof(sockaddr_in, sin_port));

    //! Constructor.
    SocketBus(
        Packet* cmdPacket,  //!< [in] Place to store command packet
        Packet* rspPacket   //!< [in] Place to store response packet
    );

    //! Destructor.
    ~SocketBus() override;

    //! @returns the underlying socket object.
    int socket() const { return this->m_socket; }

    //! Sets up a server.
    //! @returns Error::NONE if the server was setup successfully, or an error code otherwise.
    Error setupServer(
        char const* portStr  //!< [in] Port (as a string) to serve.
    );

    //! Attempts to connect to a server.
    //! @returns Error::NONE if the client connected to the server successfully, or an error code
    //! otherwise.
    Error connectToServer(
        char const* server,  //!< [in] Server to connect to.
        char const* portStr  //!< [in] Port to connect to.
    );

    //! Prints information from the addrinfo struct.
    void printAddrInfo(
        char const* label,   //!< [in] Label to prefix output with,
        struct addrinfo* ai  //!< [in] Pointer to addrinfo struct.
    ) const;

    //! Prints information about a socket address.
    void printAddrInfo(
        char const* label,   //!< [in] Label to prefix output with.
        Address const& addr  //!< [in] Address to print.
    ) const;

    //! Prints information about a socket address.
    void printAddrInfo(
        char const* label,  //!< [in] Label to prefix output with.
        int family,         //!< [in] Family that defines the address.
        void const* addr,   //!< [in] Pointer to address bytes.
        int port            //!< [in] Port number to print.
    ) const;

    bool isDataAvailable() const override;
    bool readByte(uint8_t* byte) override;
    bool isSpaceAvailable() const override;
    void writeByte(uint8_t byte) override;

 private:
    Error makeSocketNonBlocking(Socket skt);

    Socket m_socket = INVALID_SOCKET;  //!< Connected socket.
};

#endif  // !defined(ARDUINO)

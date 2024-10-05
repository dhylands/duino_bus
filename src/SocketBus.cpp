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
 *   @file   SocketBus.cpp
 *
 *   @brief  Implements a TCP socket port.
 *
 ****************************************************************************/

#if !defined(ARDUINO)

#include "SocketBus.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <memory>

#include "Log.h"
#include "ScopeGuard.h"

SocketBus::SocketBus(Packet* cmdPacket, Packet* rspPacket) : IBus{cmdPacket, rspPacket} {}

SocketBus::~SocketBus() {
    Log::info("Closing socket: %d", this->m_socket);
    ::close(this->m_socket);
}

IBus::Error SocketBus::setupServer(char const* portStr) {
    struct addrinfo hints;
    struct addrinfo* serverInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    if (auto rc = getaddrinfo(NULL, portStr, &hints, &serverInfo); rc != 0) {
        Log::error("getaddrinfo failed: %s", gai_strerror(rc));
        return Error::OS;
    }

    // Loop thru all of the results, and pick the first one where bind is successful
    Socket listenSocket;
    struct addrinfo* info = serverInfo;
    for (; info != nullptr; info = info->ai_next) {
        listenSocket = ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (listenSocket < 0) {
            Log::error("Failed to create socket: %s", strerror(errno));
            continue;
        }

        int enable = 1;
        if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
            Log::error("Failed to set REUSEADDR socket option: %s", strerror(errno));
            close(listenSocket);
            return Error::OS;
        }
        if (bind(listenSocket, info->ai_addr, info->ai_addrlen) < 0) {
            Log::error("bind failed: %s", strerror(errno));
            close(listenSocket);
            continue;
        }

        // We successfully bound the socket to a port.
        break;
    }
    if (info == nullptr) {
        Log::error("No IP Address found for binding");
        freeaddrinfo(serverInfo);
        close(listenSocket);
        return Error::OS;
    }

    freeaddrinfo(serverInfo);

    Log::info("Listening on port %s ...", portStr);
    if (::listen(listenSocket, 1) < 0) {
        Log::error("Failed to listen for incoming connection: %s", strerror(errno));
        close(listenSocket);
        return Error::OS;
    }

    Address client;
    memset(&client, 0, sizeof(client));
    socklen_t client_len = sizeof(client);
    Socket clientSocket;
    if ((clientSocket = ::accept(listenSocket, (sockaddr*)&client, &client_len)) < 0) {
        Log::error("Failed to accept incoming connection: %s", strerror(errno));
        close(listenSocket);
        return Error::OS;
    }
    if (auto rc = this->makeSocketNonBlocking(clientSocket); rc != Error::NONE) {
        return rc;
    }
    close(listenSocket);

    this->printAddrInfo("Accepted connection from", client);

    this->m_socket = clientSocket;
    return Error::NONE;
}

IBus::Error SocketBus::connectToServer(char const* server, char const* portStr) {
    struct addrinfo hints;
    struct addrinfo* serverInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (auto rc = getaddrinfo(server, portStr, &hints, &serverInfo); rc != 0) {
        Log::error("getaddrinfo failed: %s", gai_strerror(rc));
        return Error::OS;
    }

    // Loop thru all of the results, and pick the first one where bind is successful
    Socket serverSocket;
    struct addrinfo* info = serverInfo;
    for (; info != nullptr; info = info->ai_next) {
        this->printAddrInfo("Trying", info);
        serverSocket = ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (serverSocket < 0) {
            Log::error("Failed to create socket: %s", strerror(errno));
            continue;
        }

        if (connect(serverSocket, info->ai_addr, info->ai_addrlen) < 0) {
            Log::error("connect failed: %s", strerror(errno));
            close(serverSocket);
            continue;
        }

        // We successfully bound the socket to a port.
        break;
    }
    if (info == nullptr) {
        Log::error("No IP Address found for connecting");
        freeaddrinfo(serverInfo);
        close(serverSocket);
        return Error::OS;
    }

    if (auto rc = this->makeSocketNonBlocking(serverSocket); rc != Error::NONE) {
        close(serverSocket);
        return rc;
    }

    this->printAddrInfo("Connected to", info);

    freeaddrinfo(serverInfo);

    this->m_socket = serverSocket;
    return Error::NONE;
}

void SocketBus::printAddrInfo(char const* label, struct addrinfo* ai) const {
    void* addrPtr;
    int port;
    switch (ai->ai_family) {
        case AF_INET: {
            auto sa = reinterpret_cast<struct sockaddr_in*>(ai->ai_addr);
            addrPtr = &sa->sin_addr;
            port = ntohs(sa->sin_port);
            break;
        }
        case AF_INET6: {
            auto sa = reinterpret_cast<struct sockaddr_in6*>(ai->ai_addr);
            addrPtr = &sa->sin6_addr;
            port = ntohs(sa->sin6_port);
            break;
        }
        default: {
            Log::error("Unrecognized ai_family: %d\n", ai->ai_family);
            return;
        }
    }
    this->printAddrInfo(label, ai->ai_family, addrPtr, port);
}

void SocketBus::printAddrInfo(char const* label, Address const& addr) const {
    void const* addrPtr;
    int family = addr.sa4.sin_family;
    int port;
    switch (family) {
        case AF_INET:
            addrPtr = reinterpret_cast<void const*>(&addr.sa4.sin_addr);
            port = ntohs(addr.sa4.sin_port);
            break;
        case AF_INET6:
            addrPtr = reinterpret_cast<void const*>(&addr.sa6.sin6_addr);
            port = ntohs(addr.sa6.sin6_port);
            break;
        default:
            Log::error("Unrecognized family: %d\n", family);
            return;
    }
    this->printAddrInfo(label, family, addrPtr, port);
}

void SocketBus::printAddrInfo(char const* label, int family, void const* addr, int port) const {
    char addrStr[100];
    char const* familyStr = family == PF_INET6 ? "6" : "4";
    inet_ntop(family, addr, addrStr, LEN(addrStr));
    Log::info("%s IPv%s [%s]:%d", label, familyStr, addrStr, port);
}

IBus::Error SocketBus::makeSocketNonBlocking(Socket skt) {
    // Try to make the socket non-blocking.
    int flags = ::fcntl(skt, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (::fcntl(skt, F_SETFL, flags) < 0) {
        Log::error("Failed to make socket non-blocking: %s", strerror(errno));
        return Error::OS;
    }
    return Error::NONE;
}

bool SocketBus::isDataAvailable() const {
    struct pollfd pfd = {
        .fd = this->m_socket,
        .events = POLLIN,
        .revents = 0,
    };

    return poll(&pfd, 1, 0) > 0;
}

bool SocketBus::readByte(uint8_t* byte) {
    return ::recv(this->m_socket, byte, 1, 0) == 1;
}

bool SocketBus::isSpaceAvailable() const {
    struct pollfd pfd = {
        .fd = this->m_socket,
        .events = POLLOUT,
        .revents = 0,
    };

    return poll(&pfd, 1, 0) > 0;
}

void SocketBus::writeByte(uint8_t byte) {
    ::send(this->m_socket, &byte, 1, 0);
}

#endif  // !defined(ARDUINO)

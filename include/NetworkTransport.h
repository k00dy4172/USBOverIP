#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdint>

#include "USBTransport.h"

enum class NetworkMode
{
    Client,
    Server
};

class NetworkTransport :
    public USBTransport
{
public:

    explicit NetworkTransport(
        NetworkMode Mode
    );

    ~NetworkTransport() override;

    bool Initialize() override;

    bool Read(
        USBPacket& Packet
    ) override;

    bool Write(
        const USBPacket& Packet
    ) override;

    void Shutdown() override;

private:

    bool InitializeClient();

    bool InitializeServer();

    bool SendAll(
        SOCKET Socket,
        const char* Data,
        int Size
    );

    bool ReceiveAll(
        SOCKET Socket,
        char* Data,
        int Size
    );

private:

    NetworkMode m_Mode;

    SOCKET m_Socket =
        INVALID_SOCKET;

    SOCKET m_ClientSocket =
        INVALID_SOCKET;
};
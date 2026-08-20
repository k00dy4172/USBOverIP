#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

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

    SOCKET m_Socket;

    SOCKET m_ClientSocket;
};
#include "NetworkTransport.h"
#include "NetworkPacket.h"

#include <iostream>

NetworkTransport::NetworkTransport(
    NetworkMode Mode
)
    : m_Mode(Mode),
    m_Socket(INVALID_SOCKET),
    m_ClientSocket(INVALID_SOCKET)
{
}

NetworkTransport::~NetworkTransport()
{
    Shutdown();
}

bool NetworkTransport::Initialize()
{
    WSADATA WsaData{};

    int Result =
        WSAStartup(
            MAKEWORD(2, 2),
            &WsaData
        );

    if (Result != 0)
    {
        std::cout
            << "WSAStartup failed."
            << std::endl;

        return false;
    }

    m_Socket =
        socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP
        );

    if (m_Socket == INVALID_SOCKET)
    {
        std::cout
            << "Socket creation failed."
            << std::endl;

        WSACleanup();

        return false;
    }

    if (m_Mode == NetworkMode::Client)
    {
        return InitializeClient();
    }

    return InitializeServer();
}

bool NetworkTransport::InitializeClient()
{
    sockaddr_in ServerAddress{};

    ServerAddress.sin_family =
        AF_INET;

    ServerAddress.sin_port =
        htons(5000);

    if (inet_pton(
        AF_INET,
        "127.0.0.1",
        &ServerAddress.sin_addr
    ) <= 0)
    {
        std::cout
            << "Invalid server address."
            << std::endl;

        return false;
    }

    int Result =
        connect(
            m_Socket,
            reinterpret_cast<sockaddr*>(
                &ServerAddress
                ),
            sizeof(ServerAddress)
        );

    if (Result == SOCKET_ERROR)
    {
        std::cout
            << "Connection failed : "
            << WSAGetLastError()
            << std::endl;

        return false;
    }

    std::cout
        << "Connected to server."
        << std::endl;

    return true;
}

bool NetworkTransport::InitializeServer()
{
    sockaddr_in ServerAddress{};

    ServerAddress.sin_family =
        AF_INET;

    ServerAddress.sin_addr.s_addr =
        htonl(INADDR_ANY);

    ServerAddress.sin_port =
        htons(5000);

    int Result =
        bind(
            m_Socket,
            reinterpret_cast<sockaddr*>(
                &ServerAddress
                ),
            sizeof(ServerAddress)
        );

    if (Result == SOCKET_ERROR)
    {
        std::cout
            << "Bind failed : "
            << WSAGetLastError()
            << std::endl;

        return false;
    }

    Result =
        listen(
            m_Socket,
            1
        );

    if (Result == SOCKET_ERROR)
    {
        std::cout
            << "Listen failed : "
            << WSAGetLastError()
            << std::endl;

        return false;
    }

    std::cout
        << "Waiting for client..."
        << std::endl;

    m_ClientSocket =
        accept(
            m_Socket,
            nullptr,
            nullptr
        );

    if (m_ClientSocket == INVALID_SOCKET)
    {
        std::cout
            << "Accept failed : "
            << WSAGetLastError()
            << std::endl;

        return false;
    }

    std::cout
        << "Client connected."
        << std::endl;

    return true;
}

bool NetworkTransport::SendAll(
    SOCKET Socket,
    const char* Data,
    int Size
)
{
    int TotalSent = 0;

    while (TotalSent < Size)
    {
        int Result =
            send(
                Socket,
                Data + TotalSent,
                Size - TotalSent,
                0
            );

        if (Result == SOCKET_ERROR)
        {
            return false;
        }

        if (Result == 0)
        {
            return false;
        }

        TotalSent += Result;
    }

    return true;
}

bool NetworkTransport::ReceiveAll(
    SOCKET Socket,
    char* Data,
    int Size
)
{
    int TotalReceived = 0;

    while (TotalReceived < Size)
    {
        int Result =
            recv(
                Socket,
                Data + TotalReceived,
                Size - TotalReceived,
                0
            );

        if (Result <= 0)
        {
            return false;
        }

        TotalReceived += Result;
    }

    return true;
}

bool NetworkTransport::Write(
    const USBPacket& Packet
)
{
    SOCKET Socket =
        m_Mode == NetworkMode::Client
        ? m_Socket
        : m_ClientSocket;

    if (Socket == INVALID_SOCKET)
    {
        return false;
    }

    if (Packet.Data.size() >
        NETWORK_MAX_PACKET_SIZE)
    {
        std::cout
            << "Packet too large."
            << std::endl;

        return false;
    }

    NetworkPacketHeader Header{};

    Header.Magic =
        NETWORK_PACKET_MAGIC;

    Header.DataSize =
        static_cast<uint32_t>(
            Packet.Data.size()
            );

    Header.InterfaceNumber =
        Packet.InterfaceNumber;

    Header.EndpointAddress =
        Packet.EndpointAddress;

    if (!SendAll(
        Socket,
        reinterpret_cast<const char*>(
            &Header
            ),
        sizeof(Header)
    ))
    {
        std::cout
            << "Failed to send packet header."
            << std::endl;

        return false;
    }

    if (!Packet.Data.empty())
    {
        if (!SendAll(
            Socket,
            reinterpret_cast<const char*>(
                Packet.Data.data()
                ),
            static_cast<int>(
                Packet.Data.size()
                )
        ))
        {
            std::cout
                << "Failed to send packet data."
                << std::endl;

            return false;
        }
    }

    return true;
}

bool NetworkTransport::Read(
    USBPacket& Packet
)
{
    SOCKET Socket =
        m_Mode == NetworkMode::Client
        ? m_Socket
        : m_ClientSocket;

    if (Socket == INVALID_SOCKET)
    {
        return false;
    }

    NetworkPacketHeader Header{};

    if (!ReceiveAll(
        Socket,
        reinterpret_cast<char*>(
            &Header
            ),
        sizeof(Header)
    ))
    {
        return false;
    }

    if (Header.Magic !=
        NETWORK_PACKET_MAGIC)
    {
        std::cout
            << "Invalid network packet."
            << std::endl;

        return false;
    }

    if (Header.DataSize >
        NETWORK_MAX_PACKET_SIZE)
    {
        std::cout
            << "Received packet is too large."
            << std::endl;

        return false;
    }

    Packet.InterfaceNumber =
        Header.InterfaceNumber;

    Packet.EndpointAddress =
        Header.EndpointAddress;

    Packet.Data.resize(
        Header.DataSize
    );

    if (Header.DataSize > 0)
    {
        if (!ReceiveAll(
            Socket,
            reinterpret_cast<char*>(
                Packet.Data.data()
                ),
            static_cast<int>(
                Header.DataSize
                )
        ))
        {
            Packet =
                USBPacket{};

            return false;
        }
    }

    return true;
}

void NetworkTransport::Shutdown()
{
    if (m_ClientSocket != INVALID_SOCKET)
    {
        closesocket(
            m_ClientSocket
        );

        m_ClientSocket =
            INVALID_SOCKET;
    }

    if (m_Socket != INVALID_SOCKET)
    {
        closesocket(
            m_Socket
        );

        m_Socket =
            INVALID_SOCKET;
    }

    WSACleanup();
}
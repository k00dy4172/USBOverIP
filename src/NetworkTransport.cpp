#include "NetworkTransport.h"

#include <iostream>
#include <vector>
#include <cstring>

namespace
{
#pragma pack(push, 1)

    struct NetworkPacketHeader
    {
        uint32_t Magic;

        uint16_t Version;

        uint16_t HeaderSize;

        uint64_t IrpId;

        uint16_t Bus;

        uint16_t DeviceAddress;

        uint8_t EndpointAddress;

        uint8_t TransferType;

        uint8_t Direction;

        uint8_t Reserved;

        uint32_t DataLength;
    };

#pragma pack(pop)

    constexpr uint32_t NETWORK_MAGIC =
        0x55534250;

    constexpr uint16_t NETWORK_VERSION = 1;

    constexpr uint16_t NETWORK_HEADER_SIZE =
        sizeof(NetworkPacketHeader);

    constexpr uint32_t NETWORK_MAX_DATA_SIZE =
        16 * 1024 * 1024;

    constexpr uint16_t NETWORK_PORT = 5000;

    uint8_t ToTransferType(
        USBTransferType Type
    )
    {
        switch (Type)
        {
        case USBTransferType::Isochronous:
            return 0;

        case USBTransferType::Interrupt:
            return 1;

        case USBTransferType::Control:
            return 2;

        case USBTransferType::Bulk:
            return 3;

        default:
            return 0xFF;
        }
    }

    USBTransferType FromTransferType(
        uint8_t Type
    )
    {
        switch (Type)
        {
        case 0:
            return USBTransferType::Isochronous;

        case 1:
            return USBTransferType::Interrupt;

        case 2:
            return USBTransferType::Control;

        case 3:
            return USBTransferType::Bulk;

        default:
            return USBTransferType::Unknown;
        }
    }

    uint8_t ToDirection(
        USBTransferDirection Direction
    )
    {
        switch (Direction)
        {
        case USBTransferDirection::In:
            return 0;

        case USBTransferDirection::Out:
            return 1;

        default:
            return 0xFF;
        }
    }

    USBTransferDirection FromDirection(
        uint8_t Direction
    )
    {
        switch (Direction)
        {
        case 0:
            return USBTransferDirection::In;

        case 1:
            return USBTransferDirection::Out;

        default:
            return USBTransferDirection::Unknown;
        }
    }
}

NetworkTransport::NetworkTransport(
    NetworkMode Mode
)
    :
    m_Mode(Mode)
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
            << "WSAStartup failed : "
            << Result
            << std::endl;

        return false;
    }

    if (m_Mode ==
        NetworkMode::Client)
    {
        return InitializeClient();
    }

    return InitializeServer();
}

bool NetworkTransport::InitializeClient()
{
    m_Socket =
        socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP
        );

    if (m_Socket ==
        INVALID_SOCKET)
    {
        std::cout
            << "Client socket creation failed."
            << std::endl;

        return false;
    }

    sockaddr_in ServerAddress{};

    ServerAddress.sin_family =
        AF_INET;

    ServerAddress.sin_port =
        htons(
            NETWORK_PORT
        );

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

Result:
    {
        int ConnectResult =
            connect(
                m_Socket,
                reinterpret_cast<sockaddr*>(
                    &ServerAddress
                    ),
                sizeof(
                    ServerAddress
                    )
            );

        if (ConnectResult ==
            SOCKET_ERROR)
        {
            std::cout
                << "Connection failed : "
                << WSAGetLastError()
                << std::endl;

            return false;
        }
    }

    std::cout
        << "Connected to server."
        << std::endl;

    return true;
}

bool NetworkTransport::InitializeServer()
{
    m_Socket =
        socket(
            AF_INET,
            SOCK_STREAM,
            IPPROTO_TCP
        );

    if (m_Socket ==
        INVALID_SOCKET)
    {
        std::cout
            << "Server socket creation failed."
            << std::endl;

        return false;
    }

    sockaddr_in ServerAddress{};

    ServerAddress.sin_family =
        AF_INET;

    ServerAddress.sin_addr.s_addr =
        htonl(
            INADDR_ANY
        );

    ServerAddress.sin_port =
        htons(
            NETWORK_PORT
        );

    if (bind(
        m_Socket,
        reinterpret_cast<sockaddr*>(
            &ServerAddress
            ),
        sizeof(
            ServerAddress
            )
    ) == SOCKET_ERROR)
    {
        std::cout
            << "Bind failed : "
            << WSAGetLastError()
            << std::endl;

        return false;
    }

    if (listen(
        m_Socket,
        1
    ) == SOCKET_ERROR)
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

    if (m_ClientSocket ==
        INVALID_SOCKET)
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

        if (Result ==
            SOCKET_ERROR)
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
        m_Mode ==
        NetworkMode::Client
        ? m_Socket
        : m_ClientSocket;

    if (Socket ==
        INVALID_SOCKET)
    {
        return false;
    }

    if (Packet.Data.size() >
        NETWORK_MAX_DATA_SIZE)
    {
        std::cout
            << "Packet too large."
            << std::endl;

        return false;
    }

    NetworkPacketHeader Header{};

    Header.Magic =
        htonl(
            NETWORK_MAGIC
        );

    Header.Version =
        htons(
            NETWORK_VERSION
        );

    Header.HeaderSize =
        htons(
            NETWORK_HEADER_SIZE
        );

    Header.IrpId =
        Packet.IrpId;

    Header.Bus =
        htons(
            Packet.Bus
        );

    Header.DeviceAddress =
        htons(
            Packet.DeviceAddress
        );

    Header.EndpointAddress =
        Packet.EndpointAddress;

    Header.TransferType =
        ToTransferType(
            Packet.TransferType
        );

    Header.Direction =
        ToDirection(
            Packet.Direction
        );

    Header.Reserved =
        0;

    Header.DataLength =
        htonl(
            static_cast<uint32_t>(
                Packet.Data.size()
                )
        );

    if (!SendAll(
        Socket,
        reinterpret_cast<const char*>(
            &Header
            ),
        sizeof(Header)
    ))
    {
        std::cout
            << "Failed to send network header."
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
                << "Failed to send network payload."
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
        m_Mode ==
        NetworkMode::Client
        ? m_Socket
        : m_ClientSocket;

    if (Socket ==
        INVALID_SOCKET)
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

    if (ntohl(
        Header.Magic
    ) != NETWORK_MAGIC)
    {
        std::cout
            << "Invalid network packet magic."
            << std::endl;

        return false;
    }

    if (ntohs(
        Header.Version
    ) != NETWORK_VERSION)
    {
        std::cout
            << "Unsupported network packet version."
            << std::endl;

        return false;
    }

    if (ntohs(
        Header.HeaderSize
    ) != NETWORK_HEADER_SIZE)
    {
        std::cout
            << "Invalid network header size."
            << std::endl;

        return false;
    }

    uint32_t DataLength =
        ntohl(
            Header.DataLength
        );

    if (DataLength >
        NETWORK_MAX_DATA_SIZE)
    {
        std::cout
            << "Received packet is too large."
            << std::endl;

        return false;
    }

    Packet.IrpId =
        Header.IrpId;

    Packet.Bus =
        ntohs(
            Header.Bus
        );

    Packet.DeviceAddress =
        ntohs(
            Header.DeviceAddress
        );

    Packet.EndpointAddress =
        Header.EndpointAddress;

    Packet.TransferType =
        FromTransferType(
            Header.TransferType
        );

    Packet.Direction =
        FromDirection(
            Header.Direction
        );

    Packet.DataLength =
        DataLength;

    Packet.Data.resize(
        DataLength
    );

    if (DataLength > 0)
    {
        if (!ReceiveAll(
            Socket,
            reinterpret_cast<char*>(
                Packet.Data.data()
                ),
            static_cast<int>(
                DataLength
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
    if (m_ClientSocket !=
        INVALID_SOCKET)
    {
        closesocket(
            m_ClientSocket
        );

        m_ClientSocket =
            INVALID_SOCKET;
    }

    if (m_Socket !=
        INVALID_SOCKET)
    {
        closesocket(
            m_Socket
        );

        m_Socket =
            INVALID_SOCKET;
    }

    WSACleanup();
}
// main_server.cpp : 애플리케이션의 진입점을 정의합니다.
//

#include "main.h"

#include <iostream>
#include <iomanip>

#include "NetworkTransport.h"

int main()
{
    NetworkTransport Network(
        NetworkMode::Server
    );

    if (!Network.Initialize())
    {
        return -1;
    }

    std::cout
        << "USBOverIP Server running."
        << std::endl;

    while (true)
    {
        USBPacket Packet;

        if (!Network.Read(Packet))
        {
            std::cout
                << "Connection closed."
                << std::endl;

            break;
        }

        std::cout
            << "================================"
            << std::endl;

        std::cout
            << "USB Packet Received"
            << std::endl;

        std::cout
            << "IRP       : 0x"
            << std::hex
            << Packet.IrpId
            << std::dec
            << std::endl;

        std::cout
            << "Bus       : "
            << Packet.Bus
            << std::endl;

        std::cout
            << "Device    : "
            << Packet.DeviceAddress
            << std::endl;

        std::cout
            << "Endpoint  : 0x"
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(
                Packet.EndpointAddress
                )
            << std::dec
            << std::endl;

        std::cout
            << "Data Len  : "
            << Packet.DataLength
            << std::endl;

        std::cout
            << "Data      : ";

        for (uint8_t Byte :
        Packet.Data)
        {
            std::cout
                << std::hex
                << std::setw(2)
                << std::setfill('0')
                << static_cast<int>(
                    Byte
                    )
                << ' ';
        }

        std::cout
            << std::dec
            << std::endl;
    }

    Network.Shutdown();

    return 0;
}
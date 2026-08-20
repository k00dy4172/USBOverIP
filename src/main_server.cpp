#include "main.h"

#include <iostream>

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
            << "Received "
            << Packet.Data.size()
            << " bytes : ";

        for (uint8_t Byte : Packet.Data)
        {
            std::cout
                << std::hex
                << static_cast<int>(Byte)
                << ' ';
        }

        std::cout
            << std::dec
            << std::endl;
    }

    Network.Shutdown();

    return 0;
}
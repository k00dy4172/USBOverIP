// main_client.cpp : 애플리케이션의 진입점을 정의합니다.
//

#include "main.h"

#include <iostream>

#include "NetworkTransport.h"
#include "USBPcapTransport.h"


int main()
{
    USBPcapTransport USBTransport;

    if (!USBTransport.Initialize())
    {
        return -1;
    }

    NetworkTransport Network(
        NetworkMode::Client
    );

    if (!Network.Initialize())
    {
        USBTransport.Shutdown();

        return -1;
    }

    std::cout
        << "USBPcap -> Network client running."
        << std::endl;

    while (true)
    {
        USBPacket Packet;

        if (USBTransport.Read(Packet))
        {
            if (!Network.Write(Packet))
            {
                std::cout
                    << "Network send failed."
                    << std::endl;

                break;
            }
        }

        Sleep(1);
    }

    Network.Shutdown();

    USBTransport.Shutdown();

    return 0;
}
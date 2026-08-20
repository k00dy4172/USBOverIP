// main.cpp : 애플리케이션의 진입점을 정의합니다.
//


#include "main.h"

#include <iostream>

#include "NetworkTransport.h"
#include "RawInputTransport.h"


int main()
{
    RawInputTransport InputTransport;

    if (!InputTransport.Initialize())
    {
        return -1;
    }

    NetworkTransport Network(
        NetworkMode::Client
    );

    if (!Network.Initialize())
    {
        return -1;
    }

    std::cout
        << "Raw Input -> Network Transport running."
        << std::endl;

    while (true)
    {
        MSG Message{};

        while (PeekMessage(
            &Message,
            nullptr,
            0,
            0,
            PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        USBPacket Packet;

        if (InputTransport.Read(Packet))
        {
            std::cout
                << "Sending "
                << Packet.Data.size()
                << " bytes."
                << std::endl;

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
    InputTransport.Shutdown();

    return 0;
}


//#include "main.h"

//#include <iostream>
//
//#include "NetworkTransport.h"
//
//int main()
//{
//    NetworkTransport Transport(
//        NetworkMode::Server
//    );
//
//    if (!Transport.Initialize())
//    {
//        return -1;
//    }
//
//    std::cout
//        << "Network Transport server running."
//        << std::endl;
//
//    while (true)
//    {
//        USBPacket Packet;
//
//        if (!Transport.Read(Packet))
//        {
//            std::cout
//                << "Connection closed."
//                << std::endl;
//
//            break;
//        }
//
//        std::cout
//            << "Received "
//            << Packet.Data.size()
//            << " bytes : ";
//
//        for (uint8_t Byte : Packet.Data)
//        {
//            std::cout
//                << std::hex
//                << static_cast<int>(Byte)
//                << ' ';
//        }
//
//        std::cout
//            << std::dec
//            << std::endl;
//    }
//
//    std::cin.get();
//
//    Transport.Shutdown();
//
//    return 0;
//}

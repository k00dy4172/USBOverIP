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
        << "USBOverIP Client running."
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
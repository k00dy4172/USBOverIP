#include "main.h"

#include <iostream>

#include "RawInputTransport.h"

int main()
{
    RawInputTransport Transport;

    if (!Transport.Initialize())
    {
        return -1;
    }

    std::cout
        << "Waiting for Raw Input..."
        << std::endl;

    MSG Message{};

    while (true)
    {
        while (PeekMessage(
            &Message,
            nullptr,
            0,
            0,
            PM_REMOVE))
        {
            if (Message.message == WM_QUIT)
            {
                Transport.Shutdown();
                return 0;
            }

            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        USBPacket Packet;

        while (Transport.Read(Packet))
        {
            std::cout
                << "Packet received!"
                << std::endl;

            std::cout
                << "Size : "
                << Packet.Data.size()
                << std::endl;
        }

        Sleep(1);
    }
}
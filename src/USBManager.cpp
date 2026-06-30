#include <libusb.h>
#include "USBManager.h"
#include <iostream>


USBManager::USBManager()
{
    m_Context = nullptr;
}

USBManager::~USBManager()
{
    Shutdown();
}

bool USBManager::Initialize()
{
    int Result = libusb_init(&m_Context); 

    if (Result != LIBUSB_SUCCESS)
    {
        std::cout << "Failed to initialize libusb." << std::endl;
        return false;
    }

    std::cout << "libusb initialized successfully." << std::endl;

    return true;
}

void USBManager::Shutdown()
{
    if (m_Context != nullptr)
    {
        libusb_exit(m_Context);

        m_Context = nullptr;

        std::cout << "libusb shutdown completed." << std::endl;
    }
}

void USBManager::EnumerateDevices()
{

}

#pragma once

#include <libusb.h>

class USBManager
{
public:

    USBManager();
    ~USBManager();

    bool Initialize();
    void Shutdown();

    void EnumerateDevices();

private:

    libusb_context* m_Context;
};
#pragma once

#include <cstdint>
#include <vector>

struct USBPacket
{
    uint8_t InterfaceNumber = 0;
    uint8_t EndpointAddress = 0;

    std::vector<uint8_t> Data;
};
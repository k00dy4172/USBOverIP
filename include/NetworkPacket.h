#pragma once

#include <cstdint>

#pragma pack(push, 1) //1바이트 단위로 구조체를 저장

struct NetworkPacketHeader
{
    uint32_t Magic;
    uint32_t DataSize;

    uint8_t InterfaceNumber;
    uint8_t EndpointAddress;
};

#pragma pack(pop)

constexpr uint32_t NETWORK_PACKET_MAGIC =
0x55534250;

constexpr uint32_t NETWORK_MAX_PACKET_SIZE =
4096;
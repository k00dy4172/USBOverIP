#pragma once

#include <cstdint>
#include <vector>

enum class USBTransferType
{
    Unknown,
    Isochronous,
    Interrupt,
    Control,
    Bulk
};

enum class USBTransferDirection
{
    Unknown,
    In,
    Out
};

struct USBPacket
{
    uint64_t IrpId = 0;

    uint16_t Bus = 0;

    uint16_t DeviceAddress = 0;

    uint8_t EndpointAddress = 0;

    USBTransferType TransferType =
        USBTransferType::Unknown;

    USBTransferDirection Direction =
        USBTransferDirection::Unknown;

    uint32_t DataLength = 0;

    std::vector<uint8_t> Data;
};
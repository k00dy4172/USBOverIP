#pragma once

#include "USBPacket.h"

class USBTransport
{
public:
    virtual ~USBTransport() = default;

    virtual bool Initialize() = 0;

    virtual bool Read(
        USBPacket& Packet
    ) = 0;

    virtual bool Write(
        const USBPacket& Packet
    ) = 0;

    virtual void Shutdown() = 0;
};
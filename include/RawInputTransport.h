#pragma once

#include <windows.h>
#include <queue>

#include "USBTransport.h"

class RawInputTransport :
    public USBTransport
{
public:

    RawInputTransport();
    ~RawInputTransport() override;

    bool Initialize() override;

    bool Read(
        USBPacket& Packet
    ) override;

    bool Write(
        const USBPacket& Packet
    ) override;

    void Shutdown() override;

private:

    static LRESULT CALLBACK WindowProc(
        HWND hWnd,
        UINT Message,
        WPARAM wParam,
        LPARAM lParam
    );

    bool RegisterRawInput();

    void ProcessRawInput(
        HRAWINPUT RawInputHandle
    );

private:

    HWND m_Window = nullptr;

    std::queue<USBPacket> m_PacketQueue;
};
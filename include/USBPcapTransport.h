#pragma once

#include <windows.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "USBTransport.h"

class USBPcapTransport :
    public USBTransport
{
public:

    USBPcapTransport();

    ~USBPcapTransport() override;

    bool Initialize() override;

    bool Read(
        USBPacket& Packet
    ) override;

    bool Write(
        const USBPacket& Packet
    ) override;

    void Shutdown() override;

private:

#pragma pack(push, 1)

    struct PcapGlobalHeader
    {
        uint32_t MagicNumber;
        uint16_t VersionMajor;
        uint16_t VersionMinor;
        int32_t  ThisZone;
        uint32_t SigFigs;
        uint32_t SnapLen;
        uint32_t Network;
    };

    struct PcapPacketHeader
    {
        uint32_t TimestampSeconds;
        uint32_t TimestampMicroseconds;
        uint32_t IncludedLength;
        uint32_t OriginalLength;
    };

    struct USBPcapHeader
    {
        uint16_t HeaderLength;
        uint64_t IrpId;
        uint32_t Status;
        uint16_t Function;
        uint8_t  Info;
        uint16_t Bus;
        uint16_t Device;
        uint8_t  Endpoint;
        uint8_t  Transfer;
        uint32_t DataLength;
    };

#pragma pack(pop)

    struct Capture
    {
        std::string DeviceName;

        HANDLE Process = nullptr;

        HANDLE Pipe = nullptr;

        std::thread Thread;

        std::atomic<bool> Running = false;

        bool PcapHeaderRead = false;
    };

private:

    bool FindDualSense();

    bool StartCapture();

    bool CreateUSBPcapProcess(
        HANDLE& Process,
        HANDLE& Pipe
    );

    void CaptureThread(
        Capture* CaptureContext
    );

    bool ReadExact(
        HANDLE Pipe,
        void* Buffer,
        DWORD Size
    );

    bool ReadPacket(
        Capture* CaptureContext,
        USBPacket& Packet
    );

    void PushPacket(
        USBPacket&& Packet
    );

private:

    uint16_t m_TargetBus = 0;

    uint16_t m_TargetDevice = 0;

    std::string m_TargetUSBPcap;

    Capture* m_Capture = nullptr;

    std::queue<USBPacket> m_PacketQueue;

    std::mutex m_QueueMutex;

    std::atomic<bool> m_Running = false;
};
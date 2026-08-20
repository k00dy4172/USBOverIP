#include "USBPcapTransport.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstring>

#include <libusb.h>

namespace
{
    constexpr uint16_t DUALSENSE_VID = 0x054C;
    constexpr uint16_t DUALSENSE_PID = 0x0CE6;

    constexpr uint32_t PCAP_MAGIC = 0xA1B2C3D4;

    constexpr const char* USBPCAP_CMD =
        "C:\\Program Files\\USBPcap\\USBPcapCMD.exe";

    USBTransferType ConvertTransferType(
        uint8_t Transfer
    )
    {
        switch (Transfer)
        {
        case 0:
            return USBTransferType::Isochronous;

        case 1:
            return USBTransferType::Interrupt;

        case 2:
            return USBTransferType::Control;

        case 3:
            return USBTransferType::Bulk;

        default:
            return USBTransferType::Unknown;
        }
    }
}

USBPcapTransport::USBPcapTransport()
{
}

USBPcapTransport::~USBPcapTransport()
{
    Shutdown();
}

bool USBPcapTransport::Initialize()
{
    std::cout
        << "Searching for DualSense..."
        << std::endl;

    if (!FindDualSense())
    {
        std::cout
            << "DualSense not found."
            << std::endl;

        return false;
    }

    std::cout
        << "================================"
        << std::endl;

    std::cout
        << "DualSense Found!"
        << std::endl;

    std::cout
        << "VID     : 0x"
        << std::hex
        << DUALSENSE_VID
        << std::endl;

    std::cout
        << "PID     : 0x"
        << DUALSENSE_PID
        << std::endl;

    std::cout
        << "Bus     : "
        << std::dec
        << m_TargetBus
        << std::endl;

    std::cout
        << "Device  : "
        << m_TargetDevice
        << std::endl;

    std::cout
        << "================================"
        << std::endl;

    /*
     * USB Bus 번호와 USBPcap 번호를 직접 대응시킨다.
     *
     * 예:
     *
     * Bus 1 -> USBPcap1
     * Bus 2 -> USBPcap2
     * Bus 3 -> USBPcap3
     */
    std::ostringstream USBPcapName;

    USBPcapName
        << "\\\\.\\USBPcap"
        << m_TargetBus;

    m_TargetUSBPcap =
        USBPcapName.str();

    std::cout
        << "Target USBPcap : "
        << m_TargetUSBPcap
        << std::endl;

    if (!StartCapture())
    {
        std::cout
            << "Failed to start USBPcap capture."
            << std::endl;

        return false;
    }

    m_Running = true;

    std::cout
        << "USBPcap transport initialized."
        << std::endl;

    return true;
}

bool USBPcapTransport::FindDualSense()
{
    libusb_context* Context = nullptr;

    int Result =
        libusb_init(&Context);

    if (Result != LIBUSB_SUCCESS)
    {
        std::cout
            << "libusb initialization failed."
            << std::endl;

        return false;
    }

    libusb_device** DeviceList = nullptr;

    ssize_t DeviceCount =
        libusb_get_device_list(
            Context,
            &DeviceList
        );

    if (DeviceCount < 0)
    {
        libusb_exit(Context);

        return false;
    }

    bool Found = false;

    for (ssize_t i = 0;
        i < DeviceCount;
        ++i)
    {
        libusb_device* Device =
            DeviceList[i];

        libusb_device_descriptor Descriptor{};

        Result =
            libusb_get_device_descriptor(
                Device,
                &Descriptor
            );

        if (Result != LIBUSB_SUCCESS)
        {
            continue;
        }

        if (Descriptor.idVendor !=
            DUALSENSE_VID)
        {
            continue;
        }

        if (Descriptor.idProduct !=
            DUALSENSE_PID)
        {
            continue;
        }

        m_TargetBus =
            libusb_get_bus_number(
                Device
            );

        m_TargetDevice =
            libusb_get_device_address(
                Device
            );

        Found = true;

        break;
    }

    libusb_free_device_list(
        DeviceList,
        1
    );

    libusb_exit(Context);

    return Found;
}

bool USBPcapTransport::CreateUSBPcapProcess(
    HANDLE& Process,
    HANDLE& Pipe
)
{
    SECURITY_ATTRIBUTES SecurityAttributes{};

    SecurityAttributes.nLength =
        sizeof(SecurityAttributes);

    SecurityAttributes.bInheritHandle =
        TRUE;

    HANDLE ReadPipe = nullptr;
    HANDLE WritePipe = nullptr;

    if (!CreatePipe(
        &ReadPipe,
        &WritePipe,
        &SecurityAttributes,
        0))
    {
        std::cout
            << "CreatePipe failed."
            << std::endl;

        return false;
    }

    /*
     * 부모 프로세스에서는 ReadPipe만 사용한다.
     * 따라서 ReadPipe는 자식에게 상속되지 않도록 한다.
     */
    if (!SetHandleInformation(
        ReadPipe,
        HANDLE_FLAG_INHERIT,
        0))
    {
        CloseHandle(ReadPipe);
        CloseHandle(WritePipe);

        return false;
    }

    /*
     * USBPcapCMD 실행 인자.
     *
     * -d:
     *     사용할 USBPcap 장치
     *
     * --devices:
     *     해당 Bus에서 캡처할 USB Device Address
     *
     * -o -:
     *     PCAP 데이터를 stdout으로 출력
     */
    std::ostringstream Arguments;

    Arguments
        << " -d "
        << m_TargetUSBPcap
        << " --devices "
        << m_TargetDevice
        << " -o -";

    std::string CommandLine =
        "\"" +
        std::string(USBPCAP_CMD) +
        "\"" +
        Arguments.str();

    std::vector<char> CommandBuffer(
        CommandLine.begin(),
        CommandLine.end()
    );

    CommandBuffer.push_back('\0');

    STARTUPINFOA StartupInfo{};

    StartupInfo.cb =
        sizeof(StartupInfo);

    StartupInfo.dwFlags =
        STARTF_USESTDHANDLES;

    StartupInfo.hStdInput =
        GetStdHandle(STD_INPUT_HANDLE);

    StartupInfo.hStdOutput =
        WritePipe;

    StartupInfo.hStdError =
        WritePipe;

    PROCESS_INFORMATION ProcessInfo{};

    BOOL Result =
        CreateProcessA(
            USBPCAP_CMD,
            CommandBuffer.data(),
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &StartupInfo,
            &ProcessInfo
        );

    /*
     * 부모는 WritePipe를 사용하지 않는다.
     */
    CloseHandle(WritePipe);

    if (!Result)
    {
        DWORD Error =
            GetLastError();

        std::cout
            << "CreateProcess failed: "
            << Error
            << std::endl;

        CloseHandle(ReadPipe);

        return false;
    }

    /*
     * ProcessInfo.hThread는 필요하지 않다.
     */
    CloseHandle(
        ProcessInfo.hThread
    );

    Process =
        ProcessInfo.hProcess;

    Pipe =
        ReadPipe;

    return true;
}

bool USBPcapTransport::StartCapture()
{
    HANDLE Process = nullptr;
    HANDLE Pipe = nullptr;

    if (!CreateUSBPcapProcess(
        Process,
        Pipe))
    {
        return false;
    }

    m_Capture =
        new Capture();

    m_Capture->DeviceName =
        m_TargetUSBPcap;

    m_Capture->Process =
        Process;

    m_Capture->Pipe =
        Pipe;

    m_Capture->Running =
        true;

    m_Capture->PcapHeaderRead =
        false;

    /*
     * USBPcapCMD 프로세스가 준비된 뒤
     * worker thread를 시작한다.
     */
    m_Running = true;

    m_Capture->Thread =
        std::thread(
            &USBPcapTransport::CaptureThread,
            this,
            m_Capture
        );

    std::cout
        << "Capture started : "
        << m_TargetUSBPcap
        << std::endl;

    std::cout
        << "Target Device   : "
        << m_TargetDevice
        << std::endl;

    return true;
}

void USBPcapTransport::CaptureThread(
    Capture* CaptureContext
)
{
    while (
        m_Running &&
        CaptureContext->Running)
    {
        USBPacket Packet;

        if (!ReadPacket(
            CaptureContext,
            Packet))
        {
            break;
        }

        /*
         * USBPcapCMD에서 이미 Device를 필터링했지만
         * 한 번 더 확인한다.
         */
        if (Packet.Bus !=
            m_TargetBus)
        {
            continue;
        }

        if (Packet.DeviceAddress !=
            m_TargetDevice)
        {
            continue;
        }

        PushPacket(
            std::move(Packet)
        );
    }

    CaptureContext->Running =
        false;
}

bool USBPcapTransport::ReadExact(
    HANDLE Pipe,
    void* Buffer,
    DWORD Size
)
{
    BYTE* Destination =
        static_cast<BYTE*>(Buffer);

    DWORD TotalRead = 0;

    while (TotalRead < Size)
    {
        DWORD BytesRead = 0;

        BOOL Result =
            ReadFile(
                Pipe,
                Destination + TotalRead,
                Size - TotalRead,
                &BytesRead,
                nullptr
            );

        if (!Result ||
            BytesRead == 0)
        {
            return false;
        }

        TotalRead +=
            BytesRead;
    }

    return true;
}

bool USBPcapTransport::ReadPacket(
    Capture* CaptureContext,
    USBPacket& Packet
)
{
    /*
     * USBPcapCMD는 PCAP 형식으로 출력한다.
     *
     * 최초 한 번만 Global Header를 읽는다.
     */
    if (!CaptureContext->PcapHeaderRead)
    {
        PcapGlobalHeader GlobalHeader{};

        if (!ReadExact(
            CaptureContext->Pipe,
            &GlobalHeader,
            sizeof(GlobalHeader)))
        {
            return false;
        }

        if (GlobalHeader.MagicNumber !=
            PCAP_MAGIC)
        {
            std::cout
                << "Invalid PCAP header."
                << std::endl;

            return false;
        }

        CaptureContext->PcapHeaderRead =
            true;
    }

    PcapPacketHeader PacketHeader{};

    if (!ReadExact(
        CaptureContext->Pipe,
        &PacketHeader,
        sizeof(PacketHeader)))
    {
        return false;
    }

    if (PacketHeader.IncludedLength <
        sizeof(USBPcapHeader))
    {
        return false;
    }

    std::vector<uint8_t> Buffer(
        PacketHeader.IncludedLength
    );

    if (!ReadExact(
        CaptureContext->Pipe,
        Buffer.data(),
        PacketHeader.IncludedLength))
    {
        return false;
    }

    USBPcapHeader Header{};

    std::memcpy(
        &Header,
        Buffer.data(),
        sizeof(Header)
    );

    if (Header.HeaderLength <
        sizeof(USBPcapHeader))
    {
        return false;
    }

    if (Header.HeaderLength >
        PacketHeader.IncludedLength)
    {
        return false;
    }

    uint32_t AvailableData =
        PacketHeader.IncludedLength -
        Header.HeaderLength;

    uint32_t ActualDataLength =
        Header.DataLength;

    if (ActualDataLength >
        AvailableData)
    {
        ActualDataLength =
            AvailableData;
    }

    Packet.IrpId =
        Header.IrpId;

    Packet.Bus =
        Header.Bus;

    Packet.DeviceAddress =
        Header.Device;

    Packet.EndpointAddress =
        Header.Endpoint;

    Packet.TransferType =
        ConvertTransferType(
            Header.Transfer
        );

    Packet.DataLength =
        ActualDataLength;

    Packet.Data.resize(
        ActualDataLength
    );

    if (ActualDataLength > 0)
    {
        std::memcpy(
            Packet.Data.data(),
            Buffer.data() +
            Header.HeaderLength,
            ActualDataLength
        );
    }

    /*
     * 테스트 단계에서 실제 캡처 내용을 확인한다.
     */
    std::cout
        << "================================"
        << std::endl;

    std::cout
        << "IRP       : 0x"
        << std::hex
        << Packet.IrpId
        << std::endl;

    std::cout
        << "Bus       : "
        << std::dec
        << Packet.Bus
        << std::endl;

    std::cout
        << "Device    : "
        << Packet.DeviceAddress
        << std::endl;

    std::cout
        << "Endpoint  : 0x"
        << std::hex
        << static_cast<int>(
            Packet.EndpointAddress
            )
        << std::endl;

    std::cout
        << "Data Len  : "
        << std::dec
        << Packet.DataLength
        << std::endl;

    std::cout
        << "Data      : ";

    for (uint8_t Byte :
    Packet.Data)
    {
        std::cout
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<int>(Byte)
            << ' ';
    }

    std::cout
        << std::dec
        << std::endl;

    return true;
}

void USBPcapTransport::PushPacket(
    USBPacket&& Packet
)
{
    std::lock_guard<std::mutex> Lock(
        m_QueueMutex
    );

    m_PacketQueue.push(
        std::move(Packet)
    );
}

bool USBPcapTransport::Read(
    USBPacket& Packet
)
{
    std::lock_guard<std::mutex> Lock(
        m_QueueMutex
    );

    if (m_PacketQueue.empty())
    {
        return false;
    }

    Packet =
        std::move(
            m_PacketQueue.front()
        );

    m_PacketQueue.pop();

    return true;
}

bool USBPcapTransport::Write(
    const USBPacket& Packet
)
{
    /*
     * 현재 단계에서는
     * USBPcap은 캡처 전용으로 사용한다.
     */
    return false;
}

void USBPcapTransport::Shutdown()
{
    m_Running = false;

    if (m_Capture == nullptr)
    {
        return;
    }

    m_Capture->Running = false;

    /*
     * USBPcapCMD 프로세스를 종료한다.
     */
    if (m_Capture->Process != nullptr)
    {
        TerminateProcess(
            m_Capture->Process,
            0
        );

        WaitForSingleObject(
            m_Capture->Process,
            1000
        );
    }

    /*
     * Pipe를 닫으면 worker thread의
     * ReadFile도 종료될 수 있다.
     */
    if (m_Capture->Pipe != nullptr)
    {
        CloseHandle(
            m_Capture->Pipe
        );

        m_Capture->Pipe =
            nullptr;
    }

    if (m_Capture->Thread.joinable())
    {
        m_Capture->Thread.join();
    }

    if (m_Capture->Process != nullptr)
    {
        CloseHandle(
            m_Capture->Process
        );

        m_Capture->Process =
            nullptr;
    }

    delete m_Capture;

    m_Capture =
        nullptr;

    /*
     * 남아 있는 패킷 제거.
     */
    std::lock_guard<std::mutex> Lock(
        m_QueueMutex
    );

    std::queue<USBPacket> Empty;

    m_PacketQueue.swap(
        Empty
    );
}
#include "RawInputTransport.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <utility>

RawInputTransport::RawInputTransport()
{
}

RawInputTransport::~RawInputTransport()
{
    Shutdown();
}

bool RawInputTransport::Initialize()
{
    WNDCLASSA WindowClass{};

    WindowClass.lpfnWndProc =
        RawInputTransport::WindowProc;

    WindowClass.hInstance =
        GetModuleHandleA(nullptr);

    WindowClass.lpszClassName =
        "RawInputTransportWindow";

    if (!RegisterClassA(&WindowClass))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            std::cout
                << "Failed to register window class."
                << std::endl;

            return false;
        }
    }

    m_Window =
        CreateWindowExA(
            0,                              // dwExStyle
            WindowClass.lpszClassName,      // lpClassName
            "RawInputTransport",            // lpWindowName
            0,                              // dwStyle
            0,                              // X
            0,                              // Y
            0,                              // Width
            0,                              // Height
            HWND_MESSAGE,                   // Parent
            nullptr,                         // Menu
            WindowClass.hInstance,          // Instance
            this                            // lpParam
        );

    if (m_Window == nullptr)
    {
        std::cout
            << "Failed to create hidden window."
            << std::endl;

        return false;
    }

    if (!RegisterRawInput())
    {
        DestroyWindow(m_Window);
        m_Window = nullptr;

        return false;
    }

    std::cout
        << "Raw Input Transport initialized."
        << std::endl;

    return true;
}

bool RawInputTransport::RegisterRawInput()
{
    RAWINPUTDEVICE Device{};

    Device.usUsagePage = 0x01;
    Device.usUsage = 0x05; // Game Pad

    Device.dwFlags =
        RIDEV_INPUTSINK;

    Device.hwndTarget =
        m_Window;

    if (!RegisterRawInputDevices(
        &Device,
        1,
        sizeof(Device)))
    {
        std::cout
            << "Failed to register Raw Input."
            << std::endl;

        return false;
    }

    std::cout
        << "Raw gamepad input registered."
        << std::endl;

    return true;
}
LRESULT CALLBACK RawInputTransport::WindowProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
)
{
    RawInputTransport* Transport =
        reinterpret_cast<RawInputTransport*>(
            GetWindowLongPtr(
                hWnd,
                GWLP_USERDATA
            )
            );

    if (Message == WM_NCCREATE)
    {
        CREATESTRUCT* Create =
            reinterpret_cast<CREATESTRUCT*>(lParam);

        Transport =
            static_cast<RawInputTransport*>(
                Create->lpCreateParams
                );

        SetWindowLongPtr(
            hWnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(Transport)
        );
    }

    if (Message == WM_INPUT &&
        Transport != nullptr)
    {
        Transport->ProcessRawInput(
            reinterpret_cast<HRAWINPUT>(lParam)
        );

        return 0;
    }

    return DefWindowProc(
        hWnd,
        Message,
        wParam,
        lParam
    );
}
void RawInputTransport::ProcessRawInput(
    HRAWINPUT RawInputHandle
)
{
    UINT Size = 0;

    UINT Result =
        GetRawInputData(
            RawInputHandle,
            RID_INPUT,
            nullptr,
            &Size,
            sizeof(RAWINPUTHEADER)
        );

    if (Result == static_cast<UINT>(-1))
    {
        return;
    }

    std::vector<BYTE> Buffer(Size);

    Result =
        GetRawInputData(
            RawInputHandle,
            RID_INPUT,
            Buffer.data(),
            &Size,
            sizeof(RAWINPUTHEADER)
        );

    if (Result == static_cast<UINT>(-1))
    {
        return;
    }

    RAWINPUT* RawInput =
        reinterpret_cast<RAWINPUT*>(
            Buffer.data()
            );

    if (RawInput->header.dwType != RIM_TYPEHID)
    {
        return;
    }

    const RAWHID& HID =
        RawInput->data.hid;

    USBPacket Packet;

    Packet.InterfaceNumber = 0;
    Packet.EndpointAddress = 0;

    Packet.Data.resize(
        HID.dwSizeHid * HID.dwCount
    );

    std::copy(
        HID.bRawData,
        HID.bRawData +
        HID.dwSizeHid * HID.dwCount,
        Packet.Data.begin()
    );

    m_PacketQueue.push(
        std::move(Packet)
    );

    std::cout
        << "Raw HID Packet Queued"
        << std::endl;

    std::cout
        << "Report Size : "
        << HID.dwSizeHid
        << std::endl;

    std::cout
        << "Report Count : "
        << HID.dwCount
        << std::endl;

    std::cout
        << "Queue Size : "
        << m_PacketQueue.size()
        << std::endl;
}

bool RawInputTransport::Read(
    USBPacket& Packet
)
{
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
bool RawInputTransport::Write(
    const USBPacket& Packet
)
{
    // 아직 구현하지 않음.
    // Raw Input은 입력 수집용이므로
    // 장치로 데이터를 보내는 기능은
    // 별도의 USB/Virtual Device 계층에서 구현한다.

    return false;
}

void RawInputTransport::Shutdown()
{
    if (m_Window != nullptr)
    {
        DestroyWindow(m_Window);
        m_Window = nullptr;
    }
}
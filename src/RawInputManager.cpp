#include "RawInputManager.h"

#include <iostream>

bool RawInputManager::Initialize()
{
    HINSTANCE Instance = GetModuleHandle(nullptr);

    WNDCLASSW WindowClass{};
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = L"RawInputHiddenWindow";

    if (!RegisterClassW(&WindowClass))
    {
        std::cout << "Failed to register window class."
            << std::endl;
        return false;
    }

    m_Window = CreateWindowExW(
        0,
        WindowClass.lpszClassName,
        L"Raw Input Hidden Window",
        0,
        0,
        0,
        0,
        0,
        nullptr,
        nullptr,
        Instance,
        nullptr
    );

    if (m_Window == nullptr)
    {
        std::cout << "Failed to create window."
            << std::endl;
        return false;
    }

    std::cout << "Hidden window created."
        << std::endl;

    RAWINPUTDEVICE RawInputDevice{};

    RawInputDevice.usUsagePage = 0x01; // Generic Desktop Controls
    RawInputDevice.usUsage = 0x05;     // GamePad
    RawInputDevice.dwFlags = RIDEV_INPUTSINK;
    RawInputDevice.hwndTarget = m_Window;

    if (!RegisterRawInputDevices(
        &RawInputDevice,
        1,
        sizeof(RAWINPUTDEVICE)))
    {
        std::cout
            << "Failed to register raw input device."
            << std::endl;

        return false;
    }

    std::cout
        << "Raw input registered."
        << std::endl;

    return true;
}

void RawInputManager::Run()
{
    MSG Message{};

    while (GetMessageW(&Message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&Message);
        DispatchMessageW(&Message);
    }
}

LRESULT CALLBACK RawInputManager::WindowProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_INPUT:
    {
        UINT Size = 0;

        UINT Result = GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam),
            RID_INPUT,
            nullptr,
            &Size,
            sizeof(RAWINPUTHEADER)
        );

        if (Result == static_cast<UINT>(-1))
        {
            std::cout
                << "Failed to get raw input size."
                << std::endl;

            break;
        }

        BYTE* Buffer = new BYTE[Size];

        Result = GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam),
            RID_INPUT,
            Buffer,
            &Size,
            sizeof(RAWINPUTHEADER)
        );

        if (Result == static_cast<UINT>(-1))
        {
            std::cout
                << "Failed to get raw input data."
                << std::endl;

            delete[] Buffer;
            break;
        }

        RAWINPUT* RawInput =
            reinterpret_cast<RAWINPUT*>(Buffer);

        if (RawInput->header.dwType == RIM_TYPEHID)
        {
            const RAWHID& HID =
                RawInput->data.hid;

            std::cout
                << "===================="
                << std::endl;

            std::cout
                << "HID Report Received"
                << std::endl;

            std::cout
                << "Report Size : "
                << HID.dwSizeHid
                << std::endl;

            std::cout
                << "Report Count : "
                << HID.dwCount
                << std::endl;

            for (DWORD Report = 0;
                Report < HID.dwCount;
                Report++)
            {
                const BYTE* Data =
                    HID.bRawData + (Report * HID.dwSizeHid);

                for (DWORD i = 0;
                    i < HID.dwSizeHid;
                    i++)
                {
                    std::cout
                        << std::hex
                        << std::uppercase
                        << static_cast<int>(Data[i])
                        << ' ';
                }

                std::cout
                    << std::dec
                    << std::endl;
            }
        }

        delete[] Buffer;

        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(
        hwnd,
        msg,
        wParam,
        lParam
    );
}
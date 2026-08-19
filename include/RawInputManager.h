#pragma once

#include <windows.h>

class RawInputManager
{
public:
    bool Initialize();
    void Run();

private:
    static LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    );

private:
    HWND m_Window = nullptr;
};
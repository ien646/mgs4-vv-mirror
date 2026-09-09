#include <d3d11.h>
#include <windows.h>
#include <wrl/module.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <imgui.h>

#include "hooks.hpp"
#include "overlay.hpp"
#include "tools.hpp"

namespace
{
    DWORD WINAPI InitThread(LPVOID)
    {
        Sleep(1000);
        hooks::initPresentHook();
        return 0;
    }
} // namespace

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);

        if (const auto thread = CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr); thread)
        {
            CloseHandle(thread);
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        HWND mainWindow = findGameWindow();
        if (hooks::getOriginalWndProc() && mainWindow)
            SetWindowLongPtr(mainWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hooks::getOriginalWndProc()));

        overlay::shutdown();
    }

    return TRUE;
}
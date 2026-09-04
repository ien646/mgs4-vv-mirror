#include "tools.hpp"

#include <windows.h>

HWND findGameWindow()
{
    struct EnumContextData
    {
        DWORD in_pid;
        HWND out_hwnd;
    };

    EnumContextData resultData{ .in_pid = GetCurrentProcessId(), .out_hwnd = nullptr };

    EnumWindows(
        [](HWND hwnd, LPARAM lParam) -> BOOL {
            auto* data = reinterpret_cast<EnumContextData*>(lParam);
            DWORD pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);

            if (pid == data->in_pid && IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == nullptr)
            {
                data->out_hwnd = hwnd;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&resultData));

    return resultData.out_hwnd;
}
#pragma once

#include <dxgi.h>
#include <windows.h>

using FnPresent = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT);

namespace hooks
{
    FnPresent getOriginalPresent();
    WNDPROC getOriginalWndProc();

    void initPresentHook();
    void initWndProcHook();
}
#include "hooks.hpp"

#include <d3d11.h>
#include <detours.h>
#include <imgui.h>

#include "overlay.hpp"
#include "tools.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
    FnPresent originalPresent = nullptr;
    WNDPROC originalWndProc = nullptr;

    LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        overlay::fix_mouse();

        return CallWindowProc(hooks::getOriginalWndProc(), hWnd, msg, wParam, lParam);
    }

    HRESULT STDMETHODCALLTYPE HookedPresent(IDXGISwapChain* swapchain, UINT syncInterval, UINT flags)
    {
        overlay::draw(swapchain);
        return originalPresent(swapchain, syncInterval, flags);
    }
} // namespace

FnPresent hooks::getOriginalPresent()
{
    return originalPresent;
}

WNDPROC hooks::getOriginalWndProc()
{
    return originalWndProc;
}

void hooks::initPresentHook()
{
    HWND mainWindow = nullptr;
    for (int i = 0; i < 100 && !mainWindow; ++i)
    {
        mainWindow = findGameWindow();
        if (!mainWindow)
        {
            Sleep(100);
        }
    }

    if (!mainWindow)
    {
        return;
    }

    IDXGISwapChain* dummySwapchain = nullptr;
    ID3D11Device* dummyDevice = nullptr;
    ID3D11DeviceContext* DummyContext = nullptr;

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = 2;
    sd.BufferDesc.Height = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = mainWindow;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    constexpr D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

    D3D_FEATURE_LEVEL obtainedLevel{};

    const HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &sd,
        &dummySwapchain,
        &dummyDevice,
        &obtainedLevel,
        &DummyContext);

    if (FAILED(hr) || !dummySwapchain)
    {
        return;
    }

    void** vtable = *reinterpret_cast<void***>(dummySwapchain);
    originalPresent = reinterpret_cast<FnPresent>(vtable[8]);

    if (DummyContext)
    {
        DummyContext->Release();
    }
    if (dummyDevice)
    {
        dummyDevice->Release();
    }
    if (dummySwapchain)
    {
        dummySwapchain->Release();
    }

    if (!originalPresent)
    {
        return;
    }

    auto error = DetourTransactionBegin();
    if (error != NO_ERROR)
    {
        return;
    }

    error = DetourUpdateThread(GetCurrentThread());
    if (error != NO_ERROR)
    {
        DetourTransactionAbort();
        return;
    }

    error = DetourAttach(reinterpret_cast<PVOID*>(&originalPresent), reinterpret_cast<PVOID>(HookedPresent));
    if (error != NO_ERROR)
    {
        DetourTransactionAbort();
        return;
    }

    error = DetourTransactionCommit();
}

void hooks::initWndProcHook()
{
    HWND mainWindow = findGameWindow();
    originalWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtr(mainWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HookedWndProc)));
}
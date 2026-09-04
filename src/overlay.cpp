#include "overlay.hpp"

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include "addresses.hpp"
#include "hooks.hpp"
#include "tools.hpp"

namespace
{
    bool imguiInitialized = false;
    bool showOverlay = true;

    ID3D11Device* dxDevice = nullptr;
    ID3D11DeviceContext* dxContext = nullptr;
    ID3D11RenderTargetView* dxRenderTargetView = nullptr;

    ImGuiContext* imguiCtx = nullptr;
} // namespace

namespace overlay
{
    void init(IDXGISwapChain* swapChain)
    {
        if (!imguiInitialized)
        {
            if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&dxDevice))) || !dxDevice)
                return;

            dxDevice->GetImmediateContext(&dxContext);
            if (!dxContext)
            {
                return;
            }

            ID3D11Texture2D* backBuffer = nullptr;
            if (SUCCEEDED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
            {
                std::ignore = dxDevice->CreateRenderTargetView(backBuffer, nullptr, &dxRenderTargetView);
                backBuffer->Release();
            }

            if (!dxRenderTargetView)
            {
                return;
            }

            imguiCtx = ImGui::CreateContext();
            ImGui::StyleColorsDark();

            HWND mainWindow = findGameWindow();

            ImGui_ImplWin32_Init(mainWindow);
            ImGui_ImplDX11_Init(dxDevice, dxContext);

            hooks::initWndProcHook();

            imguiInitialized = true;
        }
    }

    void drawOverlay()
    {
        const auto io = ImGui::GetIO();
        ImGui::GetStyle().FontScaleMain = 2.0F;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::SetNextWindowBgAlpha(0.0);

        const auto textSize = ImGui::CalcTextSize("Kills:00000");

        if (ImGui::Begin("mgs4-vv", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground))
        {
            const int kills = readAddress<int>(addresses::KILLS);
            ImGui::SetCursorPosX(io.DisplaySize.x - textSize.x);
            ImGui::Text("Kills:%d", kills);
        }
        ImGui::End();
    }

    void draw(IDXGISwapChain* swapchain)
    {
        init(swapchain);

        ImGui::SetCurrentContext(imguiCtx);
        dxContext->OMSetRenderTargets(1, &dxRenderTargetView, nullptr);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (ImGui::IsKeyPressed(ImGuiKey_Insert, false))
        {
            showOverlay = !showOverlay;
        }

        if (showOverlay)
        {
            drawOverlay();
        }

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void fix_mouse()
    {
        if (imguiInitialized)
        {
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
            if (io.WantCaptureMouse || io.WantCaptureKeyboard)
            {
                // ...
            }
        }
    }

    void shutdown()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::Shutdown();

        if (dxRenderTargetView)
        {
            dxRenderTargetView->Release();
            dxRenderTargetView = nullptr;
        }
        if (dxContext)
        {
            dxContext->Release();
            dxContext = nullptr;
        }
        if (dxDevice)
        {
            dxDevice->Release();
            dxDevice = nullptr;
        }
    }
} // namespace overlay
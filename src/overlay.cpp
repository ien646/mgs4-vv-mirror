#include "overlay.hpp"

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include "hooks.hpp"
#include "mgs4_addr.hpp"
#include "tools.hpp"

#include <cstdint>
#include <vector>

#define DEBUG_OVERLAY 0

namespace
{
    bool imguiInitialized = false;
    bool showOverlay = true;

    ID3D11Device* dxDevice = nullptr;
    ID3D11DeviceContext* dxContext = nullptr;
    ID3D11RenderTargetView* dxRenderTargetView = nullptr;

    ImGuiContext* imguiCtx = nullptr;

    float getFontScale()
    {
        return ImGui::GetIO().DisplaySize.y / 2160.0F;
    }

    template <typename... TArgs>
    void drawTextWithOutline(const float cursorPosX, const char* format, TArgs&&... args)
    {
        const auto y = ImGui::GetCursorPosY();
        ImGui::SetCursorPos(ImVec2(cursorPosX + 1, y + 1));
        ImGui::TextColored(ImVec4(0.0F, 0.0F, 0.0F, 1.0F), format, std::forward<TArgs>(args)...);

        ImGui::SetCursorPos(ImVec2(cursorPosX - 1, y - 1));
        ImGui::TextColored(ImVec4(0.0F, 0.0F, 0.0F, 1.0F), format, std::forward<TArgs>(args)...);

        ImGui::SetCursorPos(ImVec2(cursorPosX + 0, y + 0));
        ImGui::TextColored(ImVec4(1.0F, 1.0F, 0.5F, 1.0F), format, std::forward<TArgs>(args)...);
    }

    void drawOverlay()
    {
        auto& iio = ImGui::GetIO();
        iio.FontGlobalScale = getFontScale() * 2;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(iio.DisplaySize);
        ImGui::SetNextWindowBgAlpha(0.0);

        const auto textSizeKills = ImGui::CalcTextSize("Continues : 00000");
        const auto textSizeAddr = ImGui::CalcTextSize("Base addr (00): 0x00000000");
        const auto xPos = iio.DisplaySize.x - textSizeKills.x - 10;
        const auto yPos = iio.DisplaySize.y * 0.65F;

        if (ImGui::Begin(
                "mgs4-vv",
                nullptr,
                ImGuiWindowFlags_NoTitleBar
                    | ImGuiWindowFlags_NoBackground
                    | ImGuiWindowFlags_NoScrollbar
                    | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoMove))
        {
#if DEBUG_OVERLAY
            const auto debugXPos = iio.DisplaySize.x - ImGui::CalcTextSize("Base addr (HI): 0x0000000000000000").x - 10;

            drawTextWithOutline(debugXPos, "Base addr     : 0x%016llx", getBaseAddress());

            const auto kills_addr = getEffectiveAddress(mgs4_addr::KILLS_u16);
            const auto alerts_addr = getEffectiveAddress(mgs4_addr::ALERTS_u16);
            const auto continues_addr = getEffectiveAddress(mgs4_addr::CONTINUES_u16);
            const auto heals_addr = getEffectiveAddress(mgs4_addr::HEALS_u16);

            drawTextWithOutline(debugXPos, "Addr-kills    : 0x%016llx", kills_addr);
            drawTextWithOutline(debugXPos, "Addr-alerts   : 0x%016llx", alerts_addr);
            drawTextWithOutline(debugXPos, "Addr-continues: 0x%016llx", continues_addr);
            drawTextWithOutline(debugXPos, "Addr-heals    : 0x%016llx", heals_addr);

            drawTextWithOutline(debugXPos, "----------------------------------");
#endif

            const auto kills = readAddress<uint16_t>(mgs4_addr::KILLS_u16);
            const auto alerts = readAddress<uint16_t>(mgs4_addr::ALERTS_u16);
            const auto continues = readAddress<uint16_t>(mgs4_addr::CONTINUES_u16);
            const auto heals = readAddress<uint16_t>(mgs4_addr::HEALS_u16);

            ImGui::SetCursorPosY(yPos);

            drawTextWithOutline(xPos, "Kills     : %u", kills);
            drawTextWithOutline(xPos, "Alerts    : %u", alerts);
            drawTextWithOutline(xPos, "Continues : %u", continues);
            drawTextWithOutline(xPos, "Heals     : %u", heals);
        }
        ImGui::End();
    }
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
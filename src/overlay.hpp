#pragma once

#include <d3d11.h>

namespace overlay
{
    void init(IDXGISwapChain* swapChain);
    void draw(IDXGISwapChain* swapchain);
    void fix_mouse();
    void shutdown();
} // namespace overlay
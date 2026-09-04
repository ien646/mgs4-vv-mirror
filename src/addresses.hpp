#pragma once

#include <utility>

#include <windows.h>

constexpr auto PROC_NAME = "mgs4.exe";

enum class addresses : ptrdiff_t
{
    KILLS = 0x23FECA98
};

template <typename T>
T readAddress(addresses address)
{
    static auto base = reinterpret_cast<uintptr_t>(GetModuleHandleA(PROC_NAME));
    return *reinterpret_cast<T*>(base + std::to_underlying(address));
}
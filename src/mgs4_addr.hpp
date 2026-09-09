#pragma once

#include <utility>

#include <windows.h>

constexpr auto PROC_NAME = "mgs4.exe";

enum class mgs4_addr : ptrdiff_t
{
    KILLS_u16 = 0x23FECA98,
    ALERTS_u16 = 0x23FECA8E,
    CONTINUES_u16 = 0x23FECA78,
    HEALS_u16 = 0x23FED400,
};

uintptr_t getBaseAddress();
uintptr_t getEffectiveAddress(mgs4_addr addr);

template <typename T>
T readAddress(const mgs4_addr address)
{
    return *reinterpret_cast<T*>(getBaseAddress() + std::to_underlying(address));
}
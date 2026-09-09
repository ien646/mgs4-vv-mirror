#include "mgs4_addr.hpp"

uintptr_t getBaseAddress()
{
    return reinterpret_cast<uintptr_t>(GetModuleHandleA(PROC_NAME));
}

uintptr_t getEffectiveAddress(const mgs4_addr addr)
{
    return getBaseAddress() + std::to_underlying(addr);
}
#include "i_mem_alloc.hpp"
#include <Windows.h>

namespace {
    void* (__fastcall* g_alloc)(std::size_t) = nullptr;
    void  (__fastcall* g_free)(void*)        = nullptr;
}

bool InitMemAlloc() {
    HMODULE tier0 = GetModuleHandleA("tier0.dll");
    if (!tier0)
        return false;

    g_alloc = reinterpret_cast<decltype(g_alloc)>(GetProcAddress(tier0, "MemAlloc_AllocFunc"));
    g_free  = reinterpret_cast<decltype(g_free)> (GetProcAddress(tier0, "MemAlloc_FreeFunc"));

    return g_alloc && g_free;
}

void* GameAlloc(std::size_t size) {
    return g_alloc ? g_alloc(size) : nullptr;
}

void GameFree(void* ptr) {
    if (ptr && g_free)
        g_free(ptr);
}

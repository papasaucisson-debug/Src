#pragma once
#include <cstddef>

bool InitMemAlloc();
void* GameAlloc(std::size_t size);
void GameFree(void* ptr);

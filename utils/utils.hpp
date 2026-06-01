#pragma once

#include <cstdint>
#include <memory>
#include <Windows.h>
#include <vector>

inline bool valid_ptr(const void* p) {
	return reinterpret_cast<std::uintptr_t>(p) >= 0x10000;
}
inline bool valid_ptr(std::uintptr_t p) {
	return p >= 0x10000;
}

class c_opcodes {
public:
    std::vector<int> ida_to_bytes(const char*);
    unsigned char* scan(const char*, const char*);
    unsigned char* scan(const char*, const char*, int);
    unsigned char* scan_absolute(const char*, const char*);
    unsigned char* scan_absolute(const char*, const char*, int);
    unsigned char* scan_absolute(const char*, const char*, int, int);
    unsigned __int64 export_fn(unsigned __int64 base, const char* procedure_name);
    unsigned char* get_absolute_address(unsigned char*);
    unsigned char* get_absolute_address(unsigned char*, int);
    unsigned char* get_absolute_address(unsigned char*, int, int);
    unsigned char* resolve_relative_address(unsigned char*, int, int);
    unsigned char* resolve_relative_address(unsigned char*, int);
};

const auto g_opcodes = std::make_unique<c_opcodes>();

#pragma once
#include "../../../sdk/vfunc/vfunc.hpp"

struct c_schema_class_field {
    const char* m_name;
    char pad0[0x8];
    int m_offset;
    char pad1[0xC];
};
static_assert(sizeof(c_schema_class_field) == 0x20,
    "c_schema_class_field stride must match game layout (32 bytes)");

class c_schema_class_info {
public:
    const char* get_name() const {
        return *reinterpret_cast<const char* const*>((uintptr_t)this + 0x8);
    }

    uint16_t get_fields_size() {
        return *reinterpret_cast<uint16_t*>((std::uintptr_t)this + 0x24);
    }

    c_schema_class_field* get_fields() {
        return *reinterpret_cast<c_schema_class_field**>((std::uintptr_t)this + 0x30);
    }
};

class c_schema_type_scope {
public:
    c_schema_class_info* find_declared_class(const char* className) {
        c_schema_class_info* rv = nullptr;
        VFUNC(this, void, 2, &rv, className);
        return rv;
    }

    void* get_class_bind_table() const {
        return *reinterpret_cast<void* const*>((uintptr_t)this + 0x5C0);
    }
};

class i_schema_system {
public:
    c_schema_type_scope* find_type_scope_for_module(const char* moduleName) {
        return VFUNC(this, c_schema_type_scope*, 13, moduleName, nullptr);
    }
};

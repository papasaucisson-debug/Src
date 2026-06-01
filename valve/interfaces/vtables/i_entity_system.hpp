#pragma once

#include "../../modules/modules.hpp"
#include "../../../utils/utils.hpp"
#include "../../schema/schema.hpp"

class c_base_entity;

class i_entity_system {
public:
	template <class C = c_base_entity>
	C* get_base_entity(int index) {
		static auto get_client_entity = reinterpret_cast<C* (__fastcall*)(i_entity_system*, int)>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "4C 8D 49 ? 81 FA")
		);

		if (!get_client_entity)
			return nullptr;
		return get_client_entity(this, index);
	}

	void* get_local_pawn() {

		static auto fn = reinterpret_cast<void* (__fastcall*)(int)>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC ? 83 F9 ? 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 4C 8D 05")
		);
		if (fn)
			return fn(-1);
		return nullptr;
	}

	void* get_local_controller() {

		static auto fn = reinterpret_cast<void* (__fastcall*)(int)>(
			g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC ? 83 F9 ? 75 ? 48 8B 0D ? ? ? ? 48 8D 54 24 ? 48 8B 01 FF 90 ? ? ? ? 8B 08 48 63 C1 48 8D 0D ? ? ? ? 48 8B 04 C1 48 83 C4 ? C3 CC CC CC CC CC CC CC CC CC CC CC CC CC 48 83 EC ? 83 F9")
		);
		if (fn)
			return fn(-1);
		return nullptr;
	}

	int get_highest_entity_index() {
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x20A0);
	}
};

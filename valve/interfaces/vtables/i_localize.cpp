#include "../../../main.hpp"
#include "i_localize.hpp"

namespace {
	using create_interface_fn_t = void* (*)(const char* name, int* return_code);

	i_localize* resolve() {
		static i_localize* cached = nullptr;
		if (cached)
			return cached;

		const auto dll = reinterpret_cast<HMODULE>(g_modules->m_modules.localize_dll.get());
		if (!dll)
			return nullptr;

		const auto factory = reinterpret_cast<create_interface_fn_t>(
			GetProcAddress(dll, "CreateInterface"));
		if (!factory)
			return nullptr;

		cached = static_cast<i_localize*>(factory("Localize_001", nullptr));
		return cached;
	}
}

namespace localize {
	const char* find_safe(const char* token) {
		if (!token || !*token)
			return token ? token : "";
		auto* loc = resolve();
		if (!loc)
			return token;
		return loc->find_safe(token);
	}
}

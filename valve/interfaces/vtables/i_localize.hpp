#pragma once

#include "../../../sdk/vfunc/vfunc.hpp"

class i_localize {
public:
	const char* find_safe(const char* token) {
		if (!token)
			return "";
		const auto* result = vmt::call_virtual<const char*>(this, 18, token);
		return result ? result : token;
	}
};

namespace localize {

const char* find_safe(const char* token);

}

#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>

class i_game_event {
public:

	using GetNameFn = const char*(__fastcall*)(void*);
	using GetStringFn = const char*(__fastcall*)(void*, void*, void*);
	using SetStringFn = const char*(__fastcall*)(void*, void*, const char*, int);
	using GetIntFn = int(__fastcall*)(void*, void*, int);

	inline static GetNameFn get_name = nullptr;
	inline static GetStringFn get_string = nullptr;
	inline static SetStringFn set_string = nullptr;
	inline static GetIntFn get_int = nullptr;

	using GetPlayerControllerFn = void*(__fastcall*)(void*, void*);
	inline static GetPlayerControllerFn get_player_controller = nullptr;

	#define STRINGTOKEN_MURMURHASH_SEED 0x31415926

	static std::uint32_t MurmurHash2(const void* key, int len, std::uint32_t seed) {
		const std::uint32_t m = 0x5bd1e995;
		const int r = 24;
		std::uint32_t h = seed ^ len;
		const unsigned char* data = (const unsigned char*)key;

		while (len >= 4) {
			std::uint32_t k = *(std::uint32_t*)data;
			k *= m;
			k ^= k >> r;
			k *= m;
			h *= m;
			h ^= k;
			data += 4;
			len -= 4;
		}

		switch (len) {
			case 3: h ^= data[2] << 16; [[fallthrough]];
			case 2: h ^= data[1] << 8; [[fallthrough]];
			case 1: h ^= data[0];
				h *= m;
		}

		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;
		return h;
	}

	#define TOLOWERU(c) ((std::uint32_t)(((c >= 'A') && (c <= 'Z')) ? c + 32 : c))

	static std::uint32_t MurmurHash2LowerCaseA(const char* pString, int len, std::uint32_t nSeed) {
		char* p = (char*)malloc(len + 1);
		for (int i = 0; i < len; i++) {
			p[i] = static_cast<char>(TOLOWERU(pString[i]));
		}
		std::uint32_t result = MurmurHash2(p, len, nSeed);
		free(p);
		return result;
	}

	struct CUtlStringToken {
		std::uint32_t m_nHashCode;
		std::uint32_t pad;
		const char* m_szDebugName;

		CUtlStringToken() {
			m_nHashCode = 0;
			pad = 0xFFFFFFFF;
			m_szDebugName = nullptr;
		}

		CUtlStringToken(const char* szString) {
			m_nHashCode = MakeStringToken(szString);
			pad = 0xFFFFFFFF;
			m_szDebugName = szString;
		}

		std::uint32_t MakeStringToken(const char* szString, int nLen) {
			return MurmurHash2LowerCaseA(szString, nLen, STRINGTOKEN_MURMURHASH_SEED);
		}

		std::uint32_t MakeStringToken(const char* szString) {
			return MakeStringToken(szString, static_cast<int>(strlen(szString)));
		}
	};

	static_assert(sizeof(CUtlStringToken) == 16, "CUtlStringToken must be exactly 16 bytes");
};

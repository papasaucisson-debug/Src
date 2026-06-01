#pragma once

#include "config.hpp"

#include <Windows.h>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <dxgi.h>
#include <d3d11.h>

#ifdef _DEBUG
#define WINCALL(func) func
#else
#define WINCALL(func) LI_FN(func).cached()
#endif

#include "sdk/includes/lazy_importer.hpp"
#include "sdk/includes/xor.hpp"
#include "sdk/includes/hash.hpp"
#include "sdk/includes/minhook/MinHook.h"
#include "sdk/includes/imgui/imgui.h"
#include "sdk/includes/imgui/imgui_impl_dx11.h"
#include "sdk/includes/imgui/imgui_impl_win32.h"
#include "sdk/console/console.hpp"
#include "sdk/typedefs/vec_t.hpp"
#include "sdk/vfunc/vfunc.hpp"
#include "utils/utils.hpp"
#include "valve/modules/modules.hpp"
#include "valve/interfaces/interfaces.hpp"
#include "valve/schema/schema.hpp"
#include "menu/menu.hpp"
#include "hooks/hooks.hpp"

class c_user_cmd;
struct globals_t {
	c_user_cmd* m_user_cmd;
	void* m_local_pawn;
	void* m_local_controller;
};

inline const auto g_ctx = std::make_unique<globals_t>();

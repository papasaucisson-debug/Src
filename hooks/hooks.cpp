#include "../main.hpp"
#include "../directx/directx.hpp"
#include "../menu/menu.hpp"
#include "../features/shared/item_schema.hpp"
#include "../features/skin_changer/skin_changer.hpp"
#include "../features/glove_changer/glove_changer.hpp"
#include "../valve/interfaces/vtables/i_csgo_input.hpp"
#include "../valve/interfaces/vtables/i_game_event.hpp"
#include "../valve/classes/c_cs_player_pawn.hpp"
#include "../sdk/includes/hash.hpp"

using namespace hooks;

namespace event_hashes {
	constexpr uint32_t round_start = fnv1a::hash_32("round_start");
	constexpr uint32_t player_death = fnv1a::hash_32("player_death");
	constexpr uint32_t item_purchase = fnv1a::hash_32("item_purchase");
}

inline uint32_t hash_event_name(const char* str) {
	uint32_t hash = fnv1a::val_32_const;
	while (*str) {
		hash = (hash ^ static_cast<uint8_t>(*str)) * fnv1a::prime_32_const;
		str++;
	}
	return hash;
}

namespace knife_db {
	struct entry { std::uint16_t def; const char* full; };
	inline constexpr entry kSkinKnives[] = {
		{500, "weapon_bayonet"},                {503, "weapon_knife_css"},
		{505, "weapon_knife_flip"},             {506, "weapon_knife_gut"},
		{507, "weapon_knife_karambit"},         {508, "weapon_knife_m9_bayonet"},
		{509, "weapon_knife_tactical"},         {512, "weapon_knife_falchion"},
		{514, "weapon_knife_survival_bowie"},   {515, "weapon_knife_butterfly"},
		{516, "weapon_knife_push"},             {517, "weapon_knife_cord"},
		{518, "weapon_knife_canis"},            {519, "weapon_knife_ursus"},
		{520, "weapon_knife_gypsy_jackknife"},  {521, "weapon_knife_outdoor"},
		{522, "weapon_knife_stiletto"},         {523, "weapon_knife_widowmaker"},
		{525, "weapon_knife_skeleton"},         {526, "weapon_knife_kukri"},
	};

	inline const char* lookup(std::uint16_t def) {
		for (auto& e : kSkinKnives)
			if (e.def == def) return e.full;
		return nullptr;
	}

	inline bool matches(const char* name) {
		if (!name || !*name) return false;
		if (std::strncmp(name, "weapon_", 7) == 0) name += 7;
		if (std::strcmp(name, "knife") == 0 || std::strcmp(name, "knife_t") == 0)
			return true;
		for (auto& e : kSkinKnives)
			if (std::strcmp(name, e.full + 7) == 0) return true;
		return false;
	}
}

static const char* get_knife_weapon_name(int knife_index) {
	if (knife_index <= 0
		|| !g_item_schema->is_initialized()
		|| knife_index >= (int)g_item_schema->knives.size())
		return nullptr;
	return knife_db::lookup(g_item_schema->knives[knife_index].definition_index);
}

bool c_hooks::initialize() {
	MH_Initialize();

	create_move::m_create_move.hook(vmt::get_v_method(g_interfaces->m_csgo_input, 5), create_move::hk_create_move);
	mouse_input_enabled::m_mouse_input_enabled.hook(vmt::get_v_method(g_interfaces->m_csgo_input, 23), mouse_input_enabled::hk_mouse_input_enabled);
	enable_cursor::m_enable_cursor.hook(vmt::get_v_method(g_interfaces->m_input_system, 76), enable_cursor::hk_enable_cursor);

	frame_stage_notify::m_frame_stage_notify.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 89 5C 24 ? 48 89 6C 24 ? 57 48 83 EC 40 48 8B F9 33 ED"),
		frame_stage_notify::hk_frame_stage_notify
	);

	{
		i_game_event::get_name = reinterpret_cast<i_game_event::GetNameFn>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "8B 41 14 0F BA E0 1E 73 05 48 8D 41 18 C3"));
		i_game_event::get_string = reinterpret_cast<i_game_event::GetStringFn>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC 38 8B 02 48 83 C1 58 89 44 24 20 8B 42 04 89 44 24 24 48 8B 42 08 48 8D 54 24 20 48 89 44 24 28 E8 ? ? ? ? 48 83 C4 38 C3 CC CC CC 33 C9"));
		i_game_event::set_string = reinterpret_cast<i_game_event::SetStringFn>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC 38 8B 02 48 83 C1 58 89 44 24 20 41 B1 1A"));
		i_game_event::get_player_controller = reinterpret_cast<i_game_event::GetPlayerControllerFn>(g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "48 83 EC 38 8B 02 4C 8D 44 24 20"));
	}

	fire_event_client_side::m_fire_event_client_side.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "40 53 41 54 41 56 48 83 EC ? 4C 8B F2"),
		fire_event_client_side::hk_fire_event_client_side
	);

	level_init::m_level_init.hook(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "40 55 56 41 56 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 0D"),
		level_init::hk_level_init
	);

	if (g_directx->m_present_address)
		present::m_present.hook(g_directx->m_present_address, present::hk_present);
	if (g_directx->m_resize_buffers_address)
		resize_buffers::m_resize_buffers.hook(g_directx->m_resize_buffers_address, resize_buffers::hk_resize_buffers);
	if (g_directx->m_create_swap_chain_address)
		create_swap_chain::m_create_swap_chain.hook(g_directx->m_create_swap_chain_address, create_swap_chain::hk_create_swap_chain);

	return true;
}

void c_hooks::destroy() {
	if (g_menu->m_opened) {
		ImGui::GetIO().MouseDrawCursor = false;
		ShowCursor(TRUE);

		auto original = enable_cursor::m_enable_cursor.get_original<decltype(&enable_cursor::hk_enable_cursor)>();
		if (original) {
			__try {
				original(g_interfaces->m_input_system, enable_cursor::m_enable_cursor_input);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}
		}
	}

	g_menu->m_opened = true;

	present::m_present.unhook();
	resize_buffers::m_resize_buffers.unhook();
	create_swap_chain::m_create_swap_chain.unhook();

	Sleep(200);

	create_move::m_create_move.unhook();
	mouse_input_enabled::m_mouse_input_enabled.unhook();
	enable_cursor::m_enable_cursor.unhook();
	frame_stage_notify::m_frame_stage_notify.unhook();
	fire_event_client_side::m_fire_event_client_side.unhook();
	level_init::m_level_init.unhook();

	Sleep(100);

	g_directx->uninitialize();

	MH_Uninitialize();
}

bool __fastcall hooks::mouse_input_enabled::hk_mouse_input_enabled(void* ptr) {
	auto original = m_mouse_input_enabled.get_original<decltype(&hk_mouse_input_enabled)>();
	return g_menu->m_opened ? false : original(ptr);
}

void* __fastcall hooks::enable_cursor::hk_enable_cursor(void* rcx, bool active) {
	auto original = m_enable_cursor.get_original<decltype(&hk_enable_cursor)>();

	m_enable_cursor_input = active;
	if (g_menu->m_opened)
		active = false;

	return original(rcx, active);
}

void __fastcall hooks::create_move::hk_create_move(i_csgo_input* rcx, int slot, bool active) {
	auto original = m_create_move.get_original<decltype(&hk_create_move)>();

	original(rcx, slot, active);

	g_ctx->m_local_pawn = g_interfaces->m_entity_system->get_local_pawn();
	g_ctx->m_local_controller = g_interfaces->m_entity_system->get_local_controller();

	if (!g_ctx->m_local_controller)
		return;

	g_ctx->m_user_cmd = rcx->get_user_cmd(g_ctx->m_local_controller);

	g_menu->on_create_move();
}

void hooks::frame_stage_notify::hk_frame_stage_notify(void* source_to_client, int stage) {
	auto original = m_frame_stage_notify.get_original<decltype(&hk_frame_stage_notify)>();

	if (g_interfaces && g_interfaces->m_entity_system) {
		g_ctx->m_local_pawn = g_interfaces->m_entity_system->get_local_pawn();
		g_ctx->m_local_controller = g_interfaces->m_entity_system->get_local_controller();
	}

	if (stage == 7) {
		g_skin_changer->run(stage);
		g_glove_changer->run(stage);
	}

	original(source_to_client, stage);
}

__int64 __fastcall hooks::level_init::hk_level_init(void* rcx, void* rdx) {
	auto original = m_level_init.get_original<decltype(&hk_level_init)>();

	if (g_cfg->knife_changer.m_enabled || g_cfg->skin_changer.m_enabled) {
		g_skin_changer->should_update = true;
	}

	if (g_cfg->glove_changer.m_enabled) {
		g_glove_changer->should_update = true;
	}

	return original(rcx, rdx);
}

bool __fastcall hooks::fire_event_client_side::hk_fire_event_client_side(void* p_game_event_manager, void* p_game_event) {
	auto original = m_fire_event_client_side.get_original<decltype(&hk_fire_event_client_side)>();

	if (!p_game_event || !i_game_event::get_name)
		return original(p_game_event_manager, p_game_event);

	const char* event_name = i_game_event::get_name(p_game_event);
	if (!event_name)
		return original(p_game_event_manager, p_game_event);

	const uint32_t event_hash = hash_event_name(event_name);

	if ((event_hash == event_hashes::round_start || event_hash == event_hashes::item_purchase))
	{
		g_skin_changer->should_update |= g_cfg->knife_changer.m_enabled || g_cfg->skin_changer.m_enabled;
		g_glove_changer->should_update |= g_cfg->glove_changer.m_enabled;
		return original(p_game_event_manager, p_game_event);
	}

	if (event_hash != event_hashes::player_death || !g_cfg->knife_changer.m_enabled)
		return original(p_game_event_manager, p_game_event);

	if (!g_ctx->m_local_controller || !i_game_event::get_player_controller)
		return original(p_game_event_manager, p_game_event);

	i_game_event::CUtlStringToken attacker_token("attacker");
	attacker_token.pad = 0xFFFFFFFF;

	void* attacker_controller = i_game_event::get_player_controller(p_game_event, &attacker_token);

	if (attacker_controller != g_ctx->m_local_controller)
		return original(p_game_event_manager, p_game_event);

	i_game_event::CUtlStringToken weapon_token("weapon");
	weapon_token.pad = 0xFFFFFFFF;

	const char* weapon_name = i_game_event::get_string(p_game_event, &weapon_token, nullptr);

	if (!valid_ptr(weapon_name))
		return original(p_game_event_manager, p_game_event);

	const bool is_knife = knife_db::matches(weapon_name);

	if (is_knife && g_cfg->knife_changer.m_knife != 0) {
		const char* new_weapon_name = get_knife_weapon_name(g_cfg->knife_changer.m_knife);
		if (new_weapon_name) {
			i_game_event::CUtlStringToken set_token("weapon");
			set_token.pad = 0xFFFFFFFF;
			i_game_event::set_string(p_game_event, &set_token, new_weapon_name, 0);
		}
	}

	return original(p_game_event_manager, p_game_event);
}

HRESULT hooks::present::hk_present(IDXGISwapChain* swap_chain, unsigned int sync_interval, unsigned int flags) {
	auto original = m_present.get_original<decltype(&hk_present)>();

	g_directx->start_frame(swap_chain);

	auto* device_context = g_directx->get_device_context();
	auto* render_target = g_directx->get_render_target();
	if (device_context && render_target) {
		device_context->OMSetRenderTargets(1, &render_target, nullptr);

		g_directx->new_frame();

		g_menu->draw();
		g_menu->draw_toasts();
		g_directx->end_frame();
	}

	return original(swap_chain, sync_interval, flags);
}

HRESULT hooks::resize_buffers::hk_resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags) {
	auto original = m_resize_buffers.get_original<decltype(&hk_resize_buffers)>();
	return original(swap_chain, buffer_count, width, height, new_format, swap_chain_flags);
}

HRESULT __stdcall hooks::create_swap_chain::hk_create_swap_chain(IDXGIFactory* factory, IUnknown* device, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swap_chain) {
	auto original = m_create_swap_chain.get_original<decltype(&hk_create_swap_chain)>();

	g_directx->destroy_render_target();
	return original(factory, device, desc, swap_chain);
}

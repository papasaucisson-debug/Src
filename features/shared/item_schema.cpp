#include "item_schema.hpp"
#include "../../valve/interfaces/interfaces.hpp"
#include "../../valve/interfaces/vtables/i_localize.hpp"
#include "../../valve/classes/c_cs_player_pawn.hpp"

bool c_item_schema::is_paint_kit_for_item(const char* simple_weapon_name, c_paint_kit* paint_kit) {
	if (!simple_weapon_name || !paint_kit || !paint_kit->m_name)
		return false;

	std::string path = "panorama/images/econ/default_generated/" +
		std::string(simple_weapon_name) + "_" +
		paint_kit->m_name + "_light_png.vtex_c";

	return g_interfaces->m_file_system->exists(path.c_str(), "GAME");
}

void c_item_schema::build_paint_kits_for_item(uint16_t def_index, c_utl_map<int, c_econ_item_definition*>& items, c_utl_map<int, c_paint_kit*>& paint_kit_map) {
	c_econ_item_definition* item_def = nullptr;
	const int items_n = items.count();
	for (int i = 0; i < items_n; i++) {
		auto& node = items.element(i);
		if (node.m_value && node.m_value->m_definition_index == def_index) {
			item_def = node.m_value;
			break;
		}
	}

	if (!item_def)
		return;

	const char* simple_name = item_def->get_item_name();
	if (!simple_name || simple_name[0] == '\0')
		return;

	item_paint_kits[def_index].push_back({ 0, "Default" });

	const int kits_n = paint_kit_map.count();
	for (int i = 0; i < kits_n; i++) {
		auto& node = paint_kit_map.element(i);
		if (!node.m_value)
			continue;

		c_paint_kit* kit = node.m_value;
		if (!kit->m_name || kit->m_id <= 0)
			continue;

		if (is_paint_kit_for_item(simple_name, kit)) {

			const char* display = localize::find_safe(kit->m_description_tag);
			if (!display || !*display)
				display = kit->m_name;
			item_paint_kits[def_index].push_back({ kit->m_id, display });
		}
	}
}

void c_item_schema::initialize() {
	if (m_initialized)
		return;

	auto* item_system = g_interfaces->m_source2_client->get_econ_item_system();
	if (!item_system)
		return;

	auto* item_schema = item_system->get_econ_item_schema();
	if (!item_schema)
		return;

	auto& items = item_schema->get_sorted_item_definition_map();
	auto& paint_kit_map = item_schema->get_paint_kits();

	const int items_n = items.count();
	for (int i = 0; i < items_n; i++) {
		auto& node = items.element(i);
		if (!node.m_value || !node.m_value->m_item_type_name)
			continue;

		c_econ_item_definition* item_def = node.m_value;

		std::string item_name;
		if (item_def->m_item_base_name && item_def->m_item_base_name[0] != '\0') {
			const char* localized = localize::find_safe(item_def->m_item_base_name);
			item_name = (localized && *localized) ? localized : item_def->m_item_base_name;
		} else {
			item_name = "Item " + std::to_string(item_def->m_definition_index);
		}

		const char* model_path = item_def->get_model_name();

		if (item_def->is_knife(false)) {
			knives.push_back({ item_def->m_definition_index, item_name, model_path });
		}
		else if (item_def->is_glove(false)) {
			gloves.push_back({ item_def->m_definition_index, item_name, model_path });
		}
		else {
			const char* type_name = item_def->m_item_type_name;
			bool is_weapon = strstr(type_name, "Pistol") || strstr(type_name, "Rifle") ||
			                 strstr(type_name, "SMG") || strstr(type_name, "Sniper") ||
			                 strstr(type_name, "Shotgun") || strstr(type_name, "Machine") ||
			                 strstr(type_name, "SubMachinegun");

			if (is_weapon && item_def->m_definition_index > 0 && item_def->m_definition_index <= 70)
				weapons.push_back({ item_def->m_definition_index, item_name, model_path });
		}
	}

	std::sort(weapons.begin(), weapons.end(), [](const item_info_t& a, const item_info_t& b) {
		return a.definition_index < b.definition_index;
	});

	for (auto& knife : knives)
		if (knife.definition_index != 0)
			build_paint_kits_for_item(knife.definition_index, items, paint_kit_map);

	for (auto& glove : gloves)
		if (glove.definition_index != 0)
			build_paint_kits_for_item(glove.definition_index, items, paint_kit_map);

	for (auto& weapon : weapons)
		if (weapon.definition_index != 0)
			build_paint_kits_for_item(weapon.definition_index, items, paint_kit_map);

	for (auto& knife : knives)
		knife_names_cstr.push_back(knife.name.c_str());
	for (auto& glove : gloves)
		glove_names_cstr.push_back(glove.name.c_str());
	for (auto& weapon : weapons)
		weapon_names_cstr.push_back(weapon.name.c_str());

	for (auto& [def_index, kits] : item_paint_kits)
		for (auto& kit : kits)
			item_paint_kit_names[def_index].push_back(kit.name.c_str());

	m_initialized = true;
	LOG_INFO(xorstr_("[item_schema] %d knives, %d gloves, %d weapons"),
		(int)knives.size() - 1, (int)gloves.size() - 1, (int)weapons.size());
}

std::uintptr_t c_hud::find_hud_element(const char* name) {

	using fn_t = std::uintptr_t(__fastcall*)(const char*);
	static auto fn = reinterpret_cast<fn_t>(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(),
			"4C 8B DC 53 48 83 EC ? 48 8B 05")
	);
	return fn ? fn(name) : 0;
}

void c_hud::clear_hud_weapon_icon(std::uintptr_t hud_weapons, std::int32_t index, std::int64_t unk) {
	static auto call_address = g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 8B F8 C6 84 24");
	if (!call_address)
		return;

	auto fn = reinterpret_cast<std::int64_t(__fastcall*)(std::uintptr_t, std::int32_t, std::int64_t)>(
		reinterpret_cast<std::uintptr_t>(call_address) + 5 + *reinterpret_cast<std::int32_t*>(call_address + 1)
	);
	fn(hud_weapons, index, unk);
}

struct hud_weapon_panel_t {
	std::uintptr_t base  = 0;
	std::uintptr_t data  = 0;
	std::int32_t   count = 0;
};

static bool resolve_weapon_panel(hud_weapon_panel_t& out) {
	const auto hud = c_hud::find_hud_element("HudWeaponSelection");
	if (!valid_ptr(hud))
		return false;

	out.base  = hud - 0x98;
	out.data  = *reinterpret_cast<std::uintptr_t*>(out.base + 0x58);
	out.count = *reinterpret_cast<std::int32_t*>  (out.base + 0x50);
	return valid_ptr(out.data) && out.count > 0 && out.count <= 64;
}

void c_hud::clear_hud_weapon_icons() {
	__try {
		hud_weapon_panel_t panel;
		if (!resolve_weapon_panel(panel))
			return;
		for (std::int32_t i = panel.count - 1; i >= 0; --i)
			clear_hud_weapon_icon(panel.base, i, 0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

void c_hud::clear_hud_weapon_icon_for(c_base_entity* weapon) {
	if (!valid_ptr(weapon))
		return;
	__try {
		hud_weapon_panel_t panel;
		if (!resolve_weapon_panel(panel))
			return;

		auto* es = g_interfaces->m_entity_system;
		for (std::int32_t i = panel.count - 1; i >= 0; --i) {
			const auto handle = *reinterpret_cast<std::int32_t*>(panel.data + 72 * i + 0x38);
			if (handle < 0)
				continue;
			if (es->get_base_entity(handle & 0x7FFF) == weapon) {
				clear_hud_weapon_icon(panel.base, i, 0);
				return;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void invoke_regen(void(__fastcall* fn)()) {
	__try {
		fn();
	} __except (EXCEPTION_EXECUTE_HANDLER) {}
}

void c_hud::regenerate_skins() {

	static auto fn = reinterpret_cast<void(__fastcall*)()>(
		g_opcodes->scan(g_modules->m_modules.client_dll.get_name(),
			"48 83 EC ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B 10"));
	if (fn)
		invoke_regen(fn);
}

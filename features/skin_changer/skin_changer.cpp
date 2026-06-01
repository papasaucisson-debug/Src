#include "skin_changer.hpp"
#include "../shared/econ_item_attribute_manager.hpp"
#include "../shared/item_schema.hpp"
#include "../../valve/interfaces/interfaces.hpp"
#include "../../valve/schema/schema.hpp"
#include "../../valve/interfaces/vtables/i_econ_item_system.hpp"
#include "../../menu/menu.hpp"
#include <vector>
#include <utility>

// 辅助函数：获取武器的 HUD 视图实体
c_base_entity* c_skin_changer::get_hud_weapon(c_base_entity* weapon, c_cs_player_pawn* local_pawn) {
	auto arms_handle = local_pawn->m_hud_model_arms();
	if (!arms_handle.is_valid())
		return nullptr;

	auto* hud_arms = reinterpret_cast<c_base_entity*>(
		g_interfaces->m_entity_system->get_base_entity(arms_handle.get_entry_index())
		);
	if (!valid_ptr(hud_arms))
		return nullptr;

	auto* arms_node = hud_arms->m_scene_node();
	if (!valid_ptr(arms_node))
		return nullptr;

	for (auto* vm = arms_node->m_child(); valid_ptr(vm); vm = vm->m_next_sibling()) {
		auto* vm_owner = vm->m_owner();
		if (!valid_ptr(vm_owner))
			continue;

		auto* vm_entity = reinterpret_cast<c_base_entity*>(vm_owner);
		auto owner_handle = vm_entity->m_owner_entity();
		if (!owner_handle.is_valid())
			continue;

		if (g_interfaces->m_entity_system->get_base_entity(owner_handle.get_entry_index()) == weapon)
			return vm_entity;
	}
	return nullptr;
}

// 应用皮肤逻辑
void c_skin_changer::apply_skin(c_econ_entity* weapon, c_econ_item_view* item, int paint_kit_id, float wear, int seed, const char* custom_name, c_cs_player_pawn* local_pawn, uint16_t def_index)
{
	auto* controller = local_pawn->get_controller();
	uint32_t local_account_id = controller ? (uint32_t)controller->m_steam_id() : 0;

	// --- 使用你类中定义的名称进行赋值 ---
	item->m_item_id_high() = 0xFFFFFFFF;
	item->m_item_id_low() = 1337;
	item->m_item_id() = 1337;            //m_iItemID
	item->m_account_id() = local_account_id; //m_iAccountID
	item->m_initialized() = true;            //m_bInitialized

	weapon->m_paint_kit() = paint_kit_id;
	weapon->m_wear() = wear;
	weapon->m_seed() = seed;
	/*econ_item_attribute_manager::remove(item);*/
	econ_item_attribute_manager::create(item, paint_kit_id, wear, seed);


	if (custom_name && custom_name[0] != '\0')
		strcpy_s(item->m_custom_name(), 161, custom_name);

	bool uses_old_model = false;
	c_paint_kit* pk = nullptr;
	if ((pk = g_interfaces->m_source2_client->get_econ_item_system()->get_econ_item_schema()->get_paint_kits().find_by_key(paint_kit_id)))
		uses_old_model = pk->uses_old_model();

	uint64_t mesh_mask = uses_old_model ? 2 : 1;

	if (auto* scene_node = weapon->m_scene_node())
		scene_node->set_mesh_group_mask(mesh_mask);

	if (auto* hud_weapon = get_hud_weapon(weapon, local_pawn))
		if (auto* hud_node = hud_weapon->m_scene_node())
			hud_node->set_mesh_group_mask(mesh_mask);

	weapon->update_skin(true);
	weapon->update_weapon_data();
	item->m_name_description_ptr() = 0;
}


void c_skin_changer::initialize() {
	if (m_initialized)
		return;

	if (!g_item_schema->is_initialized())
		g_item_schema->initialize();

	m_initialized = g_item_schema->is_initialized();
}

// 处理普通武器换肤
void c_skin_changer::process_weapon(c_econ_entity* weapon, c_econ_item_view* item, c_cs_player_pawn* local_pawn, bool force_update, bool& did_update, uint64_t local_steam_id) {
	// --- 关键修复：验证武器所有权 ---
	if (weapon->get_original_owner_xuid() != local_steam_id)
		return;
	auto* weapon_service = local_pawn->m_weapon_services();
	if (!valid_ptr(weapon_service))
		return;
	auto& my_weapons = weapon_service->my_weapons();


	uint16_t def_index = item->m_definition_index();
	int config_index = c_config::skin_changer_t::get_config_index(def_index);
	if (config_index == 0)
		return;

	auto& skin = g_cfg->skin_changer.weapon_skins[config_index];
	if (skin.paint_kit == 0)
		return;

	int paint_kit_id = g_item_schema->get_paint_kit_id_for_item(def_index, skin.paint_kit);
	if (paint_kit_id == 0 || (weapon->m_paint_kit() == paint_kit_id && !force_update))
		return;

	apply_skin(weapon, item, paint_kit_id, skin.wear, skin.seed, skin.custom_name, local_pawn, def_index);
	c_hud::clear_hud_weapon_icon_for(weapon);
	did_update = true;
}

// 处理匕首换肤
void c_skin_changer::process_knife(c_econ_entity* weapon, c_econ_item_view* item, c_cs_player_pawn* local_pawn, bool force_update, bool& did_update, uint64_t local_steam_id) {
	// --- 关键修复：验证武器所有权 ---
	auto* weapon_service = local_pawn->m_weapon_services();
	if (!valid_ptr(weapon_service))
		return;
	auto& my_weapons = weapon_service->my_weapons();

	if (g_cfg->knife_changer.m_knife == 0)
		return;
	if (!g_item_schema->is_initialized()
		|| g_cfg->knife_changer.m_knife >= (int)g_item_schema->knives.size())
		return;

	const uint16_t def_index = item->m_definition_index();
	const uint16_t selected_knife = g_item_schema->knives[g_cfg->knife_changer.m_knife].definition_index;
	if (selected_knife == 0)
		return;

	int paint_kit_id = g_item_schema->get_paint_kit_id_for_item(selected_knife, g_cfg->knife_changer.m_paint_kit);
	bool config_changed = (m_last_knife != selected_knife) ||
		(m_last_knife_paint_kit_id != paint_kit_id) ||
		(m_last_knife_wear != g_cfg->knife_changer.m_wear) ||
		(m_last_knife_seed != g_cfg->knife_changer.m_seed);

	if (def_index == selected_knife && !config_changed && !force_update)
		return;

	item->m_definition_index() = selected_knife;
	item->m_entity_quality() = QUALITY_UNUSUAL;

	if (const char* model_path = g_item_schema->knives[g_cfg->knife_changer.m_knife].model_path) {
		weapon->set_model(model_path);
		if (auto* hud_weapon = get_hud_weapon(weapon, local_pawn))
			hud_weapon->set_model(model_path);
	}

	econ_item_attribute_manager::remove(item);
	if (paint_kit_id > 0)
		econ_item_attribute_manager::create(item, paint_kit_id, g_cfg->knife_changer.m_wear, g_cfg->knife_changer.m_seed);

	bool uses_old_model = false;
	if (paint_kit_id > 0)
		if (auto* pk = g_interfaces->m_source2_client->get_econ_item_system()->get_econ_item_schema()->get_paint_kits().find_by_key(paint_kit_id))
			uses_old_model = pk->uses_old_model();

	uint64_t mesh_mask = uses_old_model ? 1 : 2;
	if (auto* scene_node = weapon->m_scene_node())
		scene_node->set_mesh_group_mask(mesh_mask);
	if (auto* hud_weapon = get_hud_weapon(weapon, local_pawn))
		if (auto* hud_node = hud_weapon->m_scene_node())
			hud_node->set_mesh_group_mask(mesh_mask);

	if (g_cfg->knife_changer.m_custom_name[0] != '\0')
		strcpy_s(item->m_custom_name(), 161, g_cfg->knife_changer.m_custom_name);
	else
		item->m_custom_name()[0] = '\0';


	weapon->update_subclass(selected_knife);
	weapon->update_skin(true);
	weapon->update_weapon_data();
	item->m_name_description_ptr() = 0;


	m_last_knife = selected_knife;
	m_last_knife_paint_kit_id = paint_kit_id;
	m_last_knife_wear = g_cfg->knife_changer.m_wear;
	m_last_knife_seed = g_cfg->knife_changer.m_seed;
	c_hud::clear_hud_weapon_icon_for(weapon);
	did_update = true;
}

void c_skin_changer::run(int stage) {
	// 1. 基础开关与阶段检查
	const bool skin_enabled = g_cfg->skin_changer.m_enabled;
	const bool knife_enabled = g_cfg->knife_changer.m_enabled;

	if ((!skin_enabled && !knife_enabled) || stage != 7 || !g_ctx->m_local_pawn)
		return;

	// 2. 本地玩家基础检查
	auto* local_pawn = reinterpret_cast<c_cs_player_pawn*>(g_ctx->m_local_pawn);
	if (!valid_ptr(local_pawn) || local_pawn->m_health() <= 0)
		return;

	// 3. 状态提取
	auto* controller = local_pawn->get_controller();
	if (!controller) return;

	const float current_spawn_time = local_pawn->m_last_spawn_time_index();
	const int   current_team = local_pawn->m_team_num();
	//const bool  buy_menu_open = local_pawn->m_is_buy_menu_open();
	uint64_t    local_steam_id = controller->m_steam_id();

	// 4. 计算状态变化
	const bool team_changed = (current_team != m_last_team) && m_last_team != 0;
	const bool spawn_changed = (current_spawn_time != m_last_spawn_time) && m_last_spawn_time != 0.0f;

	// --- 新增：检测配置参数变动 (临时内存变动) ---
	static auto last_cfg_skin = g_cfg->skin_changer;
	static auto last_cfg_knife = g_cfg->knife_changer;

	// 简单的内存对比，检测配置是否被修改
	bool config_changed = memcmp(&last_cfg_skin, &g_cfg->skin_changer, sizeof(last_cfg_skin)) != 0 ||
		memcmp(&last_cfg_knife, &g_cfg->knife_changer, sizeof(last_cfg_knife)) != 0;

	std::vector<uint16_t> current_weapon_indices;
	auto* weapon_service = local_pawn->m_weapon_services();
	if (valid_ptr(weapon_service)) {
		auto& my_weapons = weapon_service->my_weapons();
		for (unsigned int i = 0; i < my_weapons.m_size; i++) {
			auto* weapon = reinterpret_cast<c_econ_entity*>(g_interfaces->m_entity_system->get_base_entity(my_weapons.m_elements[i].get_entry_index()));
			if (!weapon) continue;

			auto* item = weapon->m_attribute_manager()->m_item();
			if (valid_ptr(item)) {
				current_weapon_indices.push_back(item->m_definition_index());
			}
		}
	}

	// 2. 对比是否有变化（数量变化或品种变化）
	bool weapon_changed = (current_weapon_indices != m_last_weapon_indices);

	// 3. 更新记录以便下次对比
	m_last_weapon_indices = current_weapon_indices;


	// 5. 触发更新周期
	// 复活、换队、手动标记、开启购买菜单、配置变动 都会触发
	if (team_changed || spawn_changed || should_update || weapon_changed || config_changed) {
		m_update_frames = 6; // 增加到10帧确保刷新成功
		should_update = false;

		// 更新备份，用于下次对比
		last_cfg_skin = g_cfg->skin_changer;
		last_cfg_knife = g_cfg->knife_changer;
	}

	// --- 关键修复：无论是否更新皮肤，每帧都同步玩家基础状态 ---
	auto sync_pawn_state = [&]() {
		m_last_spawn_time = current_spawn_time;
		m_last_team = current_team;
		};

	// 6. 判定是否处于有效更新周期
	if (m_update_frames <= 0) {
		sync_pawn_state(); // 即使不应用皮肤，也要记录当前状态
		return;
	}

	// 7. 执行应用逻辑
	/*auto* weapon_service = local_pawn->m_weapon_services();*/
	if (!valid_ptr(weapon_service)) {
		sync_pawn_state();
		return;
	}

	auto& my_weapons = weapon_service->my_weapons();
	auto* entity_system = g_interfaces->m_entity_system;
	std::vector<std::pair<c_econ_item_view*, uint32_t>> restore_list;
	bool did_update = false;

	for (unsigned int i = 0; i < my_weapons.m_size; i++) {
		auto* weapon = reinterpret_cast<c_econ_entity*>(
			entity_system->get_base_entity(my_weapons.m_elements[i].get_entry_index())
			);
		if (!weapon) continue;

		auto* item = weapon->m_attribute_manager()->m_item();
		if (!valid_ptr(item)) continue;

		uint32_t original_id = item->m_item_id_high();
		const uint16_t def_index = item->m_definition_index();
		const bool is_knife = (def_index == WEAPON_KNIFE || def_index == WEAPON_KNIFE_T || (def_index >= 500 && def_index <= 526));

		if (is_knife && knife_enabled)
			process_knife(weapon, item, local_pawn, true, did_update, local_steam_id);
		else if (!is_knife && skin_enabled)
			process_weapon(weapon, item, local_pawn, true, did_update, local_steam_id);

		restore_list.push_back({ item, original_id });
	}

	if (did_update)
		c_hud::regenerate_skins();
	
	// 8. remove!!清理与递减
	for (auto& pair : restore_list) {
		pair.first->m_item_id_high() = pair.second;
		econ_item_attribute_manager::remove(pair.first);
	}
	sync_pawn_state();
	m_update_frames--;
}
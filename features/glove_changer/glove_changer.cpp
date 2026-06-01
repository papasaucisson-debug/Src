#include "glove_changer.hpp"
#include "../shared/econ_item_attribute_manager.hpp"
#include "../../valve/interfaces/interfaces.hpp"

void c_glove_changer::run(int stage) {
	if (!g_cfg->glove_changer.m_enabled || stage != 7 || !g_ctx->m_local_pawn)
		return;

	auto* local_pawn = reinterpret_cast<c_cs_player_pawn*>(g_ctx->m_local_pawn);
	if (!valid_ptr(local_pawn) || local_pawn->m_health() <= 0)
		return;

	/*bool buy_menu_open = local_pawn->m_is_buy_menu_open();*/

	auto* identity = local_pawn->m_entity();
	if (!identity || !identity->is_valid())
		return;

	if (!identity->is_safe_to_modify())//!buy_menu_open &&
		return;

	if (g_cfg->glove_changer.m_glove == 0)
		return;

	auto* glove_item = local_pawn->m_econ_gloves();
	if (!glove_item)
		return;

	if (!g_item_schema->is_initialized()
		|| g_cfg->glove_changer.m_glove >= (int)g_item_schema->gloves.size())
		return;

	const uint16_t selected_glove =
		g_item_schema->gloves[g_cfg->glove_changer.m_glove].definition_index;
	if (selected_glove == 0)
		return;

	int paint_kit_id = g_item_schema->get_paint_kit_id_for_item(selected_glove, g_cfg->glove_changer.m_paint_kit);

	auto* paint_kit = glove_item->construct_paint_kit();

	const float current_spawn_time = local_pawn->m_last_spawn_time_index();
	const int   current_team = local_pawn->m_team_num();

	const bool config_changed = (m_last_glove != selected_glove)
		|| (m_last_paint_kit_id != paint_kit_id)
		|| (m_last_wear != g_cfg->glove_changer.m_wear)
		|| (m_last_seed != g_cfg->glove_changer.m_seed);
	const bool team_changed = (current_team != m_last_team) && m_last_team != 0;
	const bool spawn_changed = (current_spawn_time != m_last_spawn_time);
	const bool pawn_state_changed = team_changed || spawn_changed;
	const bool engine_reset = (glove_item->m_definition_index() != selected_glove)
		|| !glove_item->m_initialized()
		|| local_pawn->m_need_to_reapply_gloves();

	if (team_changed)
		m_clear_frames = 2;
	if (config_changed || pawn_state_changed || engine_reset || should_update)//|| buy_menu_open
		m_update_frames = 4;

	if (m_clear_frames > 0) {
		econ_item_attribute_manager::remove(glove_item);
		glove_item->m_definition_index() = 0;
		glove_item->m_initialized() = false;
		local_pawn->m_need_to_reapply_gloves() = true;
		m_clear_frames--;

		m_last_glove = 0;
		m_last_paint_kit_id = 0;
		m_last_team = current_team;
		should_update = false;
		return;
	}

	if (m_update_frames <= 0) {
		should_update = false;
		return;
	}

	glove_item->m_definition_index() = selected_glove;
	glove_item->m_entity_quality() = QUALITY_UNUSUAL;

	if (paint_kit && paint_kit_id > 0) {
		if (auto* desired_pk = g_interfaces->m_source2_client->get_econ_item_system()
			->get_econ_item_schema()->get_paint_kits().find_by_key(paint_kit_id))
			paint_kit->m_name = desired_pk->m_name;
	}

	econ_item_attribute_manager::remove(glove_item);
	if (paint_kit_id > 0)
		econ_item_attribute_manager::create(glove_item, paint_kit_id,
			g_cfg->glove_changer.m_wear, g_cfg->glove_changer.m_seed);

	glove_item->m_initialized() = true;
	local_pawn->set_body_group();
	local_pawn->m_need_to_reapply_gloves() = true;

	m_last_glove = selected_glove;
	m_last_paint_kit_id = paint_kit_id;
	m_last_wear = g_cfg->glove_changer.m_wear;
	m_last_seed = g_cfg->glove_changer.m_seed;
	m_last_spawn_time = current_spawn_time;
	m_last_team = current_team;
	m_update_frames--;
	should_update = false;
}

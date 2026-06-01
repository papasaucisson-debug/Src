#pragma once

#include "../../main.hpp"
#include "../../valve/classes/c_cs_player_pawn.hpp"
#include "../shared/item_schema.hpp"
#include <unordered_map>

class c_skin_changer {
public:
	void run(int stage);
	void initialize();

	bool should_update = false;
	bool is_initialized() const { return m_initialized; }

private:
	bool m_initialized = false;
	uint16_t m_last_knife = 0;
	int m_last_knife_paint_kit_id = 0;
	float m_last_knife_wear = 0.0001f;
	int m_last_knife_seed = 0;

	int m_update_frames = 0;
	int m_clear_frames = 0;
	// 用于检测复活和换队的状态缓存

	bool m_last_skin_enabled = false;
	bool m_last_knife_enabled = false;
	float m_last_spawn_time = 0.0f;
	int m_last_team = 0;
	std::vector<std::pair<c_econ_item_view*, uint32_t>> m_restore_list;
	std::vector<uint16_t> m_last_weapon_indices;
	std::vector<uint16_t> m_last_weapon_def_indices; // 存储武器定义索引以检测武器栏变化
	std::chrono::steady_clock::time_point m_change_detected_time; // 记录检测到变化的时间
	bool m_change_pending_update = false; // 标记是否有变化待触发 should_update

	// 新增的检测更新函数声明
	void check_for_updates(c_cs_player_pawn* local_pawn);

	int m_last_health = 0;
	int m_restore_delay_frames = 0;
	c_base_entity* get_hud_weapon(c_base_entity* weapon, c_cs_player_pawn* local_pawn);
	void apply_skin(c_econ_entity* weapon, c_econ_item_view* item, int paint_kit_id, float wear, int seed, const char* custom_name, c_cs_player_pawn* local_pawn, uint16_t def_index = 0);
	void process_weapon(c_econ_entity* weapon, c_econ_item_view* item, c_cs_player_pawn* local_pawn, bool force_update, bool& did_update, uint64_t local_steam_id);
	void process_knife(c_econ_entity* weapon, c_econ_item_view* item, c_cs_player_pawn* local_pawn, bool force_update, bool& did_update, uint64_t local_steam_id);
	void force_refresh_hud(c_econ_item_view* item);
};

inline const auto g_skin_changer = std::make_unique<c_skin_changer>();
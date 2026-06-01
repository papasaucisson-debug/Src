#pragma once

#include "../../main.hpp"
#include "../../valve/classes/c_cs_player_pawn.hpp"
#include "../shared/item_schema.hpp"

class c_glove_changer {
public:
	void run(int stage);
	bool should_update = false;

private:
	uint16_t m_last_glove = 0;
	int m_last_paint_kit_id = 0;
	float m_last_spawn_time = -1.f;
	float m_last_wear = 0.0001f;
	int m_last_seed = 0;
	int m_last_team = 0;

	int m_update_frames = 0;

	int m_clear_frames = 0;
};

inline const auto g_glove_changer = std::make_unique<c_glove_changer>();

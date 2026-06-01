#include "../main.hpp"
#include "menu.hpp"
#include "../features/shared/item_schema.hpp"
#include "../features/skin_changer/skin_changer.hpp"
#include "../features/glove_changer/glove_changer.hpp"
#include "../sdk/config_system/config_system.hpp"
#include "../valve/classes/c_cs_player_pawn.hpp"
#include "../valve/interfaces/interfaces.hpp"
#include <algorithm>
#include <vector>
#include <string>

void c_menu::rebuild_fonts(float scale) {
	auto& io = ImGui::GetIO();

	io.Fonts->Clear();

	ImFontConfig config;
	config.SizePixels = 13.0f * scale;
	config.OversampleH = 2;
	config.OversampleV = 1;
	io.Fonts->AddFontDefault(&config);

	io.Fonts->Build();

	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();

	m_dpi_scale = scale;
	m_style_initialized = false;
}

void c_menu::setup_style() {
	if (m_style_initialized) return;

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;
	float scale = m_dpi_scale;

	colors[ImGuiCol_WindowBg] = ImVec4(0.98f, 0.98f, 1.00f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.96f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.85f, 0.85f, 0.90f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.88f, 0.85f, 0.95f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.80f, 0.92f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.95f, 0.95f, 0.98f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.98f, 0.98f, 1.00f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.90f, 0.88f, 0.95f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.85f, 0.80f, 0.95f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.92f, 0.90f, 0.98f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.80f, 0.95f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.92f, 0.92f, 0.95f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.60f, 0.40f, 0.90f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);
	colors[ImGuiCol_Text] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);

	style.WindowRounding = 8.0f * scale;
	style.ChildRounding = 6.0f * scale;
	style.FrameRounding = 4.0f * scale;
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.40f, 0.90f, 1.00f);

	m_style_initialized = true;
}

static void draw_skins_tab() {
	ImGui::Text("Knife Changer");
	ImGui::Separator();
	ImGui::Checkbox("Enabled##knife", &g_cfg->knife_changer.m_enabled);

	if (g_cfg->knife_changer.m_enabled) {
		static int last_knife = 0;

		if (g_item_schema->is_initialized() && !g_item_schema->knife_names_cstr.empty()) {
			ImGui::Combo("Knife Model", &g_cfg->knife_changer.m_knife,
				g_item_schema->knife_names_cstr.data(),
				(int)g_item_schema->knife_names_cstr.size());
		} else {
			ImGui::TextDisabled("Loading knives...");
		}

		uint16_t selected_knife = 0;
		if (g_item_schema->is_initialized() &&
			g_cfg->knife_changer.m_knife < (int)g_item_schema->knives.size()) {
			selected_knife = g_item_schema->knives[g_cfg->knife_changer.m_knife].definition_index;
		}

		if (last_knife != g_cfg->knife_changer.m_knife) {
			g_cfg->knife_changer.m_paint_kit = 0;
			last_knife = g_cfg->knife_changer.m_knife;
		}

		if (g_item_schema->is_initialized()) {
			auto& knife_skins = g_item_schema->get_paint_kit_names_for_item(selected_knife);
			if (!knife_skins.empty()) {
				ImGui::Combo("Knife Skin", &g_cfg->knife_changer.m_paint_kit,
					knife_skins.data(), (int)knife_skins.size());
			}
		}

		static float temp_wear = g_cfg->knife_changer.m_wear;
		float item_width = ImGui::CalcItemWidth();
		float half_width = (item_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		ImGui::SetNextItemWidth(half_width);
		if (ImGui::SliderFloat("##Wear", &temp_wear, 0.0f, 1.0f, "%.4f")) {
			if (!ImGui::IsMouseDown(0))
				g_cfg->knife_changer.m_wear = temp_wear;
		}
		if (!ImGui::IsItemActive() && temp_wear != g_cfg->knife_changer.m_wear)
			g_cfg->knife_changer.m_wear = temp_wear;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(half_width);
		ImGui::InputInt("Wear / Seed", &g_cfg->knife_changer.m_seed, 0, 0);
		ImGui::InputText("Custom Name", g_cfg->knife_changer.m_custom_name,
			sizeof(g_cfg->knife_changer.m_custom_name));
	}

	ImGui::Spacing();
	ImGui::Text("Glove Changer");
	ImGui::Separator();
	ImGui::Checkbox("Enabled##glove", &g_cfg->glove_changer.m_enabled);

	if (g_cfg->glove_changer.m_enabled) {
		static int last_glove = 0;

		if (g_item_schema->is_initialized() && !g_item_schema->glove_names_cstr.empty()) {
			ImGui::Combo("Glove Model", &g_cfg->glove_changer.m_glove,
				g_item_schema->glove_names_cstr.data(),
				(int)g_item_schema->glove_names_cstr.size());
		} else {
			ImGui::TextDisabled("Loading gloves...");
		}

		uint16_t selected_glove = 0;
		if (g_item_schema->is_initialized() &&
			g_cfg->glove_changer.m_glove < (int)g_item_schema->gloves.size()) {
			selected_glove = g_item_schema->gloves[g_cfg->glove_changer.m_glove].definition_index;
		}

		if (last_glove != g_cfg->glove_changer.m_glove) {
			auto& glove_skins = g_item_schema->get_paint_kit_names_for_item(selected_glove);
			g_cfg->glove_changer.m_paint_kit = (glove_skins.size() > 1) ? 1 : 0;
			last_glove = g_cfg->glove_changer.m_glove;
		}

		if (g_item_schema->is_initialized()) {
			auto& glove_skins = g_item_schema->get_paint_kit_names_for_item(selected_glove);
			if (!glove_skins.empty()) {
				ImGui::Combo("Glove Skin", &g_cfg->glove_changer.m_paint_kit,
					glove_skins.data(), (int)glove_skins.size());
			}
		}

		static float temp_glove_wear = g_cfg->glove_changer.m_wear;
		float glove_item_width = ImGui::CalcItemWidth();
		float glove_half_width = (glove_item_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
		ImGui::SetNextItemWidth(glove_half_width);
		if (ImGui::SliderFloat("##GloveWear", &temp_glove_wear, 0.0f, 1.0f, "%.4f")) {
			if (!ImGui::IsMouseDown(0))
				g_cfg->glove_changer.m_wear = temp_glove_wear;
		}
		if (!ImGui::IsItemActive() && temp_glove_wear != g_cfg->glove_changer.m_wear)
			g_cfg->glove_changer.m_wear = temp_glove_wear;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(glove_half_width);
		ImGui::InputInt("Wear / Seed##glove", &g_cfg->glove_changer.m_seed, 0, 0);
	}

	ImGui::Spacing();
	ImGui::Text("Skin Changer");
	ImGui::Separator();
	ImGui::Checkbox("Enabled##skin", &g_cfg->skin_changer.m_enabled);

	if (g_cfg->skin_changer.m_enabled && g_item_schema->is_initialized()) {
		if (!g_item_schema->weapon_names_cstr.empty()) {
			ImGui::Combo("Weapon", &g_cfg->skin_changer.m_selected_weapon,
				g_item_schema->weapon_names_cstr.data(),
				(int)g_item_schema->weapon_names_cstr.size());
		}

		uint16_t selected_weapon_def = 0;
		if (g_cfg->skin_changer.m_selected_weapon < (int)g_item_schema->weapons.size()) {
			selected_weapon_def = g_item_schema->weapons[g_cfg->skin_changer.m_selected_weapon].definition_index;
		}

		if (selected_weapon_def > 0) {
			int config_index = c_config::skin_changer_t::get_config_index(selected_weapon_def);
			auto& weapon_skin = g_cfg->skin_changer.weapon_skins[config_index];

			auto& weapon_skins = g_item_schema->get_paint_kit_names_for_item(selected_weapon_def);
			if (!weapon_skins.empty()) {
				ImGui::Combo("Skin##weapon_skin", &weapon_skin.paint_kit,
					weapon_skins.data(), (int)weapon_skins.size());
			}

			float weapon_item_width = ImGui::CalcItemWidth();
			float weapon_half_width = (weapon_item_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
			ImGui::SetNextItemWidth(weapon_half_width);
			ImGui::SliderFloat("##WeaponWear", &weapon_skin.wear, 0.0f, 1.0f, "%.4f");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(weapon_half_width);
			ImGui::InputInt("Wear / Seed##weapon", &weapon_skin.seed, 0, 0);
			ImGui::InputText("Name##weapon_name", weapon_skin.custom_name,
				sizeof(weapon_skin.custom_name));

			if (ImGui::Button("Apply##skin_apply")) {
				g_skin_changer->should_update = true;
			}
		}
	}
}

static void draw_config_tab(float scale) {
	ImGui::Text("Config System");
	ImGui::Separator();

	g_config_system->refresh();
	auto& configs = g_config_system->get_config_files();

	ImGui::Text("Saved Configs:");
	ImGui::BeginChild("config_list", ImVec2(0, 150.0f * scale), true);
	{
		for (const auto& cfg : configs) {
			bool is_selected = (g_config_system->m_selected_config == cfg);
			if (ImGui::Selectable(cfg.c_str(), is_selected))
				g_config_system->m_selected_config = cfg;
		}
	}
	ImGui::EndChild();

	static char config_name[64] = "";
	ImGui::InputText("Config Name", config_name, sizeof(config_name));

	ImGui::Spacing();

	if (ImGui::Button("Create", ImVec2(80.0f * scale, 0))) {
		if (strlen(config_name) > 0) {
			g_config_system->save(config_name);
			g_config_system->m_selected_config = config_name;
			config_name[0] = '\0';
		}
	}
	ImGui::SameLine();

	if (ImGui::Button("Save", ImVec2(80.0f * scale, 0))) {
		if (!g_config_system->m_selected_config.empty())
			g_config_system->save(g_config_system->m_selected_config);
	}
	ImGui::SameLine();

	if (ImGui::Button("Load", ImVec2(80.0f * scale, 0))) {
		if (!g_config_system->m_selected_config.empty())
			g_config_system->load(g_config_system->m_selected_config);
	}

	if (ImGui::Button("Delete", ImVec2(80.0f * scale, 0))) {
		if (!g_config_system->m_selected_config.empty()) {
			g_config_system->remove(g_config_system->m_selected_config);
			g_config_system->m_selected_config.clear();
		}
	}
	ImGui::SameLine();

	if (ImGui::Button("Reset", ImVec2(80.0f * scale, 0)))
		g_config_system->reset();
	ImGui::SameLine();

	if (ImGui::Button("Refresh", ImVec2(80.0f * scale, 0)))
		g_config_system->refresh();
}

void c_menu::draw() {
	if (!m_opened)
		return;

	setup_style();

	float scale = m_dpi_scale;
	ImVec2 window_size(520.0f * scale, 480.0f * scale);

	ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);
	ImGui::Begin("Nerv", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	ImGui::BeginChild("tabs", ImVec2(150.0f * scale, 0), true);
	{
		static constexpr const char* tabs[]{ "Skins", "Config" };

		for (std::size_t i = 0; i < IM_ARRAYSIZE(tabs); ++i) {
			if (ImGui::Selectable(tabs[i], m_selected_tab == static_cast<int>(i)))
				m_selected_tab = static_cast<int>(i);
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();

	if (m_selected_tab == 0) {
		ImGui::BeginChild("skins_content", ImVec2(0, 0), true);
		draw_skins_tab();
		ImGui::EndChild();
	}

	if (m_selected_tab == 1) {
		ImGui::BeginChild("config_content", ImVec2(0, 0), true);
		draw_config_tab(scale);
		ImGui::EndChild();
	}

	ImGui::End();
}

void c_menu::on_create_move() {
	if (!m_opened)
		return;
}

void c_menu::add_toast(const std::string& message, float duration) {
	(void)message;
	(void)duration;
}

void c_menu::draw_toasts() {
	m_toasts.erase(
		std::remove_if(m_toasts.begin(), m_toasts.end(),
			[](const toast_t& t) { return t.is_expired(); }),
		m_toasts.end()
	);

	if (m_toasts.empty())
		return;

	auto* draw_list = ImGui::GetForegroundDrawList();
	ImVec2 screen_size = ImGui::GetIO().DisplaySize;

	float padding = 10.0f;
	float toast_height = 30.0f;
	float toast_width = 300.0f;
	float y_offset = padding;

	for (const auto& toast : m_toasts) {
		float alpha = toast.get_alpha();

		ImVec2 pos(screen_size.x - toast_width - padding, y_offset);
		ImVec2 size(toast_width, toast_height);

		draw_list->AddRectFilled(
			pos,
			ImVec2(pos.x + size.x, pos.y + size.y),
			IM_COL32(30, 30, 35, (int)(220 * alpha)),
			5.0f
		);

		draw_list->AddRect(
			pos,
			ImVec2(pos.x + size.x, pos.y + size.y),
			IM_COL32(100, 80, 160, (int)(255 * alpha)),
			5.0f
		);

		ImVec2 text_size = ImGui::CalcTextSize(toast.message.c_str());
		ImVec2 text_pos(
			pos.x + (size.x - text_size.x) * 0.5f,
			pos.y + (size.y - text_size.y) * 0.5f
		);
		draw_list->AddText(text_pos, IM_COL32(255, 255, 255, (int)(255 * alpha)), toast.message.c_str());

		y_offset += toast_height + 5.0f;
	}
}

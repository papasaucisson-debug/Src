#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono>

struct toast_t {
	std::string message;
	std::chrono::steady_clock::time_point start_time;
	float duration;

	bool is_expired() const {
		auto now = std::chrono::steady_clock::now();
		float elapsed = std::chrono::duration<float>(now - start_time).count();
		return elapsed >= duration;
	}

	float get_alpha() const {
		auto now = std::chrono::steady_clock::now();
		float elapsed = std::chrono::duration<float>(now - start_time).count();

		if (elapsed < 0.2f)
			return elapsed / 0.2f;

		float remaining = duration - elapsed;
		if (remaining < 0.5f)
			return remaining / 0.5f;

		return 1.0f;
	}
};

class c_menu {
	int m_selected_tab = 0;
	bool m_style_initialized = false;
	float m_dpi_scale = 1.0f;

	std::vector<toast_t> m_toasts;

public:
	bool m_opened = true;

	void draw();
	void on_create_move();
	void rebuild_fonts(float scale);
	void draw_toasts();

	void add_toast(const std::string& message, float duration = 3.0f);

	bool is_open() const { return m_opened; }
	float get_dpi_scale() const { return m_dpi_scale; }
	void set_dpi_scale(float scale) { m_dpi_scale = scale; }

private:
	void setup_style();
};

inline const auto g_menu = std::make_unique<c_menu>();

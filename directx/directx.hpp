#pragma once

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <memory>

class c_directx {
	IDXGISwapChain* m_swap_chain = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_device_context = nullptr;
	ID3D11RenderTargetView* m_render_target = nullptr;
	IDXGIFactory* m_factory = nullptr;

	HWND m_window = nullptr;
	WNDPROC m_wnd_proc_original = nullptr;
	bool m_started = false;
	bool m_imgui_initialized = false;
	bool m_initial_cursor_synced = false;
	float m_last_dpi_scale = 0.0f;
	float m_mouse_scale_x = 1.0f;
	float m_mouse_scale_y = 1.0f;

public:
	void* m_present_address = nullptr;
	void* m_resize_buffers_address = nullptr;
	void* m_create_swap_chain_address = nullptr;

	void initialize();
	void uninitialize();
	void try_resolve_late_hooks(IDXGISwapChain* swap_chain);

	void create_render_target();
	void destroy_render_target();
	void update_dpi_scale();

	void start_frame(IDXGISwapChain* swap_chain);
	void new_frame();
	void end_frame();

	WNDPROC get_wnd_proc_original() const { return m_wnd_proc_original; }
	ID3D11DeviceContext* get_device_context() const { return m_device_context; }
	ID3D11RenderTargetView* get_render_target() const { return m_render_target; }
	ID3D11Device* get_device() const { return m_device; }
	HWND get_window() const { return m_window; }
	float get_mouse_scale_x() const { return m_mouse_scale_x; }
	float get_mouse_scale_y() const { return m_mouse_scale_y; }
};

inline const auto g_directx = std::make_unique<c_directx>();

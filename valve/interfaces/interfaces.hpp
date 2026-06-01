#pragma once
#include "../modules/modules.hpp"
#include "vtables/i_csgo_input.hpp"
#include "vtables/i_entity_system.hpp"
#include "vtables/i_schema_system.hpp"
#include "vtables/i_econ_item_system.hpp"
#include "vtables/i_file_system.hpp"

template <typename type_t = void*>
inline type_t* get_interface(c_dll* dll, const char* name)
{
	const HINSTANCE module_handle = GetModuleHandle(dll->get_name());
	if (!module_handle)
		return nullptr;

	using create_interface_t = type_t * (*)(const char*, int*);
	const create_interface_t create_interface = reinterpret_cast<create_interface_t>(GetProcAddress(module_handle, "CreateInterface"));
	if (!create_interface)
		return nullptr;

	return create_interface(name, nullptr);
};

class c_interfaces
{
public:
	i_csgo_input*     m_csgo_input;
	i_entity_system*  m_entity_system;
	i_schema_system*  m_schema_system;
	void*             m_input_system;
	c_source2_client* m_source2_client;
	i_file_system*    m_file_system;

	void initialize();
};

inline const auto g_interfaces = std::make_unique<c_interfaces>();

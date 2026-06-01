#include "../../main.hpp"

void* get_entity_by_index(int index) {
	if (!g_interfaces->m_entity_system)
		return nullptr;
	return g_interfaces->m_entity_system->get_base_entity(index);
}

#define CHECK(name, arg) \
    if (arg == nullptr) { \
        LOG_ERROR(xorstr_("[-] Failed to get: %s"), name); \
        MessageBoxA(NULL, name, "Failed to initialize", MB_OK | MB_ICONERROR); \
        return; \
    }

void c_interfaces::initialize()
{
	const char* client_dll = g_modules->m_modules.client_dll.get_name();

	using get_input_t = i_csgo_input * (__fastcall*)();
	get_input_t get_input = reinterpret_cast<get_input_t>(g_opcodes->scan_absolute(client_dll, xorstr_("E8 ? ? ? ? 48 8B 56 ? 48 8B C8 E8 ? ? ? ? 4C 89 7E"), 0x1));
	CHECK(xorstr_("Input"), get_input)
	m_csgo_input = get_input();

	m_entity_system = *reinterpret_cast<i_entity_system**>(g_opcodes->scan_absolute(client_dll, xorstr_("48 8B 0D ? ? ? ? 48 89 7C 24 ? 8B FA C1 EB"), 0x3));
	CHECK(xorstr_("Entity System"), m_entity_system);

	m_schema_system = get_interface<i_schema_system>(&g_modules->m_modules.schemasystem_dll, xorstr_("SchemaSystem_001"));
	CHECK(xorstr_("Schema System"), m_schema_system);

	m_input_system = get_interface(&g_modules->m_modules.input_system, xorstr_("InputSystemVersion001"));
	CHECK(xorstr_("Input System"), m_input_system);

	m_source2_client = get_interface<c_source2_client>(&g_modules->m_modules.client_dll, xorstr_("Source2Client002"));
	CHECK(xorstr_("Source2Client"), m_source2_client);

	m_file_system = get_interface<i_file_system>(&g_modules->m_modules.filesystem_stdio, xorstr_("VFileSystem017"));
	CHECK(xorstr_("FileSystem"), m_file_system);
}

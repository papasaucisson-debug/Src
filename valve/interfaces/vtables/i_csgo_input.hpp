#pragma once
#include "../../../sdk/typedefs/vec_t.hpp"
#include "../../../sdk/vfunc/vfunc.hpp"
#include "../../../utils/utils.hpp"

#include "usercmd.pb.h"
#include "cs_usercmd.pb.h"

#define	FL_ONGROUND				(1 << 0)
#define FL_DUCKING				(1 << 1)

enum e_button : std::uint32_t
{
    IN_ATTACK = (1 << 0),
    IN_JUMP = (1 << 1),
    IN_DUCK = (1 << 2),
    IN_FORWARD = (1 << 3),
    IN_BACK = (1 << 4),
    IN_USE = (1 << 5),
    IN_CANCEL = (1 << 6),
    IN_LEFT = (1 << 7),
    IN_RIGHT = (1 << 8),
    IN_MOVELEFT = (1 << 9),
    IN_MOVERIGHT = (1 << 10),
    IN_ATTACK2 = (1 << 11),
    IN_RUN = (1 << 12),
    IN_RELOAD = (1 << 13),
    IN_LEFT_ALT = (1 << 14),
    IN_RIGHT_ALT = (1 << 15),
    IN_SCORE = (1 << 16),
    IN_SPEED = (1 << 17),
    IN_WALK = (1 << 18),
    IN_ZOOM = (1 << 19),
    IN_FIRST_WEAPON = (1 << 20),
    IN_SECOND_WEAPON = (1 << 21),
    IN_BULLRUSH = (1 << 22),
    IN_FIRST_GRENADE = (1 << 23),
    IN_SECOND_GRENADE = (1 << 24),
    IN_MIDDLE_ATTACK = (1 << 25),
    IN_USE_OR_RELOAD = (1 << 26)
};

class c_in_button_state
{
public:
    void* __vfptr;
    uint64_t m_button_state;
    uint64_t m_button_state2;
    uint64_t m_button_state3;

    enum e_button_state : int8_t {
        IN_BUTTON_UP = 0,
        IN_BUTTON_DOWN = 1,
        IN_BUTTON_DOWN_UP = 2,
        IN_BUTTON_UP_DOWN = 3,
        IN_BUTTON_UP_DOWN_UP = 4,
        IN_BUTTON_DOWN_UP_DOWN = 5,
        IN_BUTTON_DOWN_UP_DOWN_UP = 6,
        IN_BUTTON_UP_DOWN_UP_DOWN = 7
    };

    void set_button_state(uint64_t button, e_button_state state) {
        switch (state) {
        case IN_BUTTON_UP:
            m_button_state &= ~button;
            m_button_state2 &= ~button;
            m_button_state3 &= ~button;
            break;
        case IN_BUTTON_DOWN:
            m_button_state |= button;
            m_button_state2 &= ~button;
            m_button_state3 &= ~button;
            break;
        case IN_BUTTON_DOWN_UP:
            m_button_state &= ~button;
            m_button_state2 |= button;
            m_button_state3 &= ~button;
            break;
        case IN_BUTTON_UP_DOWN:
            m_button_state |= button;
            m_button_state2 |= button;
            m_button_state3 &= ~button;
            break;
        case IN_BUTTON_UP_DOWN_UP:
            m_button_state &= ~button;
            m_button_state2 &= ~button;
            m_button_state3 |= button;
            break;
        case IN_BUTTON_DOWN_UP_DOWN:
            m_button_state |= button;
            m_button_state2 &= ~button;
            m_button_state3 |= button;
            break;
        case IN_BUTTON_DOWN_UP_DOWN_UP:
            m_button_state &= ~button;
            m_button_state2 |= button;
            m_button_state3 |= button;
            break;
        case IN_BUTTON_UP_DOWN_UP_DOWN:
            m_button_state |= button;
            m_button_state2 |= button;
            m_button_state3 |= button;
            break;
        }
    }
};

class c_user_cmd
{
public:
    void* __vfptr;
    int64_t m_command_number;
    CSGOUserCmdPB pb;
    c_in_button_state m_button_state;
    char pad_0078[8];
    double m_last_server_time;
    bool m_has_been_predicted;
    int m_flag;
    int m_cmd_type;

    CBaseUserCmdPB* get_base_cmd() {
        return pb.mutable_base();
    }
};

class i_csgo_input
{
public:

    char pad_0000[0x228];
    bool block_shot;
    bool in_thirdperson;
    char pad_0252[0x6];
    vec3_t third_person_angles;
    char pad_0264[0x04];
    uint64_t button_pressed;
    uint64_t mouse_button_pressed;
    uint64_t button_un_pressed;
    uint64_t keyboard_copy;
    float forward_move;
    float left_move;
    float up_move;
    int mouse_delta_x;
    int mouse_delta_y;
    int32_t subtick_count;

    void* subticks[0xC];
    vec3_t view_angles;
    int32_t target_entity_index;
    char pad_03E0[0x230];
    int32_t attack_history_1;
    int32_t attack_history_2;
    int32_t attack_history_3;
    char pad_061C[0x4];
    int32_t message_size;
    char pad_0624[0x4];
    void* message;

    vec3_t get_view_angles() {
        using get_view_angles_t = void*(__fastcall*)(i_csgo_input*, int);
        static get_view_angles_t fn = reinterpret_cast<get_view_angles_t>(
            g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "4C 8B C1 85 D2 74 ? 48 8D 05")
        );
        return *reinterpret_cast<vec3_t*>(fn(this, 0));
    }

    void set_view_angles(vec3_t& view_angles) {
        using set_view_angles_t = void(__fastcall*)(i_csgo_input*, int, vec3_t&);
        static set_view_angles_t fn = reinterpret_cast<set_view_angles_t>(
            g_opcodes->scan(g_modules->m_modules.client_dll.get_name(), "85 D2 75 ? 48 63 81")
        );
        fn(this, 0, view_angles);
    }

    c_user_cmd* get_user_cmd(void* local_controller)
    {
        if (!local_controller)
            return nullptr;

        int player_idx = 0;

        using get_entity_index_t = void* (__fastcall*)(void*, int*);
        static get_entity_index_t get_entity_index = reinterpret_cast<get_entity_index_t>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 8B 8D ? ? ? ? 8D 51", 0x1));

        get_entity_index(local_controller, &player_idx);

        int entity_index = player_idx - 1;
        if (player_idx == -1)
            entity_index = -1;

        static auto sig_poo = reinterpret_cast<void**>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B CF 48 8B F0", 0x3));
        static auto construct_input_data = reinterpret_cast<void* (__fastcall*)(void*, int)>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 48 8B CF 48 8B F0", 0x1));

        if (!sig_poo || !*sig_poo || !construct_input_data) {
            return nullptr;
        }

        void* array_inputs = construct_input_data(sig_poo, entity_index);
        if (!array_inputs) return nullptr;

        int sequence_number = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(array_inputs) + 0x5910);

        static auto automake_user_cmd = reinterpret_cast<void*>(g_opcodes->scan_absolute(g_modules->m_modules.client_dll.get_name(), "E8 ? ? ? ? 48 89 44 24 ? 48 8D 4D", 0x1));
        static auto poo = reinterpret_cast<c_user_cmd * (__fastcall*)(void*, int)>(automake_user_cmd);

        return poo(local_controller, sequence_number);
    }
};

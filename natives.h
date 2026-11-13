// natives.h - غلاف لاستدعاء الـ natives + الهاشات
// ⚠ الهاشات من نسخة PC وبعضها ممكن يختلف حسب نسخة الـ PS4. تحقق منها.
// ⚠ شكل الـ context تحت حسب RAGE المعروف، عدّله إذا نسختك مختلفة.
#pragma once
#include <stdint.h>

struct NativeCtx {
    uint64_t* ret;
    uint32_t argc;
    uint64_t* args;
    uint32_t data_count;
    uint64_t data[48];
};
typedef void (*NativeHandler)(NativeCtx*);
typedef NativeHandler (*GetHandlerFn)(uint64_t hash);

extern GetHandlerFn g_getHandler;

template <typename T> static inline uint64_t to_u64(T v) {
    uint64_t r = 0;
    __builtin_memcpy(&r, &v, sizeof(T) < 8 ? sizeof(T) : 8);
    return r;
}

template <typename R = void, typename... A>
static inline R invoke(uint64_t hash, A... a) {
    if (!g_getHandler) return R();
    NativeHandler h = g_getHandler(hash);
    if (!h) return R();
    uint64_t args[sizeof...(A) + 1] = { to_u64(a)... };
    uint64_t ret[3] = {0, 0, 0};
    NativeCtx c{};
    c.ret = ret; c.args = args; c.argc = sizeof...(A);
    h(&c);
    if constexpr (!__is_same(R, void)) { R r; __builtin_memcpy(&r, ret, sizeof(R)); return r; }
}

namespace H {
    constexpr uint64_t PLAYER_PED_ID            = 0xD80958FC74E988A6;
    constexpr uint64_t SET_ENTITY_INVINCIBLE    = 0x3882114BDE571AD4;
    constexpr uint64_t SET_PLAYER_WANTED_LEVEL  = 0x39FF19C64EF7DA5B;
    constexpr uint64_t SET_PLAYER_WANTED_NOW    = 0xE0A7D1E497FFCD6F;
    constexpr uint64_t SET_PED_INFINITE_AMMO_CLIP = 0x183DADC6AA953186;
    constexpr uint64_t SET_ENTITY_COORDS        = 0x06843DA7060A026B;
    constexpr uint64_t GET_ENTITY_COORDS        = 0x3FEF770D40960D5A;
    constexpr uint64_t SET_CLOCK_TIME           = 0x47C3B5848C3E45D8;
    constexpr uint64_t SET_WEATHER_NOW_PERSIST  = 0xED712CA327900C8A;
    constexpr uint64_t DRAW_RECT                = 0x3A618A217E5154F0;
    constexpr uint64_t SET_TEXT_FONT            = 0x66E0276CC5F6B9DA;
    constexpr uint64_t SET_TEXT_SCALE           = 0x07C837F9A01C34C9;
    constexpr uint64_t SET_TEXT_COLOUR          = 0xBE6B23FFA53FB442;
    constexpr uint64_t BEGIN_TEXT_CMD           = 0x25FBB336DF1804CB;
    constexpr uint64_t ADD_TEXT_SUBSTRING       = 0x6C188BE134E074AA;
    constexpr uint64_t END_TEXT_CMD             = 0xCD015E5BB0D96A57;
}

static inline void draw_rect(float x, float y, float w, float h, int r, int g, int b, int a) {
    invoke(H::DRAW_RECT, x, y, w, h, r, g, b, a, 0);
}
static inline void draw_text(const char* s, float x, float y, float scale, int r, int g, int b, int a) {
    invoke(H::SET_TEXT_FONT, 0);
    invoke(H::SET_TEXT_SCALE, 0.0f, scale);
    invoke(H::SET_TEXT_COLOUR, r, g, b, a);
    invoke(H::BEGIN_TEXT_CMD, "STRING");
    invoke(H::ADD_TEXT_SUBSTRING, s);
    invoke(H::END_TEXT_CMD, x, y, 0);
}
static inline uint32_t player_ped() { return invoke<uint32_t>(H::PLAYER_PED_ID); }

namespace H2 {
    constexpr uint64_t NETWORK_IS_SESSION_STARTED = 0x9DE624D2FC4B603F;
    constexpr uint64_t SET_ENTITY_HEALTH       = 0x6B76DC1F3AE6E6A3;
    constexpr uint64_t SET_PED_ARMOUR          = 0xCEA04D83135264CC;
    constexpr uint64_t SET_PED_CAN_RAGDOLL     = 0xB128377056A54E2A;
    constexpr uint64_t SET_ENTITY_VISIBLE      = 0xEA1C610A04DB6BBB;
    constexpr uint64_t SET_RUN_SPRINT_MULT     = 0x6DB47AA77FD94E09;
    constexpr uint64_t SET_SUPER_JUMP          = 0x57FFF03E423A4C0B;
    constexpr uint64_t GIVE_WEAPON_TO_PED      = 0xBF0FD6E56C964FCB;
    constexpr uint64_t REMOVE_ALL_WEAPONS      = 0xF25DF915FA38C5F3;
    constexpr uint64_t SET_EXPLOSIVE_AMMO      = 0xA66C71C98D5F2CFB;
    constexpr uint64_t SET_FIRE_AMMO           = 0x11879CDD803D30F4;
    constexpr uint64_t SET_EXPLOSIVE_MELEE     = 0xFF1BED81BFDC0FE0;
    constexpr uint64_t REQUEST_MODEL           = 0x963D27A58DF860AC;
    constexpr uint64_t HAS_MODEL_LOADED        = 0x98A4EB5D89A0C952;
    constexpr uint64_t MODEL_NO_LONGER_NEEDED  = 0xE532F5D78798DAAB;
    constexpr uint64_t CREATE_VEHICLE          = 0xAF35D0D2583051B0;
    constexpr uint64_t SET_PED_INTO_VEHICLE    = 0xF75B0D629E1C063D;
    constexpr uint64_t GET_VEHICLE_PED_IS_IN   = 0x9A9112A0FE9A4713;
    constexpr uint64_t SET_VEHICLE_FIXED       = 0x115722B1B9C14C1C;
    constexpr uint64_t SET_VEH_DEFORM_FIXED    = 0x953DA1E1B12C0491;
    constexpr uint64_t SET_VEHICLE_DIRT        = 0x79D3B596FE44EE8B;
    constexpr uint64_t SET_VEH_ON_GROUND       = 0x49733E92263139D1;
    constexpr uint64_t SET_VEH_FWD_SPEED       = 0xAB54A438726D25D5;
    constexpr uint64_t SET_VEH_MOD_KIT         = 0x1F2AA07F00B3217A;
    constexpr uint64_t GET_NUM_VEH_MODS        = 0xE38E9162A2500646;
    constexpr uint64_t SET_VEH_MOD             = 0x6AF0636DDEDCB6DD;
    constexpr uint64_t TOGGLE_VEH_MOD          = 0x2A1F4F37F95BAD08;
    constexpr uint64_t SET_VEH_CUSTOM_PRIMARY  = 0x7141766F91D15BEA;
    constexpr uint64_t SET_TIME_SCALE          = 0x1D408577D440E81E;
    constexpr uint64_t CLEAR_AREA              = 0xA56F01F3765B93A0;
    constexpr uint64_t GET_FIRST_BLIP_INFO_ID  = 0x1BEDE233E6CD2A1F;
    constexpr uint64_t DOES_BLIP_EXIST         = 0xA6DB27D19ECBB7DA;
    constexpr uint64_t GET_BLIP_INFO_ID_COORD  = 0xFA7C7F0AADF25D09;
    constexpr uint64_t GET_GROUND_Z            = 0xC906A7DAB05C8D2B;
    constexpr uint64_t SET_ENTITY_COLLISION    = 0x1A9205C1B9EE827F;
    constexpr uint64_t FREEZE_ENTITY           = 0x428CA6DBD1094446;
    constexpr uint64_t SET_COORDS_NO_OFFSET    = 0x239A3351AC1DA385;
    constexpr uint64_t GET_GAMEPLAY_CAM_ROT    = 0x837765A25378F0BB;
    constexpr uint64_t GET_ENTITY_HEADING      = 0xE83D4F9BA2A38914;
    constexpr uint64_t GET_OFFSET_FROM_ENTITY  = 0x1899F328B0E12848;
}

// استدعاء native يرجّع Vector3
template <typename... A>
static inline bool invoke_v3(uint64_t hash, float* out, A... a) {
    if (!g_getHandler) return false;
    NativeHandler h = g_getHandler(hash);
    if (!h) return false;
    uint64_t args[sizeof...(A) + 1] = { to_u64(a)... };
    uint64_t ret[3] = {0, 0, 0};
    NativeCtx c{};
    c.ret = ret; c.args = args; c.argc = sizeof...(A);
    h(&c);
    for (int i = 0; i < 3; i++) { float f; __builtin_memcpy(&f, &ret[i], 4); out[i] = f; }
    return true;
}

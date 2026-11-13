// features.h - كل ميزات المنيو (سنقل بلاير فقط)
// الحماية: كل الميزات تتوقف تلقائياً إذا اللاعب دخل جلسة أونلاين.
#pragma once
#include <math.h>
#include <stdint.h>
#include "natives.h"

struct PadState { float lx = 0, ly = 0; uint32_t btn = 0; };
extern PadState g_pad;       // يتحدث من هوك الأزرار (قبل الحجب)
extern bool g_menuOpen;
extern void (*g_notify)(const char*);

constexpr uint32_t joaat(const char* s) {
    uint32_t h = 0;
    for (; *s; ++s) { char c = *s; if (c >= 'A' && c <= 'Z') c += 32; h += (uint8_t)c; h += h << 10; h ^= h >> 6; }
    h += h << 3; h ^= h >> 11; h += h << 15; return h;
}

namespace F {
    // ----- الحالات -----
    static bool god, neverWanted, infAmmo, noRagdoll, invisible, superJump, fastRun;
    static bool noclip, explosiveAmmo, fireAmmo, explosiveMelee;
    static bool vehGod, vehBoost;
    static float noclipSpeed = 1.0f, timeScale = 1.0f, hour = 12;
    static bool warned = false;

    static bool online() { return invoke<int>(H2::NETWORK_IS_SESSION_STARTED) != 0; }
    static uint32_t cur_veh() { return invoke<uint32_t>(H2::GET_VEHICLE_PED_IS_IN, player_ped(), 0); }
    static void say(const char* m) { if (g_notify) g_notify(m); }

    // ----- Self -----
    static void heal() { uint32_t p = player_ped(); invoke(H2::SET_ENTITY_HEALTH, p, 200); invoke(H2::SET_PED_ARMOUR, p, 100); }
    static void clear_wanted() { invoke(H::SET_PLAYER_WANTED_LEVEL, 0, 0, 0); invoke(H::SET_PLAYER_WANTED_NOW, 0, 0); }

    // ----- Weapons -----
    static const char* kWeapons[] = {
        "weapon_knife", "weapon_pistol", "weapon_combatpistol", "weapon_appistol", "weapon_pistol50",
        "weapon_microsmg", "weapon_smg", "weapon_assaultsmg", "weapon_assaultrifle", "weapon_carbinerifle",
        "weapon_advancedrifle", "weapon_specialcarbine", "weapon_bullpuprifle", "weapon_mg", "weapon_combatmg",
        "weapon_pumpshotgun", "weapon_sawnoffshotgun", "weapon_assaultshotgun", "weapon_sniperrifle",
        "weapon_heavysniper", "weapon_marksmanrifle", "weapon_grenadelauncher", "weapon_rpg", "weapon_minigun",
        "weapon_grenade", "weapon_stickybomb", "weapon_smokegrenade", "weapon_molotov", "weapon_fireextinguisher",
    };
    static void give_all_weapons() {
        uint32_t p = player_ped();
        for (auto w : kWeapons) invoke(H2::GIVE_WEAPON_TO_PED, p, joaat(w), 9999, 0, 1);
        say("All weapons given");
    }
    static void remove_weapons() { invoke(H2::REMOVE_ALL_WEAPONS, player_ped()); }

    // ----- Vehicles -----
    struct VehDef { const char* label; const char* model; };
    static const VehDef kVehicles[] = {
        {"Adder", "adder"}, {"Zentorno", "zentorno"}, {"T20", "t20"}, {"Bati 801", "bati"},
        {"Insurgent", "insurgent"}, {"Rhino Tank", "rhino"}, {"Buzzard", "buzzard"},
        {"Hydra", "hydra"}, {"Lazer", "lazer"}, {"Deluxo", "deluxo"}, {"Dump", "dump"},
    };
    static uint32_t pendingModel = 0;
    static int pendingFrames = 0;

    static void spawn(const char* model) { pendingModel = joaat(model); pendingFrames = 0; invoke(H2::REQUEST_MODEL, pendingModel); }

    static void spawn_tick() {
        if (!pendingModel) return;
        if (++pendingFrames > 600) { pendingModel = 0; say("Spawn timed out"); return; }
        if (!invoke<int>(H2::HAS_MODEL_LOADED, pendingModel)) { invoke(H2::REQUEST_MODEL, pendingModel); return; }
        uint32_t p = player_ped();
        float pos[3];
        invoke_v3(H2::GET_OFFSET_FROM_ENTITY, pos, p, 0.0f, 5.0f, 0.5f);
        float hd = invoke<float>(H2::GET_ENTITY_HEADING, p);
        uint32_t v = invoke<uint32_t>(H2::CREATE_VEHICLE, pendingModel, pos[0], pos[1], pos[2], hd, 0, 0, 0);
        if (v) { invoke(H2::SET_PED_INTO_VEHICLE, p, v, -1); invoke(H2::SET_VEH_ON_GROUND, v); }
        invoke(H2::MODEL_NO_LONGER_NEEDED, pendingModel);
        pendingModel = 0;
    }
    static void repair() { uint32_t v = cur_veh(); if (!v) return; invoke(H2::SET_VEHICLE_FIXED, v); invoke(H2::SET_VEH_DEFORM_FIXED, v); invoke(H2::SET_VEHICLE_DIRT, v, 0.0f); }
    static void flip() { uint32_t v = cur_veh(); if (v) invoke(H2::SET_VEH_ON_GROUND, v); }
    static void max_upgrade() {
        uint32_t v = cur_veh(); if (!v) return;
        invoke(H2::SET_VEH_MOD_KIT, v, 0);
        for (int m = 0; m < 50; m++) {
            if (m >= 17 && m <= 22) { invoke(H2::TOGGLE_VEH_MOD, v, m, 1); continue; }
            int n = invoke<int>(H2::GET_NUM_VEH_MODS, v, m);
            if (n > 0) invoke(H2::SET_VEH_MOD, v, m, n - 1, 0);
        }
        say("Vehicle maxed");
    }
    static void random_color() {
        uint32_t v = cur_veh(); if (!v) return;
        static uint32_t s = 12345; s = s * 1664525u + 1013904223u;
        invoke(H2::SET_VEH_CUSTOM_PRIMARY, v, (int)(s >> 24), (int)((s >> 16) & 255), (int)((s >> 8) & 255));
    }

    // ----- World -----
    static const char* kWeather[] = {"EXTRASUNNY", "CLEAR", "CLOUDS", "OVERCAST", "RAIN", "THUNDER", "FOGGY", "SNOW", "XMAS"};
    static void set_weather(const char* w) { invoke(H::SET_WEATHER_NOW_PERSIST, w); }
    static void clear_area() {
        float c[3]; invoke_v3(H::GET_ENTITY_COORDS, c, player_ped(), 1);
        invoke(H2::CLEAR_AREA, c[0], c[1], c[2], 100.0f, 0, 0, 0, 0);
    }

    // ----- Teleport -----
    static int tpFrames = 0; static float tpX, tpY;
    static void teleport(float x, float y, float z) {
        uint32_t e = cur_veh(); if (!e) e = player_ped();
        invoke(H::SET_ENTITY_COORDS, e, x, y, z, 0, 0, 0, 1);
    }
    static void teleport_waypoint() {
        uint32_t blip = invoke<uint32_t>(H2::GET_FIRST_BLIP_INFO_ID, 8);
        if (!invoke<int>(H2::DOES_BLIP_EXIST, blip)) { say("No waypoint set"); return; }
        float c[3]; invoke_v3(H2::GET_BLIP_INFO_ID_COORD, c, blip);
        tpX = c[0]; tpY = c[1]; tpFrames = 1;
    }
    static void tp_tick() {
        if (!tpFrames) return;
        uint32_t e = cur_veh(); if (!e) e = player_ped();
        if (tpFrames == 1) invoke(H2::FREEZE_ENTITY, e, 1);
        float gz = 0;
        invoke(H::SET_ENTITY_COORDS, e, tpX, tpY, 600.0f, 0, 0, 0, 1);
        bool found = invoke<int>(H2::GET_GROUND_Z, tpX, tpY, 1000.0f, &gz, 0) != 0;
        if (found || ++tpFrames > 300) {
            invoke(H::SET_ENTITY_COORDS, e, tpX, tpY, (found ? gz : 100.0f) + 1.0f, 0, 0, 0, 1);
            invoke(H2::FREEZE_ENTITY, e, 0);
            tpFrames = 0;
        }
    }

    // ----- NoClip -----
    static bool ncLast = false;
    static void noclip_tick() {
        uint32_t ped = player_ped();
        uint32_t e = cur_veh(); if (!e) e = ped;
        if (noclip != ncLast) {
            invoke(H2::SET_ENTITY_COLLISION, e, noclip ? 0 : 1, noclip ? 0 : 1);
            invoke(H2::FREEZE_ENTITY, e, noclip ? 1 : 0);
            ncLast = noclip;
        }
        if (!noclip || g_menuOpen) return;
        float p[3], r[3];
        invoke_v3(H::GET_ENTITY_COORDS, p, e, 1);
        invoke_v3(H2::GET_GAMEPLAY_CAM_ROT, r, 2);
        float yaw = r[2] * 0.0174533f;
        float fwd = -g_pad.ly, str = g_pad.lx;
        float dx = (-sinf(yaw) * fwd + cosf(yaw) * str) * noclipSpeed;
        float dy = ( cosf(yaw) * fwd + sinf(yaw) * str) * noclipSpeed;
        float dz = 0;
        if (g_pad.btn & 0x0200) dz += noclipSpeed;   // R2 فوق
        if (g_pad.btn & 0x0100) dz -= noclipSpeed;   // L2 تحت
        if (g_pad.btn & 0x0400) { dx *= 3; dy *= 3; dz *= 3; } // L1 سرعة
        invoke(H2::SET_COORDS_NO_OFFSET, e, p[0] + dx, p[1] + dy, p[2] + dz, 0, 0, 0);
    }

    // ----- الحلقة الرئيسية -----
    static bool invLast = false, ragLast = false, runLast = false;
    static void tick() {
        if (online()) {
            if (!warned) { say("zokii: online session - menu disabled"); warned = true; }
            return;
        }
        warned = false;
        uint32_t ped = player_ped();
        if (god)         invoke(H::SET_ENTITY_INVINCIBLE, ped, 1);
        if (neverWanted) invoke(H::SET_PLAYER_WANTED_LEVEL, 0, 0, 0);
        if (infAmmo)     invoke(H::SET_PED_INFINITE_AMMO_CLIP, ped, 1);
        if (superJump)   invoke(H2::SET_SUPER_JUMP, 0);
        if (explosiveAmmo)  invoke(H2::SET_EXPLOSIVE_AMMO, 0);
        if (fireAmmo)       invoke(H2::SET_FIRE_AMMO, 0);
        if (explosiveMelee) invoke(H2::SET_EXPLOSIVE_MELEE, 0);
        if (invisible != invLast) { invoke(H2::SET_ENTITY_VISIBLE, ped, invisible ? 0 : 1, 0); invLast = invisible; }
        if (noRagdoll != ragLast) { invoke(H2::SET_PED_CAN_RAGDOLL, ped, noRagdoll ? 0 : 1); ragLast = noRagdoll; }
        if (fastRun != runLast)   { invoke(H2::SET_RUN_SPRINT_MULT, 0, fastRun ? 1.49f : 1.0f); runLast = fastRun; }
        uint32_t v = cur_veh();
        if (v && vehGod) invoke(H::SET_ENTITY_INVINCIBLE, v, 1);
        if (v && vehBoost && (g_pad.btn & 0x0400) && !g_menuOpen) invoke(H2::SET_VEH_FWD_SPEED, v, 60.0f); // L1
        noclip_tick();
        spawn_tick();
        tp_tick();
    }
}

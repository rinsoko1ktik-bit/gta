// zokii mod menu - GoldHEN plugin (GTA V PS4 1.00 -> 1.52)
// فتح/قفل المنيو: R1 + مربع | سنقل بلاير فقط (يتعطل تلقائياً أونلاين)
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <Common.h>       // GoldHEN Plugins SDK
#include "offsets.h"
#include "menu.h"
#include "lua_bridge.h"
#include "features.h"

attr_module_hidden module_t g_hook_module = 0;
attr_public const char* g_pluginName = "zokii_mod_menu";
attr_public const char* g_pluginDesc = "zokii mod menu";
attr_public const char* g_pluginAuth = "Zokii";
attr_public uint32_t g_pluginVersion = 0x00000200;

GetHandlerFn g_getHandler = nullptr;
PadState g_pad;
void (*g_notify)(const char*) = nullptr;
bool g_menuOpen = false;
static Menu g_menu;
static const GameOffsets* g_off = nullptr;
static uintptr_t g_base = 0;

static void notify(const char* m) { Notify(TEX_ICON_SYSTEM, "%s", m); }

typedef int (*ReadStateFn)(int handle, OrbisPadData* data);
static ReadStateFn orig_scePadReadState = nullptr;
static int hk_scePadReadState(int handle, OrbisPadData* data) {
    int r = orig_scePadReadState(handle, data);
    if (r >= 0 && data) {
        g_pad.btn = data->buttons;
        g_pad.lx = ((int)data->leftStick.x - 128) / 128.0f;
        g_pad.ly = ((int)data->leftStick.y - 128) / 128.0f;
        if (fabsf(g_pad.lx) < 0.15f) g_pad.lx = 0;
        if (fabsf(g_pad.ly) < 0.15f) g_pad.ly = 0;
        data->buttons = g_menu.input(data->buttons);
        g_menuOpen = g_menu.open;
    }
    return r;
}

extern "C" void on_game_tick() {
    if (F::online()) { g_menu.open = false; g_menuOpen = false; }
    F::tick();
    if (!F::online()) scripts_tick();
    g_menu.draw();
}

static void build_menu() {
    using M = Menu;
    g_menu.tabs.clear();

    Tab self{"Self", {
        M::Toggle("God Mode", &F::god),
        M::Toggle("Never Wanted", &F::neverWanted),
        M::Toggle("Infinite Ammo", &F::infAmmo),
        M::Toggle("No Ragdoll", &F::noRagdoll),
        M::Toggle("Invisible", &F::invisible),
        M::Toggle("Super Jump", &F::superJump),
        M::Toggle("Fast Run", &F::fastRun),
        M::Toggle("NoClip (L1 fast, R2/L2 up/down)", &F::noclip),
        M::Slider("NoClip Speed", &F::noclipSpeed, 0.2f, 5.0f, 0.2f),
        M::Action("Heal + Armor", [] { F::heal(); }),
        M::Action("Clear Wanted", [] { F::clear_wanted(); }),
    }};

    Tab wpn{"Weapons", {
        M::Action("Give All Weapons", [] { F::give_all_weapons(); }),
        M::Action("Remove All Weapons", [] { F::remove_weapons(); }),
        M::Toggle("Explosive Ammo", &F::explosiveAmmo),
        M::Toggle("Fire Ammo", &F::fireAmmo),
        M::Toggle("Explosive Melee", &F::explosiveMelee),
        M::Toggle("Infinite Ammo", &F::infAmmo),
    }};

    Tab veh{"Vehicle", {
        M::Toggle("Vehicle God Mode", &F::vehGod),
        M::Toggle("Boost (hold L1)", &F::vehBoost),
        M::Action("Repair", [] { F::repair(); }),
        M::Action("Flip Upright", [] { F::flip(); }),
        M::Action("Max Upgrades", [] { F::max_upgrade(); }),
        M::Action("Random Color", [] { F::random_color(); }),
    }};

    Tab spawn{"Spawn", {}};
    for (auto& v : F::kVehicles) {
        const char* m = v.model;
        spawn.items.push_back(M::Action(v.label, [m] { F::spawn(m); }));
    }

    Tab world{"World", {
        M::Slider("Hour", &F::hour, 0, 23, 1),
        M::Action("Apply Time", [] { invoke(H::SET_CLOCK_TIME, (int)F::hour, 0, 0); }),
        M::Slider("Game Speed", &F::timeScale, 0.1f, 2.0f, 0.1f),
        M::Action("Apply Game Speed", [] { invoke(H2::SET_TIME_SCALE, F::timeScale); }),
        M::Action("Clear Area", [] { F::clear_area(); }),
    }};
    for (auto w : F::kWeather) {
        static char labels[16][32]; static int n = 0;
        snprintf(labels[n], 32, "Weather: %s", w);
        world.items.push_back(M::Action(labels[n++], [w] { F::set_weather(w); }));
    }

    Tab tp{"Teleport", {
        M::Action("To Waypoint", [] { F::teleport_waypoint(); }),
        M::Action("Los Santos Airport", [] { F::teleport(-1336.0f, -3044.0f, 14.0f); }),
        M::Action("Mount Chiliad", [] { F::teleport(450.0f, 5566.0f, 797.0f); }),
        M::Action("Maze Bank Roof", [] { F::teleport(-75.0f, -819.0f, 326.0f); }),
        M::Action("Vinewood Sign", [] { F::teleport(711.0f, 1198.0f, 348.0f); }),
        M::Action("Michael's House", [] { F::teleport(-813.0f, 178.0f, 72.0f); }),
    }};

    Tab scr{"Scripts", {}};
    scripts_scan();
    scr.items.push_back(M::Action("Reload Scripts", [] {
        for (auto& s : g_scripts) script_set(s, false);
        scripts_scan();
        notify("Scripts reloaded - reopen menu");
    }));
    for (size_t i = 0; i < g_scripts.size(); i++)
        scr.items.push_back(M::Action(g_scripts[i].name.c_str(), [i] { script_set(g_scripts[i], !g_scripts[i].enabled); }));

    g_menu.tabs = { self, wpn, veh, spawn, world, tp, scr };
}

extern "C" int32_t attr_module_hidden module_start(size_t argc, const void* args) { return 0; }
extern "C" int32_t attr_module_hidden module_stop(size_t argc, const void* args) { return 0; }

extern "C" int32_t attr_public plugin_load(int32_t argc, const char* argv[]) {
    g_notify = notify;
    char ver[16] = {0};
    if (!read_app_ver(ver, sizeof ver)) { notify("zokii: cannot read game version"); return 0; }
    g_off = find_offsets(ver);
    if (!g_off) { Notify(TEX_ICON_SYSTEM, "zokii: version %s not supported (1.00-1.52)", ver); return 0; }
    if (!g_off->get_native_handler || !g_off->game_tick) {
        Notify(TEX_ICON_SYSTEM, "zokii: offsets for %s are empty - fill offsets.h", ver);
        return 0;
    }
    // g_base = عنوان eboot (من الـ SDK)
    g_getHandler = (GetHandlerFn)(g_base + g_off->get_native_handler);
    build_menu();
    // TODO: ركّب الهوكين بـ Detour الخاص بالـ SDK:
    //   scePadReadState -> hk_scePadReadState (احفظ الأصلية في orig_scePadReadState)
    //   g_base + g_off->game_tick -> on_game_tick
    Notify(TEX_ICON_SYSTEM, "zokii mod menu loaded (%s) - R1 + SQUARE", ver);
    return 0;
}
extern "C" int32_t attr_public plugin_unload(int32_t argc, const char* argv[]) { return 0; }

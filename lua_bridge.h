// lua_bridge.h - نظام السكربتات: يقرأ ملفات .lua من /data/zokii/scripts
// كل سكربت ممكن يعرّف: function on_start() / function on_tick() / function on_stop()
#pragma once
#include <string>
#include <vector>
#include <dirent.h>
#include <string.h>
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "natives.h"

struct Script { std::string name, path; bool enabled = false; lua_State* L = nullptr; };
static std::vector<Script> g_scripts;
extern void (*g_notify)(const char*);

static int l_notify(lua_State* L) { if (g_notify) g_notify(luaL_checkstring(L, 1)); return 0; }
static int l_god(lua_State* L) { invoke(H::SET_ENTITY_INVINCIBLE, player_ped(), (int)lua_toboolean(L, 1)); return 0; }
static int l_wanted(lua_State* L) { int n = (int)luaL_checkinteger(L, 1); invoke(H::SET_PLAYER_WANTED_LEVEL, 0, n, 0); invoke(H::SET_PLAYER_WANTED_NOW, 0, 0); return 0; }
static int l_teleport(lua_State* L) {
    invoke(H::SET_ENTITY_COORDS, player_ped(), (float)luaL_checknumber(L, 1), (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), 0, 0, 0, 1);
    return 0;
}
static int l_time(lua_State* L) { invoke(H::SET_CLOCK_TIME, (int)luaL_checkinteger(L, 1), (int)luaL_checkinteger(L, 2), 0); return 0; }

static lua_State* make_state() {
    lua_State* L = luaL_newstate();
    // بدون io/os عشان السكربت ما يوصل لملفات النظام
    luaL_requiref(L, "_G", luaopen_base, 1); lua_pop(L, 1);
    luaL_requiref(L, "math", luaopen_math, 1); lua_pop(L, 1);
    luaL_requiref(L, "string", luaopen_string, 1); lua_pop(L, 1);
    luaL_requiref(L, "table", luaopen_table, 1); lua_pop(L, 1);
    lua_register(L, "notify", l_notify);
    lua_register(L, "god", l_god);
    lua_register(L, "wanted", l_wanted);
    lua_register(L, "teleport", l_teleport);
    lua_register(L, "set_time", l_time);
    return L;
}
static void call_fn(Script& s, const char* fn) {
    if (!s.L) return;
    lua_getglobal(s.L, fn);
    if (lua_isfunction(s.L, -1)) { if (lua_pcall(s.L, 0, 0, 0)) lua_pop(s.L, 1); }
    else lua_pop(s.L, 1);
}
static void scripts_scan(const char* dir = "/data/zokii/scripts") {
    g_scripts.clear();
    DIR* d = opendir(dir);
    if (!d) return;
    while (dirent* e = readdir(d)) {
        size_t n = strlen(e->d_name);
        if (n > 4 && !strcmp(e->d_name + n - 4, ".lua")) {
            Script s; s.name = e->d_name; s.path = std::string(dir) + "/" + e->d_name;
            g_scripts.push_back(s);
        }
    }
    closedir(d);
}
static void script_set(Script& s, bool on) {
    if (on && !s.L) {
        s.L = make_state();
        if (luaL_dofile(s.L, s.path.c_str())) { if (g_notify) g_notify(lua_tostring(s.L, -1)); lua_close(s.L); s.L = nullptr; return; }
        s.enabled = true; call_fn(s, "on_start");
    } else if (!on && s.L) {
        call_fn(s, "on_stop"); lua_close(s.L); s.L = nullptr; s.enabled = false;
    }
}
static void scripts_tick() { for (auto& s : g_scripts) if (s.enabled) call_fn(s, "on_tick"); }

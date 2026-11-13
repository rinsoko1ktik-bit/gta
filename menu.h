// menu.h - إطار المنيو: تبويبات + خيارات (toggle / slider / action)
#pragma once
#include <vector>
#include <functional>
#include <stdio.h>
#include "natives.h"

#define BTN_L3 0x0002
#define BTN_UP 0x0010
#define BTN_RIGHT 0x0020
#define BTN_DOWN 0x0040
#define BTN_LEFT 0x0080
#define BTN_R1 0x0800
#define BTN_TRIANGLE 0x1000
#define BTN_CIRCLE 0x2000
#define BTN_CROSS 0x4000
#define BTN_SQUARE 0x8000

struct Item {
    enum Type { TOGGLE, SLIDER, ACTION } type;
    const char* name;
    bool* b = nullptr;
    float* f = nullptr; float mn = 0, mx = 1, step = 0.1f;
    std::function<void()> cb;
};
struct Tab { const char* name; std::vector<Item> items; };

struct Menu {
    bool open = false;
    int tab = 0, sel = 0;
    uint32_t prev = 0;
    std::vector<Tab> tabs;

    static Item Toggle(const char* n, bool* b) { Item i; i.type = Item::TOGGLE; i.name = n; i.b = b; return i; }
    static Item Slider(const char* n, float* f, float mn, float mx, float st) { Item i; i.type = Item::SLIDER; i.name = n; i.f = f; i.mn = mn; i.mx = mx; i.step = st; return i; }
    static Item Action(const char* n, std::function<void()> cb) { Item i; i.type = Item::ACTION; i.name = n; i.cb = cb; return i; }

    // نادِها مع حالة الأزرار؛ ترجع الأزرار بعد حجب اللي المنيو استهلكها عن اللعبة
    uint32_t input(uint32_t btn) {
        uint32_t pressed = btn & ~prev;
        prev = btn;
        if ((btn & BTN_R1) && (pressed & BTN_SQUARE)) { open = !open; return btn & ~(BTN_SQUARE); }
        if (!open || tabs.empty()) return btn;

        auto& items = tabs[tab].items;
        if (pressed & BTN_UP)   sel = (sel + (int)items.size() - 1) % items.size();
        if (pressed & BTN_DOWN) sel = (sel + 1) % items.size();
        if (pressed & BTN_L3)   { tab = (tab + 1) % tabs.size(); sel = 0; }
        if (pressed & BTN_TRIANGLE) { tab = (tab + (int)tabs.size() - 1) % tabs.size(); sel = 0; }
        if (pressed & BTN_CIRCLE) open = false;
        Item& it = items[sel];
        if (it.type == Item::TOGGLE && (pressed & BTN_CROSS)) *it.b = !*it.b;
        if (it.type == Item::ACTION && (pressed & BTN_CROSS) && it.cb) it.cb();
        if (it.type == Item::SLIDER) {
            if (pressed & BTN_LEFT)  { *it.f -= it.step; if (*it.f < it.mn) *it.f = it.mn; }
            if (pressed & BTN_RIGHT) { *it.f += it.step; if (*it.f > it.mx) *it.f = it.mx; }
        }
        return 0; // المنيو مفتوح: نحجب كل الأزرار عن اللعبة
    }

    void draw() {
        if (!open || tabs.empty()) return;
        const float X = 0.05f, W = 0.28f;
        draw_rect(X + W / 2, 0.06f, W, 0.07f, 130, 0, 200, 230);
        draw_text("zokii mod menu", X + 0.01f, 0.035f, 0.6f, 255, 255, 255, 255);
        char t[64]; snprintf(t, sizeof t, "< %s >", tabs[tab].name);
        draw_rect(X + W / 2, 0.11f, W, 0.035f, 30, 30, 30, 220);
        draw_text(t, X + 0.01f, 0.097f, 0.38f, 200, 160, 255, 255);

        auto& items = tabs[tab].items;
        for (size_t i = 0; i < items.size(); i++) {
            float y = 0.15f + i * 0.036f;
            bool s = (int)i == sel;
            draw_rect(X + W / 2, y + 0.016f, W, 0.034f, s ? 110 : 15, s ? 40 : 15, s ? 180 : 15, 200);
            char line[96];
            Item& it = items[i];
            if (it.type == Item::TOGGLE) snprintf(line, sizeof line, "%s  [%s]", it.name, *it.b ? "ON" : "OFF");
            else if (it.type == Item::SLIDER) snprintf(line, sizeof line, "%s  < %.1f >", it.name, *it.f);
            else snprintf(line, sizeof line, "%s", it.name);
            draw_text(line, X + 0.01f, y, 0.36f, 255, 255, 255, 255);
        }
        draw_text("R1+[] open | X select | O close | L3/TRI tabs", X, 0.15f + items.size() * 0.036f + 0.01f, 0.28f, 180, 180, 180, 255);
    }
};

// offsets.h - جدول الـ offsets لكل نسخة 1.00 .. 1.52
// ⚠ القيم صفر = ما انملت بعد. لازم تجيبها من نسختك (IDA/Ghidra أو مجتمع الهوم براو).
// المنيو ما يشتغل على نسخة قيمها صفر، ويطلع إشعار بدل ما يكرّش اللعبة.
#pragma once
#include <stdint.h>
#include <string.h>

struct GameOffsets {
    const char* version;
    uint64_t get_native_handler; // دالة: handler = fn(hash)
    uint64_t game_tick;          // مكان الهوك اللي ينادي كل فريم
};

#define V(s) { s, 0x0, 0x0 }
static GameOffsets kOffsets[] = {
    V("1.00"), V("1.01"), V("1.02"), V("1.03"), V("1.04"), V("1.05"),
    V("1.06"), V("1.07"), V("1.08"), V("1.09"), V("1.10"), V("1.11"),
    V("1.12"), V("1.13"), V("1.14"), V("1.15"), V("1.16"), V("1.17"),
    V("1.18"), V("1.19"), V("1.20"), V("1.21"), V("1.22"), V("1.23"),
    V("1.24"), V("1.25"), V("1.26"), V("1.27"), V("1.28"), V("1.29"),
    V("1.30"), V("1.31"), V("1.32"), V("1.33"), V("1.34"), V("1.35"),
    V("1.36"), V("1.37"), V("1.38"), V("1.39"), V("1.40"), V("1.41"),
    V("1.42"), V("1.43"), V("1.44"), V("1.45"), V("1.46"), V("1.47"),
    V("1.48"), V("1.49"), V("1.50"), V("1.51"), V("1.52"),
};
#undef V

static const GameOffsets* find_offsets(const char* ver) {
    for (auto& o : kOffsets)
        if (!strcmp(o.version, ver)) return &o;
    return nullptr;
}

// يقرأ APP_VER من param.sfo
#include <stdio.h>
static bool read_app_ver(char* out, size_t n) {
    FILE* f = fopen("/app0/sce_sys/param.sfo", "rb");
    if (!f) return false;
    static uint8_t buf[4096];
    size_t len = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (len < 20 || memcmp(buf, "\0PSF", 4)) return false;
    uint32_t keyOff = *(uint32_t*)(buf + 8), dataOff = *(uint32_t*)(buf + 12);
    uint32_t count = *(uint32_t*)(buf + 16);
    for (uint32_t i = 0; i < count; i++) {
        uint8_t* e = buf + 20 + i * 16;
        uint16_t kOff = *(uint16_t*)e;
        uint32_t dOff = *(uint32_t*)(e + 12);
        if (!strcmp((char*)buf + keyOff + kOff, "APP_VER")) {
            strncpy(out, (char*)buf + dataOff + dOff, n - 1);
            out[n - 1] = 0;
            return true;
        }
    }
    return false;
}

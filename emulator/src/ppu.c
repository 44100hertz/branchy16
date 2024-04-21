#include "ppu.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "cpu.h"

#define M_PI 3.1415926535

enum {
    CYCLES_PER_HDRAW = 6,
    CYCLES_PER_HBLANK = 2,
    CYCLES_PER_HLINE = CYCLES_PER_HBLANK + CYCLES_PER_HDRAW,
    VBLANK_LINES = 40,

    ADDR_VBLANK_LOCK = 0xf100,
    ADDR_HBLANK_LOCK = 0xf102,
    ADDR_SCANLINE_COUNT = 0xf103,

    ADDR_BG_COLOR = 0xf10f,
};

static bool ignore_poke = false;
static Color bg_color;
static Color screenbuf[SCREEN_SIZE];

static void ppu_poke(word addr, word value);
static Color word_to_color(word c);
static Color hue_table[256];
static void set_pixel_color(byte x, byte y, Color value);

void ppu_init() {
    set_poke_callback(1, ppu_poke);

    // make screen opaque
    for (uintptr_t i = 0; i < SCREEN_SIZE; ++i) {
        screenbuf[i].r = 255;
        screenbuf[i].a = 255;
    }
    bg_color = word_to_color(0);

    double phases[3] = {-0.3, -0.1, 0.5};
    double blackish[3] = {0.2, 0, 0.4};

    for (uintptr_t i = 0; i < 256; ++i) {
        double phase = i / (256.0);
        for (uintptr_t c = 0; c < 3; ++c) {
            double hue = sin(M_PI * 2.0 * (phase + phases[c]));
            if (hue < blackish[c]) hue = blackish[c];
            hue_table[i].chan[c] = hue * 255;
        }
    }
}

static Color word_to_color(word c) {
    Color hue = hue_table[c >> 8];
    float lum = (c & 0xff) / 255.0;

    byte whitish[3] = {255, 230, 204};

    Color color;
    for (uintptr_t i = 0; i < 3; ++i) {
        color.chan[i] = (1.0 - lum) * hue.chan[i] + lum * whitish[i];
    }
    color.a = 255;
    return color;
}

static void ppu_poke(word addr, word value) {
    if (ignore_poke) return;
    if (addr == ADDR_BG_COLOR) {
        bg_color = word_to_color(value);
    }
}

bool ppu_frame() {
    for (uintptr_t y = 0; y < SCREEN_HEIGHT; ++y) {
        io_store(ADDR_SCANLINE_COUNT, y);
        // ignore pokes during scanline draw
        ignore_poke = true;
        // write to HBLANK lock 1 cycle early
        cpu_step_multiple(CYCLES_PER_HDRAW - 1);

        for (uintptr_t x = 0; x < SCREEN_WIDTH; ++x) {
            set_pixel_color(x, y, bg_color);
        }

        // write to HBLANK lock 1 cycle early
        io_store(ADDR_HBLANK_LOCK, 0);
        cpu_step();
        //  allow pokes during HBLANK
        ignore_poke = false;
        cpu_step_multiple(CYCLES_PER_HBLANK);
    }

    // VBLANK
    ignore_poke = false;
    io_store(ADDR_VBLANK_LOCK, 0);
    bool running = cpu_step_multiple(VBLANK_LINES * CYCLES_PER_HLINE);
    return running;
}

static void set_pixel_color(byte x, byte y, Color value) {
    screenbuf[y * SCREEN_WIDTH + x] = value;
}

Color *ppu_screen() { return screenbuf; }

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

    A_VBLANK_LOCK = 0xf100,
    A_HBLANK_LOCK = 0xf102,
    A_SCANLINE_COUNT = 0xf103,

    A_PALETTE_START = 0xf180,
    PALETTE_SIZE = 0x40,
    A_PALETTE_END = 0xf1C0,
    A_BG_COLOR = 0xf10f,

    A_BG_0 = 0xf120,
};

typedef struct {
    byte scroll_x, scroll_y;
    word o_pattern;
    word o_attribute;
} BgLayer;

static Color hue_table[256];

extern word cpu_memory[CPU_MEMSIZE];
static bool ignore_poke;
static byte scanline_count;
static Color bg_color;
static Color screenbuf[SCREEN_SIZE];
static Color palette[PALETTE_SIZE];

static void ppu_poke(word addr, word value);
static word ppu_peek(word addr);
static Color word_to_color(word c);
static void set_pixel_color(byte x, Color value);

static void draw_bg_scanline(BgLayer b);
static void draw_tile_slice(byte x, byte i_pal, word i_pattern, bool reverse,
                            bool swapxy);

BgLayer bg_0;

bool ppu_frame() {
    for (scanline_count = 0; scanline_count < SCREEN_HEIGHT; scanline_count++) {
        io_store(A_SCANLINE_COUNT, scanline_count);
        // ignore pokes during scanline draw
        ignore_poke = true;
        // write to HBLANK lock 1 cycle early
        cpu_step_multiple(CYCLES_PER_HDRAW - 1);
        io_store(A_HBLANK_LOCK, 0);
        cpu_step();

        for (uintptr_t x = 0; x < SCREEN_WIDTH; ++x) {
            set_pixel_color(x, bg_color);
        }
        draw_bg_scanline(bg_0);

        // allow pokes during HBLANK
        ignore_poke = false;
        cpu_step_multiple(CYCLES_PER_HBLANK);
    }

    // VBLANK
    ignore_poke = false;
    io_store(A_VBLANK_LOCK, 0);
    bool running = cpu_step_multiple(VBLANK_LINES * CYCLES_PER_HLINE);
    return running;
}

void ppu_init() {
    set_poke_callback(1, ppu_poke);
    set_peek_callback(1, ppu_peek);

    // make screen opaque and red
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

    for (uintptr_t i = 0; i < PALETTE_SIZE; ++i) {
        palette[i] = word_to_color(i * 1028);
    }

    bg_0 = (BgLayer){0};
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

static void bg_poke(BgLayer *bg, word addr, word value) {
    switch (addr & 0xf) {
    case 0: bg->o_pattern = value / 2; break;  // treat as 32-bit offset
    case 1: bg->o_attribute = value; break;
    case 4: bg->scroll_x = value; break;
    case 5: bg->scroll_y = value; break;
    }
}

void ppu_poke(word addr, word value) {
    if (ignore_poke) return;
    if (addr == A_BG_COLOR) {
        bg_color = word_to_color(value);
    } else if (addr >= A_PALETTE_START && addr < A_PALETTE_END) {
        palette[addr - A_PALETTE_START] = word_to_color(value);
    } else if (addr >= A_BG_0 && addr < A_BG_0 + 0x10) {
        bg_poke(&bg_0, addr, value);
    }
}

word ppu_peek(word addr) {
    if (addr == A_SCANLINE_COUNT) {
        return scanline_count;
    }
    return 0;
}

static void set_pixel_color(byte x, Color value) {
    if (x >= SCREEN_WIDTH) return;
    screenbuf[scanline_count * SCREEN_WIDTH + x] = value;
}

Color *ppu_screen() { return screenbuf; }

static void draw_bg_scanline(BgLayer bg) {
    // calculate tile line to draw
    byte source_line = scanline_count - bg.scroll_y;  // wrapping subtraction
    byte pattern_offset_y = source_line % 8;          // index within tile
    // base offset of desired attribute
    word i_attrib = bg.o_attribute + (source_line / 8) * 32;

    for (byte x = 0; x < 32; ++x) {
        // get attribute word.
        // don't get attributes from outside of memory.
        if (i_attrib + x > CPU_MEMSIZE) return;
        word attrib = cpu_memory[i_attrib + x];

        // attribute: TTTT TTTT _CCC XY__
        byte i_tile = attrib >> 8;
        byte i_pal = attrib >> 4 & 0x7;
        bool flipx = attrib >> 3 & 0x1;
        bool flipy = attrib >> 2 & 0x1;
        bool swapxy = attrib >> 1 & 0x1;

        // get pattern line from specified tile
        word i_pattern = bg.o_pattern + i_tile * 8 |
                         (flipy ? 7 - pattern_offset_y : pattern_offset_y);

        draw_tile_slice(x * 8 + bg.scroll_x, i_pal, i_pattern, flipx, swapxy);
    }
}

// draw an 8-pixel slice for a tile or sprite.
// @i_pal: palette index 0-8
// @i_pattern: tile offset of pattern (32-bit index)
static void draw_tile_slice(byte x, byte i_pal, word i_pattern, bool reverse,
                            bool swapxy) {
    // only draw if pattern is within memory bounds
    if (i_pattern >= CPU_MEMSIZE / 2) return;
    if (i_pattern + 1 >= CPU_MEMSIZE / 2) return;
    if (swapxy && i_pattern + 8 >= CPU_MEMSIZE / 2) return;

    for (byte i = 0; i < 8; ++i) {
        // get pattern nibble
        byte nib;
        byte o = reverse ? i : 7 - i;
        if (swapxy) {
            byte pat_high = (i_pattern & 0xfff8) + o;
            byte pat_low = i_pattern & 0x0003;
            bool high_word = !(i_pattern & 0x4);
            nib = cpu_memory[pat_high * 2 + high_word] >> (pat_low * 4);
        } else {
            nib = cpu_memory[i_pattern * 2 + 1 - o / 4] >> ((o % 4) * 4);
        }
        nib &= 0xf;
        // don't draw transparent pixels
        if (nib & 0x8) continue;
        // draw final pixel from palette
        set_pixel_color(x + i, palette[(8 * i_pal | nib) % PALETTE_SIZE]);
    }
}

#pragma once
#include <stdint.h>

enum {
    SCREEN_WIDTH = 240,
    SCREEN_HEIGHT = 160,
    SCREEN_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT,
};

typedef union {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint8_t chan[4];
} Color;

void ppu_init();
bool ppu_frame();
Color *ppu_screen();

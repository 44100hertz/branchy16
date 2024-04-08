#pragma once
#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

enum {
    CPU_MEMSIZE = 1 << 16,
    CPU_NUM_BRANCHES = 1024,
    CPU_ITAG_OFFSET = 11,
};

enum {
    ITAG_LOAD,
    ITAG_STORE,
    ITAG_JUMP,
    ITAG_BRANCH,
    ITAG_PUTC,
    ITAG_HALT,
};

void cpu_init();
bool cpu_step();  // Returns true if CPU is still running
void cpu_store(word addr, word value, bool indirect);
word cpu_load(word addr, bool indirect);

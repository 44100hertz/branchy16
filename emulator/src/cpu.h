#pragma once
#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

enum {
    CPU_MEMSIZE = 1 << 16,
    CPU_NUM_BRANCHES = 1024,
    CPU_ITAG_OFFSET = 11,
    CPU_UNARY_MASK = 0b11110 << CPU_ITAG_OFFSET,
};

// See cpu.c for instruction documentation
enum {
    // 0xxxx special instructions
    ITAG_LOAD = 0,
    ITAG_STORE,
    ITAG_JUMP,
    ITAG_BRANCH,
    ITAG_COMPARE,
    ITAG_PUTC,
    ITAG_HALT,
    // UNUSED: 8-16
    //
    // 1xxxx binary arithmetic
    ITAG_ADD = 0b10000,
    // 1111x unary arithmetic; rest of operation encoded in low bits
    ITAG_COPY = 0,
};

static inline word cpu_encode_unary(byte instr) {
    return CPU_UNARY_MASK | (instr >> 4 & 1) << 11 | (instr & 0xf);
}

static inline byte cpu_decode_unary(word instr) {
    return ((instr >> 11) & 1) << 4 | (instr & 0xf);
}

enum {
    COND_FLAG_EQ = 1,
    COND_FLAG_LT = 2,
    COND_FLAG_GT = 4,
};

void cpu_init();
bool cpu_step();  // Returns true if CPU is still running
void cpu_store(word addr, word value, bool indirect);
word cpu_load(word addr, bool indirect);
void override_putchar(int (*)(int));

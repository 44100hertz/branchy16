#pragma once
#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef void (*poke_cb)(word addr, word value);
typedef word (*peek_cb)(word addr);

enum {
    CPU_MEMSIZE = 0xf000,
    IO_SIZE = 0x1000,
    CPU_NUM_BRANCHES = 0x100,
    CPU_ITAG_OFFSET = 11,
    CPU_UNARY_MASK = 0b11110 << CPU_ITAG_OFFSET,
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
bool cpu_step_multiple(int steps);
void cpu_store(word addr, word value);
word cpu_load(word addr);

void set_poke_callback(int device, poke_cb);
void io_store(word addr, word value);
word io_load(word addr);

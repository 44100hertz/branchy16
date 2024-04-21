#pragma once
#include <stdint.h>
#pragma once
#include <stdbool.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef void (*poke_cb)(word addr, word value);
typedef word (*peek_cb)(word addr);

enum {
    CPU_MEMSIZE = 0xf000,
    IO_SIZE = 0x1000,
    CPU_NUM_BRANCHES = 128,
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

typedef enum {
    LS_NONE = 0,
    LS_LOAD,
    LS_LOADWAIT,
    LS_STORE,
} LoadStoreState;

typedef struct {
    word reg[8];
    word bp;
    word pc;

    // special state for memory operations
    word mem_addr;  // target addr for memory operations
    word mem_val;   // register or value for memory operations
    LoadStoreState ls_state;

    byte compare_flags;
    bool running;
} CpuBranch;

void cpu_init();
void set_poke_callback(int device, poke_cb);
void cpu_write_binary(int len, word binary[len]);
bool cpu_step();  // Returns true if CPU is still running
bool cpu_step_multiple(int steps);
void cpu_store(word addr, word value);
void io_store(word addr, word value);
word cpu_load(word addr);

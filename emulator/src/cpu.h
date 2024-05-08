#pragma once
#include <stdint.h>
#pragma once
#include <stdbool.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef void (*PokeCb)(word addr, word value);
typedef word (*PeekCb)(word addr);

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
    CPU_FLAG_RUNNING = 1,
    CPU_FLAG_CARRY = 2,
    CPU_FLAG_ZERO = 4,
    CPU_FLAG_NEG = 8,
    CPU_FLAG_EQ = 0x10,
    CPU_FLAG_LT = 0x20,
    CPU_FLAG_GT = 0x40,
    CPU_FLAG_CONST_1 = 0x80,
};

enum {
    CPU_COMPARE_MASK = 0x70,
};

#define GETFLAG(br, name) (0 != ((br).flags & CPU_FLAG_##name))

#define SETFLAG(br, name, value) \
    (br).flags =                 \
        ((br).flags & (~CPU_FLAG_##name)) | (value) ? CPU_FLAG_##name : 0

#define COMPAREFLAGS(br) (((br).flags >> 4) & 0x7)

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

    byte flags;
} CpuBranch;

void cpu_init();
void set_poke_callback(int device, PokeCb);
void set_peek_callback(int device, PeekCb);
void cpu_write_binary(int len, word binary[len]);
bool cpu_step();  // Returns true if CPU is still running
bool cpu_step_multiple(int steps);
void cpu_store(word addr, word value);
void io_store(word addr, word value);
word cpu_load(word addr);

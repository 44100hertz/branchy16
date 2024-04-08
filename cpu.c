#include "cpu.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    word reg[8];
    word bp;
    word pc;
    bool running;

    // special state for memory operations
    bool store_enable;  // do a store operation
    bool mem_indirect;  // enable indirection on memory access
    byte mem_reg;       // register for memory operations
    word mem_target;    // target addr for memory operations
} cpu_branch;

static word cpu_memory[CPU_MEMSIZE];
static cpu_branch cpu_branches[CPU_NUM_BRANCHES];

static word branch_fetch(cpu_branch *br);
static void branch_step(cpu_branch *br);
static void cpu_branch_create(cpu_branch *source, word pc, word bp);

void cpu_init() {
    // Zero memory and branch state
    memset(cpu_memory, 0, sizeof(cpu_memory));
    memset(cpu_branches, 0, sizeof(cpu_branches));
    // Run branch 0
    cpu_branches[0] = (cpu_branch){
        .running = true,
    };
}

bool cpu_step() {
    bool running = false;
    cpu_branch *last = &cpu_branches[CPU_NUM_BRANCHES];
    for (cpu_branch *br = cpu_branches; br < last; ++br) {
        if (br->running) {
            branch_step(br);
        }
        running = running || br->running;
    }
    return running;
}

// Common pattern: a nibble will define a parameter.
// If top = 0, it's an immediate value.
// If top = 1, it loads a register indexed by the last 3 bits.
#define ARG_NIBBLE(offset)                                          \
    ((instr & (1 << (offset + 3))) ? br->reg[instr >> offset & 0x3] \
                                   : branch_fetch(br))

// Step a single CPU branch
void branch_step(cpu_branch *br) {
    word instr = branch_fetch(br);
    // top 5 bits determine instruction
    switch (instr >> CPU_ITAG_OFFSET) {
        case ITAG_LOAD: {
            // Load = xxxxx DDD SSSS -oAA
            // D = destination register
            word *dest = &br->reg[instr >> 8 & 0x7];
            // Address mode zero. Load a zero and quit.
            if ((instr & 0x3) == 0) {
                *dest = 0;
                break;
            }
            // r = load target from register
            // S = source register (only matters if r is set)
            word target = ARG_NIBBLE(4);
            // o = use relative address
            if (instr & 4) target += br->bp;
            // AA = addressing mode.
            // 0 = zero, 1 = immediate, 2 = address, 3 = indirect
            if (instr & 2) target = cpu_load(target, instr & 1);
            *dest = target;
            break;
        }
        case ITAG_STORE: {
            // Store = xxxxx SSS DDDD -oA-
            // Store does not immediately store the value, but instead sets up a
            // store. This allows writes to happen after all loads have
            // finished.
            br->store_enable = true;
            // S = source register
            br->mem_reg = instr >> 8 & 0x7;
            // A = addressing indirect
            br->mem_indirect = instr & 2;
            // r = load target from register
            // D = dest register (only if r is set)
            word target = ARG_NIBBLE(4);
            // o = use relative address
            if (instr & 4) target += br->bp;
            br->mem_target = target;
            break;
        }
        case ITAG_JUMP: {
            // Jump = xxxxx --i AAAA ----
            word addr = ARG_NIBBLE(4);
            // i = jump indirect (only for immediate value)
            if (instr & 1 << 8 && !(instr & 1 << 7)) addr = cpu_load(addr, 0);
            br->pc = addr;
            break;
        }
        case ITAG_BRANCH: {
            // Branch = xxxxx --i AAAA OOOO

            // A = register or immediate
            word addr = ARG_NIBBLE(4);
            // i = jump indirect (only for immediate value)
            if (instr & 1 << 8 && !(instr & 1 << 7)) addr = cpu_load(addr, 0);
            // O = register or immediate
            word offset = ARG_NIBBLE(0);
            cpu_branch_create(br, addr, offset);
            break;
        }
        case ITAG_PUTC: {
            // Putchar = xxxxx --- ---- rSSS
            word c = ARG_NIBBLE(0);
            putchar(c);
            break;
        }
        case ITAG_HALT:
            br->running = false;
            break;
    }
}

word branch_fetch(cpu_branch *br) {
    word out = cpu_memory[br->pc % CPU_MEMSIZE];
    ++br->pc;
    return out;
}

// Create a branch from a running branch
void cpu_branch_create(cpu_branch *source, word pc, word bp) {
    // Find a branch that isn't running.
    uintptr_t new_index = 0;
    for (uintptr_t index = 0; index < CPU_NUM_BRANCHES; ++index) {
        if (!cpu_branches[index].running) {
            new_index = index;
            break;
        }
    }

    // If we allocated every branch, forget it.
    if (!new_index) {
        return;
    }

    // Copy contents of running branch into new one
    cpu_branches[new_index] = *source;
    // set program counter and address offset
    cpu_branches[new_index].pc = pc;
    cpu_branches[new_index].bp = bp;
}

void cpu_store(word addr, word value, bool indirect) {
    if (indirect) addr = cpu_memory[addr];
    cpu_memory[addr] = value;
}

word cpu_load(word addr, bool indirect) {
    if (indirect) addr = cpu_memory[addr];
    return cpu_memory[addr];
}

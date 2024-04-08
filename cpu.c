#include "cpu.h"

#define DEBUG true

#include <stdio.h>
#include <string.h>

typedef struct {
    word reg[8];
    word bp;
    word pc;
    byte compare_flags;

    // special state for memory operations
    word mem_target;    // target addr for memory operations
    byte mem_reg;       // register for memory operations
    bool mem_indirect;  // enable indirection on memory access
    bool store_enable;  // do a store operation

    bool running;
} cpu_branch;

static word cpu_memory[CPU_MEMSIZE];
static cpu_branch cpu_branches[CPU_NUM_BRANCHES];

static word branch_fetch(cpu_branch *br);
static void branch_step(cpu_branch *br);
static void branch_step_special(cpu_branch *br, word);
static void branch_step_binary(cpu_branch *br, word);
static void branch_step_unary(cpu_branch *br, word);
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
// If top = 1, it's an immediate value.
// If top = 0, it loads a register indexed by the last 3 bits.
#define ARG_NIBBLE(offset)                            \
    ((instr & (1 << (offset + 3))) ? branch_fetch(br) \
                                   : br->reg[instr >> offset & 0x7])

// Step a single CPU branch
void branch_step(cpu_branch *br) {
    word instr = branch_fetch(br);
    if ((instr & CPU_UNARY_MASK) == CPU_UNARY_MASK) {
        branch_step_unary(br, instr);
    } else if (instr >> 15) {
        branch_step_binary(br, instr);
    } else {
        branch_step_special(br, instr);
    }
}

void branch_step_special(cpu_branch *br, word instr) {
    switch (instr >> CPU_ITAG_OFFSET) {
    case ITAG_LOAD: {
        // Load = xxxxx -oi -DDD TTTT
        // T = address target
        word target = ARG_NIBBLE(0);
        // o = use relative address
        if (instr & (1 << 9)) target += br->bp;
        // i = use indirect
        // D = destination register
        br->reg[instr >> 4 & 0x7] = cpu_load(target, instr & (1 << 8));
        break;
    }
    case ITAG_STORE: {
        // Store = xxxxx -oi DDDD -SSS
        // Store does not immediately store the value, but instead sets up a
        // store. This allows writes to happen after all loads have
        // finished.
        br->store_enable = true;
        // S = source register
        br->mem_reg = instr & 0x7;
        // A = addressing indirect
        br->mem_indirect = instr & (1 << 8);
        // D = destination
        word target = ARG_NIBBLE(4);
        // o = use relative address
        if (instr & (1 << 9)) target += br->bp;
        br->mem_target = target;
        break;
    }
    case ITAG_JUMP: {
        // Jump = xxxxx --i AAAA -CCC
        word addr = ARG_NIBBLE(4);
        // i = jump indirect (only for immediate value)
        if (instr & 1 << 8 && instr & 1 << 7) addr = cpu_load(addr, 0);
        // Jump if compare flags are satisfied
        word requirement = instr & 0x7;
        if (requirement & br->compare_flags) {
            br->pc = addr;
        }
        break;
    }
    case ITAG_BRANCH: {
        // Branch = xxxxx --i AAAA OOOO

        // A = register or immediate
        word addr = ARG_NIBBLE(4);
        // i = branch indirect (only for immediate value)
        if (instr & 1 << 8 && instr & 1 << 7) addr = cpu_load(addr, 0);
        // O = register or immediate
        word offset = ARG_NIBBLE(0);
        cpu_branch_create(br, addr, offset);
        break;
    }
    case ITAG_PUTC: {
        // Putchar = xxxxx --- ---- SSSS
        word c = ARG_NIBBLE(0);
        putchar(c);
        break;
    }
    case ITAG_COMPARE: {
        // Compare = xxxxx --- -AAA BBBB
        word a = br->reg[instr >> 4 & 0x7];
        word b = ARG_NIBBLE(0);

        br->compare_flags = (a == b ? COND_FLAG_EQ : 0) |
                            (a < b ? COND_FLAG_LT : 0) |
                            (a > b ? COND_FLAG_GT : 0);
        break;
    }
    case ITAG_HALT: br->running = false; break;
    }
}

void branch_step_binary(cpu_branch *br, word instr) {
    // Binary = xxxxx OOO AAAA BBBB
    word out;
    word a = ARG_NIBBLE(4);
    word b = ARG_NIBBLE(0);
    switch (instr >> CPU_ITAG_OFFSET) {
    case ITAG_ADD: out = a + b; break;
    }
    br->reg[instr >> 8 & 0x7] = out;
}

void branch_step_unary(cpu_branch *br, word instr) {
    // Unary = 1111x OOO AAAA xxxx
    word out;
    word a = ARG_NIBBLE(4);
    word operation = cpu_decode_unary(instr);
    switch (operation) {
    case ITAG_COPY: out = a; break;
    }
    br->reg[instr >> 8 & 0x7] = out;
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

#include "cpu.h"

#include <stdio.h>
#include <string.h>

#include "consts.h"

typedef struct {
    word reg[8];
    word bp;
    word pc;
    byte compare_flags;

    // special state for memory operations
    word mem_addr;      // target addr for memory operations
    word mem_val;       // register or value for memory operations
    bool store_enable;  // do a store operation
    bool load_wait;     // wait for another branch to store

    bool running;
} cpu_branch;

static word cpu_memory[CPU_MEMSIZE];
static cpu_branch cpu_branches[CPU_NUM_BRANCHES];

static inline bool branch_running(cpu_branch *br) {
    return br->running && !br->load_wait;
}
static word branch_fetch(cpu_branch *br);
static void branch_step(cpu_branch *br);
static void branch_step_special(cpu_branch *br, word);
static void branch_step_binary(cpu_branch *br, word);
static void branch_step_unary(cpu_branch *br, word);
static void branch_start(cpu_branch *source, word pc, word bp);
static void branch_clear_loadwait(cpu_branch *br, word value);

static void print_cb(word addr, word value) { putchar(value); }

static poke_cb device_poke_cbs[0x10] = {[0 ... 15] = print_cb};

static void poke(word addr, word value) {
    device_poke_cbs[addr >> 8 & 0xf](addr & 0xff, value);
}

void set_poke_callback(int index, poke_cb fn) {
    device_poke_cbs[index & 0xf] = fn;
}

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
    static bool did_write[CPU_MEMSIZE];

    for (cpu_branch *br = cpu_branches; br < last; ++br) {
        // either finish load or execute next instruction
        if (br->load_wait && br->mem_addr < CPU_MEMSIZE &&
            did_write[br->mem_addr])
        {
            branch_clear_loadwait(br, cpu_memory[br->mem_addr]);
            // only unlock one waiting branch at a time, except for I/O
            did_write[br->mem_addr] = false;
        } else if (branch_running(br)) {
            branch_step(br);
        }
        // branch step can affect running, do not put this in the 'if'
        running = running || branch_running(br);
    }

    // Run every store operation at once. Multiple parallel stores will OR each
    // other. Other than side effects, branch execution order doesn't matter.
    memset(did_write, 0, sizeof(did_write));

    for (cpu_branch *br = cpu_branches; br < last; ++br) {
        if (br->store_enable) {
            word addr = br->mem_addr;
            if (addr >= CPU_MEMSIZE) {
                // pokes are executed sequentially in any order
                poke(addr, br->mem_val);
            } else {
                // first write clears memory addr
                if (!did_write[addr]) cpu_memory[addr] = 0;
                did_write[addr] = true;
                cpu_memory[addr] |= br->mem_val;
            }
        }
        br->store_enable = false;
    }

    return running;
}

bool cpu_step_multiple(int steps) {
    bool running = true;
    for (int i = 0; i < steps && running; ++i) {
        running = cpu_step();
    }
    return running;
}

// Common pattern: a nibble will define a parameter.
// 0-7 = register
// 8 = immediate
// 9 = zero
// 10 = 1
// 11 = -1/0xffff
#define ARG_NIBBLE(offset)                                           \
    (instr >> offset & 0xf) < 8     ? br->reg[instr >> offset & 0x7] \
    : (instr >> offset & 0xf) == 8  ? branch_fetch(br)               \
    : (instr >> offset & 0xf) == 9  ? 0                              \
    : (instr >> offset & 0xf) == 10 ? 1                              \
                                    : -1;

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
        // Load = xxxxx wo- dDDD TTTT
        // T = address target
        word target = ARG_NIBBLE(0);
        // o = use relative address
        if (instr & (1 << 9)) target += br->bp;
        // D = destination. If d is set, do no load.
        uintptr_t dest = instr >> 4 & 0xf;
        // w = wait for another branch to write, then load
        if (instr & (1 << 10)) {
            br->mem_addr = target;
            br->load_wait = true;
            br->store_enable = false;
            br->mem_val = dest;
        } else {
            if (dest < 8) {
                br->reg[dest] = cpu_load(target);
            }
        }
        break;
    }
    case ITAG_STORE: {
        // Store = xxxxx -o- DDDD SSSS
        // Store does not immediately store the value, but instead sets up a
        // store. This allows writes to happen after all loads have
        // finished.
        br->store_enable = true;
        // D = destination
        word target = ARG_NIBBLE(4);
        // S = source register
        br->mem_val = ARG_NIBBLE(0);
        // o = use relative address
        if (instr & (1 << 9)) target += br->bp;
        br->mem_addr = target;
        break;
    }
    case ITAG_JUMP: {
        // Jump = xxxxx --- AAAA -CCC
        word addr = ARG_NIBBLE(4);
        // Jump if compare flags are satisfied
        word requirement = instr & 0x7;
        if (requirement == 7 || requirement & br->compare_flags) {
            br->pc = addr;
        }
        break;
    }
    case ITAG_BRANCH: {
        // Branch = xxxxx --- AAAA OOOO
        // A = register or immediate
        word addr = ARG_NIBBLE(4);
        // O = register or immediate
        word offset = ARG_NIBBLE(0);
        branch_start(br, addr, offset);
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
    // Binary = 1xxxx OOO AAAA BBBB
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
    case ITAG_UNARY_COPY: out = a; break;
    }
    br->reg[instr >> 8 & 0x7] = out;
}

word branch_fetch(cpu_branch *br) {
    word out = cpu_memory[br->pc];
    br->pc = (br->pc + 1) % CPU_MEMSIZE;
    return out;
}

// Create a branch from a running branch
void branch_start(cpu_branch *source, word pc, word bp) {
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

void branch_clear_loadwait(cpu_branch *br, word value) {
    if (br->mem_val < 8) {
        br->reg[br->mem_val] = value;
    }
    br->load_wait = false;
}

void cpu_store(word addr, word value) {
    if (addr >= CPU_MEMSIZE) {
        poke(addr & 0xfff, value);
    } else {
        cpu_memory[addr] = value;
    }
}

word cpu_load(word addr) {
    if (addr >= CPU_MEMSIZE) {
        return 0;
    } else {
        return cpu_memory[addr];
    }
}

void io_store(word addr, word value) {
    addr = addr | 0xf000;

    cpu_branch *last = &cpu_branches[CPU_NUM_BRANCHES];
    for (cpu_branch *br = cpu_branches; br < last; ++br) {
        if (br->load_wait && br->mem_addr == addr) {
            branch_clear_loadwait(br, value);
        }
    }
}

word io_load(word addr) {
    if (addr >= CPU_MEMSIZE) {
        return 0;
    }
    return cpu_memory[addr];
}

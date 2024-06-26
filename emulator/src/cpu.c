#include "cpu.h"

#include <string.h>

#include "consts.h"

word cpu_memory[CPU_MEMSIZE];
static CpuBranch cpu_branches[CPU_NUM_BRANCHES];

#define MAX_CPU_MEMWRITES 4
#define MAX_STORES 6
#define MAX_LOADS 4

typedef struct {
    word addr;
    word value;
    bool io;
} MemWrite;
static MemWrite memwrites[MAX_STORES];
static int num_memwrites;

static inline bool branch_running(CpuBranch *br) {
    return GETFLAG(*br, RUNNING) &&
           (br->ls_state != LS_LOADWAIT || br->mem_addr >= 0xf000);
}
static bool try_queue_store(word addr, word value, bool io);
static word branch_fetch(CpuBranch *br);
static void branch_step(CpuBranch *br);
static void branch_step_special(CpuBranch *br, word);
static void branch_step_binary(CpuBranch *br, word);
static void branch_step_unary(CpuBranch *br, word);
static void branch_start(CpuBranch *source, word pc, word bp);
static void branch_clear_loadwait(CpuBranch *br, word value);

static PokeCb device_poke_cbs[0x10] = {0};
static PeekCb device_peek_cbs[0x10] = {0};

void set_poke_callback(int index, PokeCb fn) {
    device_poke_cbs[index & 0xf] = fn;
}

void set_peek_callback(int index, PeekCb fn) {
    device_peek_cbs[index & 0xf] = fn;
}

void cpu_init() {
    // Zero memory and branch state
    memset(cpu_memory, 0, sizeof(cpu_memory));
    memset(cpu_branches, 0, sizeof(cpu_branches));
    // Run branch 0
    cpu_branches[0] = (CpuBranch){
        .flags = CPU_FLAG_RUNNING,
    };
}

void cpu_write_binary(int len, word binary[len]) {
    memcpy(cpu_memory, binary, len * sizeof(word));
}

bool cpu_step() {
    bool running = false;
    CpuBranch *last = &cpu_branches[CPU_NUM_BRANCHES];

    int load_count = 0;
    for (CpuBranch *br = cpu_branches; br < last; ++br) {
        // either finish load or execute next instruction
        if (!GETFLAG(*br, RUNNING)) continue;
        switch (br->ls_state) {
        case LS_NONE: branch_step(br); break;
        case LS_LOADWAIT:
            for (int i = 0; i < num_memwrites; ++i) {
                if (memwrites[i].addr == br->mem_addr) {
                    branch_clear_loadwait(br, memwrites[i].value);
                    // swap-remove
                    if (!memwrites[i].io) {
                        memwrites[i] = memwrites[--num_memwrites];
                    }
                    break;
                }
            }
            break;
        case LS_LOAD:
            if (load_count < MAX_LOADS) {
                if (br->mem_val < 8) {
                    br->reg[br->mem_val] = cpu_load(br->mem_addr);
                }
                br->ls_state = LS_NONE;
                ++load_count;
            }
            break;
        case LS_STORE: break;
        }
        running = running || branch_running(br);
    }

    // Run every store operation at once. Multiple parallel stores will OR
    // each other. Other than side effects, branch execution order doesn't
    // matter.
    num_memwrites = 0;
    for (CpuBranch *br = cpu_branches; br < last; ++br) {
        if (br->ls_state == LS_STORE) {
            if (try_queue_store(br->mem_addr, br->mem_val, false)) {
                br->ls_state = LS_NONE;
            }
        }
    }

    // Finalize writes
    for (int i = 0; i < num_memwrites; ++i) {
        cpu_store(memwrites[i].addr, memwrites[i].value);
    }
    return running;
}

static bool try_queue_store(word addr, word value, bool io) {
    uintptr_t max = io ? MAX_STORES : MAX_CPU_MEMWRITES;
    if (num_memwrites == max) return false;
    for (int i = 0; i < num_memwrites; ++i) {
        // if store exists, OR it
        if (memwrites[i].addr == addr) {
            memwrites[i].value |= value;
            return true;
        }
    }
    // otherwise add one to list
    memwrites[num_memwrites++] = (MemWrite){addr, value, io};
    return true;
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
void branch_step(CpuBranch *br) {
    word instr = branch_fetch(br);
    if ((instr & CPU_UNARY_MASK) == CPU_UNARY_MASK) {
        branch_step_unary(br, instr);
    } else if (instr >> 15) {
        branch_step_binary(br, instr);
    } else {
        branch_step_special(br, instr);
    }
}

void branch_step_special(CpuBranch *br, word instr) {
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
        br->ls_state = (instr >> 10 & 0x1) == 1 ? LS_LOADWAIT : LS_LOAD;
        br->mem_addr = target;
        br->mem_val = dest;
        break;
    }
    case ITAG_STORE: {
        // Store = xxxxx -o- DDDD SSSS
        // Store does not immediately store the value, but instead sets up a
        // store. This allows writes to happen after all loads have
        // finished.
        br->ls_state = LS_STORE;
        // D = destination
        word target = ARG_NIBBLE(4);
        // S = source
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
        if (requirement == 7 || requirement & COMPAREFLAGS(*br)) {
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
        // Compare = xxxxx --- AAAA BBBB
        word a = ARG_NIBBLE(4);
        word b = ARG_NIBBLE(0);

        br->flags &= ~CPU_COMPARE_MASK;
        br->flags |= (a == b ? CPU_FLAG_EQ : 0) |  //
                     (a < b ? CPU_FLAG_LT : 0) |   //
                     (a > b ? CPU_FLAG_GT : 0);
        break;
    }
    case ITAG_HALT: SETFLAG(*br, RUNNING, 0); break;
    }
}

void branch_step_binary(CpuBranch *br, word instr) {
    // Binary = 1xxxx OOO AAAA BBBB
    word out;
    word a = ARG_NIBBLE(4);
    word b = ARG_NIBBLE(0);
    switch (instr >> CPU_ITAG_OFFSET) {
    case ITAG_ADD: out = a + b; break;
    }
    br->reg[instr >> 8 & 0x7] = out;
}

void branch_step_unary(CpuBranch *br, word instr) {
    // Unary = 1111x OOO AAAA xxxx
    word out;
    word a = ARG_NIBBLE(4);
    word operation = cpu_decode_unary(instr);
    switch (operation) {
    case ITAG_UNARY_COPY: out = a; break;
    }
    br->reg[instr >> 8 & 0x7] = out;
}

word branch_fetch(CpuBranch *br) {
    word out = cpu_memory[br->pc];
    br->pc += 1;
    return out;
}

// Create a branch from a running branch
void branch_start(CpuBranch *source, word pc, word bp) {
    // Find a branch that isn't running.
    uintptr_t new_index = -1;
    for (uintptr_t index = 0; index < CPU_NUM_BRANCHES; ++index) {
        if (!GETFLAG(cpu_branches[index], RUNNING)) {
            new_index = index;
            break;
        }
    }

    // If we allocated every branch, forget it.
    if (new_index == -1) {
        return;
    }

    // Copy contents of running branch into new one
    cpu_branches[new_index] = *source;
    // set program counter and address offset
    cpu_branches[new_index].pc = pc;
    cpu_branches[new_index].bp = bp;
}

static void poke(word addr, word value) {
    PokeCb cb = device_poke_cbs[addr >> 8 & 0xf];
    if (cb) {
        cb(addr, value);
    }
}

static word peek(word addr) {
    PeekCb cb = device_peek_cbs[addr >> 8 & 0xf];
    if (cb) {
        return cb(addr);
    }
    return 0;
}

void branch_clear_loadwait(CpuBranch *br, word value) {
    if (br->mem_val < 8) {
        br->reg[br->mem_val] = value;
    }
    br->ls_state = LS_NONE;
}

void cpu_store(word addr, word value) {
    if (addr >= CPU_IO_ADDR) {
        poke(addr, value);
    } else {
        cpu_memory[addr] = value;
    }
}

word cpu_load(word addr) {
    if (addr >= CPU_IO_ADDR) {
        return peek(addr);
    } else {
        return cpu_memory[addr];
    }
}

void io_store(word addr, word value) {
    try_queue_store(addr | 0xf000, value, true);
}

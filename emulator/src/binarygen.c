#include "binarygen.h"

#include "consts.h"
#include "cpu.h"

// write instruction word
#define WW(word) (cpu_store(offset, word), offset++)
// write special or binary word
#define WI(instr, n0, n1, n2) \
    (cpu_store(offset, (ITAG_##instr) << 11 | n0 << 8 | n1 << 4 | n2), offset++)
// write unary instruction word
#define WU(instr, n0, n1)                                                 \
    (cpu_store(offset,                                                    \
               cpu_encode_unary(ITAG_UNARY_##instr) | n0 << 8 | n1 << 4), \
     offset++)

void write_branching_hello() {
    word offset = 0;
    word char_ptr = 0x100;
    word writer_offset = 0x200;

    // hello, world branching program
    // which duplicates characters using a second branch
    //
    WI(BRANCH, 0b000, IMMED, IMMED);
    WW(writer_offset);
    WW(char_ptr);

    WU(COPY, R0, IMMED);
    word hello_offset = offset++;
    // loop:
    word loop_offset = WI(LOAD, 0, R1, R0);
    WI(COMPARE, 0, R1, CONST_0);
    WI(JUMP, 0, IMMED, COND_EQ);
    word done_offset = offset++;
    WI(STORE, 0, IMMED, R1);
    WW(char_ptr);
    WI(ADD, R0, R0, CONST_1);
    WI(JUMP, 0, IMMED, COND_ALWAYS);
    WW(loop_offset);
    // done:
    cpu_store(done_offset, offset);
    WI(HALT, 0, 0, 0);

    cpu_store(hello_offset, offset);
    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WW(*c);
    WW(0);

    // second branch loop -- write chars twice
    offset = writer_offset;
    WI(LOAD, 0b110, R0, CONST_0);
    WI(STORE, 0, IMMED, R0);
    WW(0xf000);
    WI(STORE, 0, IMMED, R0);
    WW(0xf000);
    WI(JUMP, 0, IMMED, COND_ALWAYS);
    WW(writer_offset);
}

void write_hello() {
    word offset = 0;

    // hello, world program
    //
    WU(COPY, R0, IMMED);
    word hello_offset = offset++;
    // loop:
    word loop_offset = WI(LOAD, 0, R1, R0);
    WI(COMPARE, 0, R1, CONST_0);
    WI(JUMP, 0, IMMED, COND_EQ);
    word done_offset = offset++;
    WI(STORE, 0, IMMED, R1);
    WW(0xf000);
    WI(ADD, R0, R0, CONST_1);
    WI(JUMP, 0, IMMED, COND_ALWAYS);
    WW(loop_offset);
    // done:
    cpu_store(done_offset, offset);
    WI(HALT, 0, 0, 0);

    cpu_store(hello_offset, offset);
    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WW(*c);
    WW(0);
}

void write_display_busyloop() {
    // spam BG color writes
    word offset = 0;
    word loop = WI(LOAD, 0b100, R1, IMMED);  // loop
    WW(0xf102);                              //
    WI(STORE, 0, IMMED, R0);                 // write background color
    WW(0xf10f);                              //
    WI(ADD, R0, R0, IMMED);                  // increment background color
    WW(0x0008);                              //
    WI(JUMP, 0, IMMED, COND_ALWAYS);         // loop
    WW(loop);
}

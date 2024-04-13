#include "binarygen.h"

#include "consts.h"
#include "cpu.h"

// write instruction word
#define WW(word) (cpu_store(offset, word, false), offset++)
// write special or binary word
#define WI(instr, n0, n1, n2)                                                 \
    (cpu_store(offset, (ITAG_##instr) << 11 | n0 << 8 | n1 << 4 | n2, false), \
     offset++)
// write unary instruction word
#define WU(instr, n0, n1)                                                \
    (cpu_store(offset,                                                   \
               cpu_encode_unary(ITAG_UNARY_##instr) | n0 << 8 | n1 << 4, \
               false),                                                   \
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
    cpu_store(done_offset, offset, 0);
    WW(ITAG_HALT);

    cpu_store(hello_offset, offset, 0);
    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WW(*c);

    // second branch loop -- write chars twice
    offset = writer_offset;
    WI(LOAD, 0b110, R0, CONST_0);
    WI(PUTC, 0, 0, R0);
    WI(PUTC, 0, 0, R0);
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
    word loop_offset = offset;
    WI(LOAD, 0b000, R1, R0);
    WI(COMPARE, 0, R1, CONST_0);
    WI(JUMP, 0, IMMED, COND_EQ);
    word done_offset = offset++;
    WI(PUTC, 0, 0, R1);
    WI(ADD, R0, R0, CONST_1);
    WI(JUMP, 0, IMMED, COND_ALWAYS);
    WW(loop_offset);
    // done:
    cpu_store(done_offset, offset, 0);
    WW(ITAG_HALT);

    cpu_store(hello_offset, offset, 0);

    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WW(*c);
}

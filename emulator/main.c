#include <stdio.h>

#include "cpu.h"
#include "helpful_constants.h"
#ifdef TESTING
#include "tests.h"
#endif

#pragma GCC diagnostic ignored "-Wunused-function"
static void write_hello();
static void write_branching_hello();

// write instruction word
#define WRITE(word) (cpu_store(offset, word, false), offset++)
// write special or binary word
#define WRITE_I(instr, n0, n1, n2)                                            \
    (cpu_store(offset, (ITAG_##instr) << 11 | n0 << 8 | n1 << 4 | n2, false), \
     offset++)
// write unary instruction word
#define WRITE_U(instr, n0, n1)                                             \
    (cpu_store(offset, cpu_encode_unary(ITAG_##instr) | n0 << 8 | n1 << 4, \
               false),                                                     \
     offset++)

int main(int argc, char **argv) {
#ifdef TESTING
    run_tests();
    return 0;
#endif
    cpu_init();
    write_branching_hello();

    for (int i = 0; i < 1000; ++i) {
        bool running = cpu_step();
        if (!running) {
            printf("Executed %d cycles.\n", i);
            break;
        }
    }
}

void write_branching_hello() {
    word offset = 0;
    word char_ptr = 0x100;
    word writer_offset = 0x200;

    // hello, world branching program
    // which duplicates characters using a second branch
    //
    //   branch writer char_ptr
    WRITE_I(BRANCH, 0b000, IMMED, IMMED);
    WRITE(writer_offset);
    WRITE(char_ptr);

    // first branch loop -- feed chars to second branch
    //   mov r0, &hello
    WRITE_U(COPY, R0, IMMED);
    word hello_offset = offset++;
    // loop:
    //   load r1 r0
    word loop_offset = WRITE_I(LOAD, 0, R1, R0);
    //   cmp r1 0
    WRITE_I(COMPARE, 0, R1, IMMED);
    WRITE(0);
    //   jeq done
    WRITE_I(JUMP, 0, IMMED, COND_EQ);
    word done_offset = offset++;
    //   store char_ptr r1
    WRITE_I(STORE, 0, IMMED, R1);
    WRITE(char_ptr);
    //   add r0 r0 1
    WRITE_I(ADD, R0, R0, IMMED);
    WRITE(1);
    //   jmp loop
    WRITE_I(JUMP, 0, IMMED, COND_ALWAYS);
    WRITE(loop_offset);
    // done:
    cpu_store(done_offset, offset, 0);
    //   halt
    WRITE(ITAG_HALT);

    cpu_store(hello_offset, offset, 0);
    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WRITE(*c);

    // second branch loop -- write chars twice
    offset = writer_offset;
    WRITE_I(LOAD, 0b110, R0, IMMED);
    WRITE(0);
    WRITE_I(PUTC, 0, 0, R0);
    WRITE_I(PUTC, 0, 0, R0);
    WRITE_I(JUMP, 0, IMMED, COND_ALWAYS);
    WRITE(writer_offset);
}

void write_hello() {
    word offset = 0;

    // hello, world program
    //
    //   mov r0, &hello
    WRITE_U(COPY, R0, IMMED);
    word hello_offset = offset++;
    // loop:
    word loop_offset = offset;
    //   load r1 r0
    WRITE_I(LOAD, 0b000, R1, R0);
    //   cmp r1 0
    WRITE_I(COMPARE, 0, R1, IMMED);
    WRITE(0);
    //   jeq done
    WRITE_I(JUMP, 0, IMMED, COND_EQ);
    word done_offset = offset++;
    //   putc r1
    WRITE_I(PUTC, 0, 0, R1);
    //   add r0 r0 1
    WRITE_I(ADD, 0, 0, IMMED);
    WRITE(1);
    //   jmp loop
    WRITE_I(JUMP, 0, IMMED, COND_ALWAYS);
    WRITE(loop_offset);
    // done:
    cpu_store(done_offset, offset, 0);
    //   halt
    WRITE(ITAG_HALT);

    cpu_store(hello_offset, offset, 0);

    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WRITE(*c);
}

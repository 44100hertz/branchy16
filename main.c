#include <stdio.h>

#include "cpu.h"
#ifdef TESTING
#include "tests.h"
#endif

#pragma GCC diagnostic ignored "-Wunused-function"
static void write_hello();
static void write_branching_hello();

// write instruction word
#define WRITE(word) (cpu_store(offset, word, false), offset++)
// write special or binary word
#define WRITE_I(instr, word) \
    (cpu_store(offset, instr << CPU_ITAG_OFFSET | word, false), offset++)
// write unary instruction word
#define WRITE_U(instr, word) \
    (cpu_store(offset, cpu_encode_unary(instr) | word, false), offset++)

int main(int argc, char **argv) {
#ifdef TESTING
    run_tests();
https:  // www.youtube.com/watch?v=GKnAWcWnJJc
    return 0;
#endif
    cpu_init();
    write_branching_hello();

    for (int i = 0; i < 100; ++i) {
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
    WRITE_I(ITAG_BRANCH, 0b000 << 8 | 8 << 4 | 8 << 0);
    WRITE(writer_offset);
    WRITE(char_ptr);

    // first branch loop -- feed chars to second branch
    //   mov r0, &hello
    WRITE_U(ITAG_COPY, 0 << 8 | 8 << 4);
    word hello_offset = offset++;
    // loop:
    word loop_offset = offset;
    //   load r1 r0
    WRITE_I(ITAG_LOAD, 1 << 4 | 0);
    //   cmp r1 0
    WRITE_I(ITAG_COMPARE, 1 << 4 | 8);
    WRITE(0);
    //   jeq done
    WRITE_I(ITAG_JUMP, 8 << 4 | 0b001);
    word done_offset = offset++;
    //   store char_ptr r1
    WRITE_I(ITAG_STORE, 8 << 4 | 1);
    WRITE(char_ptr);
    //   add r0 r0 1
    WRITE_I(ITAG_ADD, 0 << 8 | 0 << 4 | 8);
    WRITE(1);
    //   jmp loop
    WRITE_I(ITAG_JUMP, 0b000 << 8 | 8 << 4 | 0b111);
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
    WRITE_I(ITAG_LOAD, 0b110 << 8 | 0 << 4 | 8);
    WRITE(0);
    WRITE_I(ITAG_PUTC, 0);
    WRITE_I(ITAG_PUTC, 0);
    WRITE_I(ITAG_JUMP, 8 << 4 | 0b111);
    WRITE(writer_offset);
}

void write_hello() {
    word offset = 0;

    // hello, world program
    //
    //   mov r0, &hello
    WRITE_U(ITAG_COPY, 0 << 8 | 8 << 4);
    word hello_offset = offset++;
    // loop:
    word loop_offset = offset;
    //   load r1 r0
    WRITE_I(ITAG_LOAD, 1 << 4 | 0);
    //   cmp r1 0
    WRITE_I(ITAG_COMPARE, 1 << 4 | 8);
    WRITE(0);
    //   jeq done
    WRITE_I(ITAG_JUMP, 8 << 4 | 0b001);
    word done_offset = offset++;
    //   putc r1
    WRITE_I(ITAG_PUTC, 1);
    //   add r0 r0 1
    WRITE_I(ITAG_ADD, 0 << 8 | 0 << 4 | 8);
    WRITE(1);
    //   jmp loop
    WRITE_I(ITAG_JUMP, 8 << 4 | 0b111);
    WRITE(loop_offset);
    // done:
    cpu_store(done_offset, offset, 0);
    //   halt
    WRITE(ITAG_HALT);

    cpu_store(hello_offset, offset, 0);

    char *hello = "hello, world\n";
    for (char *c = hello; *c != 0; ++c) WRITE(*c);
}

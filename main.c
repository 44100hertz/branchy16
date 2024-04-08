#include <stdio.h>

#include "cpu.h"
#ifdef TESTING
#include "tests.h"
#endif

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
    return 0;
#endif
    cpu_init();

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
    WRITE_I(ITAG_JUMP, 0 << 8 | 8 << 4 | 0b001);
    word done_offset = offset++;
    //   putc r1
    WRITE_I(ITAG_PUTC, 1);
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

    for (int i = 0; i < 100; ++i) {
        bool running = cpu_step();
        if (!running) {
            printf("Executed %d cycles.", i);
            break;
        }
    }
}

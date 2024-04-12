#include "tests.h"

#include <stdio.h>
#include <string.h>

#include "binarygen.h"
#include "cpu.h"

extern int (*emit_char)(int);  // from cpu

static void test_unary_encode();
static void test_hello();
static void test_hello_branching();
static void cpu_run_bounded(int limit);

static char char_buf[1024];
static uintptr_t char_buf_pos;

static void clear_string() {
    memset(char_buf, 0, sizeof(char_buf));
    char_buf_pos = 0;
}
static int emit_override(int c) {
    char_buf[char_buf_pos++] = c;
    return c;
}
static void test_string(char *test) {
    if (strncmp(char_buf, test, sizeof(char_buf))) {
        printf("FAIL\nexpected\n%s\ngot\n%s\n", test, char_buf);
    } else {
        puts("PASS");
    }
}

int main(int _argc, char **_argv) {
    emit_char = emit_override;
    puts("-------- Test: Unary Encode");
    test_unary_encode();
    puts("-------- Test: Hello");
    test_hello();
    puts("-------- Test: Hello Branching");
    test_hello_branching();
    puts("Tests complete.");
    return 0;
}

void test_unary_encode() {
    bool fail = false;
    for (int i = 0; i < 32; ++i) {
        word encode = cpu_encode_unary(i);
        word decode = cpu_decode_unary(encode);
        if (decode != i) {
            fail = true;
            printf("\tfail: %x -> %x -> %x\n", i, encode, decode);
        }
    }
    for (int i = 0; i < 32; ++i) {
        word encode = cpu_encode_unary(i) | 0x7f << 4;
        word decode = cpu_decode_unary(encode);
        if (decode != i) {
            fail = true;
            printf("\tfail: %x -> %x -> %x\n", i, encode, decode);
        }
    }
    if (!fail) {
        puts("PASS");
    }
}

void test_hello() {
    cpu_init();
    write_hello();
    clear_string();
    cpu_run_bounded(1000);
    test_string("hello, world\n");
}

void test_hello_branching() {
    cpu_init();
    write_branching_hello();
    clear_string();
    cpu_run_bounded(1000);
    test_string("hheelllloo,,  wwoorrlldd\n\n");
}

void cpu_run_bounded(int limit) {
    for (int i = 0, running = true; i < limit && running; ++i) {
        running = cpu_step();
    }
}

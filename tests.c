#include "tests.h"

#include <stdio.h>

#include "cpu.h"

static void test_unary_encode();

void run_tests() {
    puts("Test: Unary Encode");
    test_unary_encode();
    puts("Tests complete.");
}

void test_unary_encode() {
    for (int i = 0; i < 32; ++i) {
        word encode = cpu_encode_unary(i);
        word decode = cpu_decode_unary(encode);
        if (decode != i) {
            printf("\tfail: %x -> %x -> %x\n", i, encode, decode);
        }
    }
    for (int i = 0; i < 32; ++i) {
        word encode = cpu_encode_unary(i) | 0x7f << 4;
        word decode = cpu_decode_unary(encode);
        if (decode != i) {
            printf("\tfail: %x -> %x -> %x\n", i, encode, decode);
        }
    }
}

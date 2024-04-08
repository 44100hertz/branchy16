#include <stdio.h>

#include "cpu.h"

int main(int argc, char **argv) {
    cpu_init();

    word offset = 0;
    cpu_store(offset++,
              ITAG_PUTC << CPU_ITAG_OFFSET | 0b000 << 8 | 0b0000 << 4 | 0b0000,
              false);
    cpu_store(offset++, 'h', false);
    cpu_store(offset++, ITAG_HALT, false);

    for (int i = 0; i < 100; ++i) {
        bool running = cpu_step();
        if (!running) {
            printf("Executed %d cycles.", i);
            break;
        }
    }
}

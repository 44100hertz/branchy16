#include <stdint.h>
#include <stdio.h>

int main(int _argc, char **_argv) {
  for (long i = 0; i < 1000000; ++i) {
    uint8_t x = getchar();
    for (int i = 0; i < 8; ++i) {
      putchar(x & 1 ? -50 : 50);
      x >>= 1;
    }
  }
}

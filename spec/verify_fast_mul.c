#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void run_test(void);

bool verbose = false;

void simulate_signed_add(int num, bool subtract, int bit_count) {
  unsigned signmag = num;
  if (num < 0) {
    signmag ^= ((1 << (bit_count - 1)) - 1);
    signmag += 1;
  }
  bool sign = (signmag >> (bit_count - 1) & 0x1) ^ subtract;
  int total = 0;
  for (int i = 0; i < bit_count - 1; ++i) {
    if (signmag >> i & 1) {
      if (sign)
        total -= (1 << i);
      else
        total += (1 << i);
    }
  }
  if (verbose) {
    printf("input: %04b, signmag %04b\n", num & 0xf, signmag & 0xf);
    printf("original number: %d\n", num);
    printf("calculated: %d\n", total);
  }
  int expected = num * (subtract ? -1 : 1);
  if (total != expected) {
    printf("Expected %d, Got %d", expected, total);
  }
}

int main() {
  verbose = false;
  // for (int i = -32; i < 31; ++i) {
  //   simulate_signed_add(i, false, 6);
  // }
  for (int i = 0; i < 50; ++i) {
    run_test();
  }
  // run_test();
  // run_test();
  return 0;
}

void run_test() {
  const int factor_count = 16;
  const uint16_t bitmask = rand();
  const int bit_count = 7;
  int8_t *const factors = malloc(factor_count);
  int16_t *const factors_rot = calloc(bit_count, sizeof(*factors_rot));

  rand();
  for (int x = 0; x < factor_count; ++x) {
    factors[x] = rand() % (1 << (bit_count - 1)) * ((rand() & 1) ? 1 : -1);
    uint8_t slice = factors[x];
    if ((int8_t)slice < 0) {
      // converts twos complement into sign-mag representation
      slice ^= (1 << (bit_count - 1)) - 1;
      ++slice;
      // if (verbose) {
      //   printf("%d converted to %d\n", factors[x], slice & 0x3f);
      // }
    }
    for (int b = 0; b < bit_count; ++b) {
      if (slice & (1 << b)) {
        factors_rot[b] |= (1 << x);
      }
    }
  }

  {
    puts("conventional sum.");
    int sum = 0;
    for (int i = 0; i < factor_count; ++i) {
      int sign = bitmask & (1 << i) ? 1 : -1;
      sum += factors[i] * sign;
    }
    printf("result: %d\n", sum);
  }

  {
    puts("sideways sum.");
    int sum = 0;
    // invert input based on sign of slice table
    uint16_t bitmask_xor = bitmask ^ factors_rot[bit_count - 1];
    // sum each bit in parallel
    for (int i = 0; i < bit_count - 1; ++i) {
      int step = __builtin_popcount(factors_rot[i] & bitmask_xor & 0xffff) -
                 __builtin_popcount(factors_rot[i] & (~bitmask_xor) & 0xffff);
      sum += step << i;
    }
    printf("result: %d\n", sum);
  }

  free(factors);
  free(factors_rot);
}

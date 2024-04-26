#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef __int128_t FirSlice;

#define FIR_WIDTH (sizeof(FirSlice) * 8)
#define NUM_FIRS 64
#define FIR_BITS 10
#define MAX_VALUE ((1 << (FIR_BITS - 1)) - 1)
#define MIN_FIR_CYCLES 1
#define MAX_FIR_CYCLES 8

FirSlice fir_table[FIR_BITS * NUM_FIRS] = {0};
int16_t bit_counts[FIR_BITS * NUM_FIRS] = {0};

typedef struct {
  FirSlice slice;
  long pos;
} FirBuffer;

void step_fir(FirBuffer *fir);
int16_t apply_fir(FirSlice samples, int index);
void gen_firs(void);
static double sinc_norm(double t);
static double blackman(double t);
static int16_t popcount(FirSlice slice) {
  return __builtin_popcountl(slice) + __builtin_popcountl(slice >> 64);
}

int main(int argc, char **argv) {
  long len = 0;
  if (argc > 1) {
    len = atol(argv[1]);
  }
  gen_firs();
  FirBuffer fir = {
      .slice = 0,
      .pos = 0,
  };
  for (int i = 0; !len || i < len; ++i) {
    step_fir(&fir);
  }
}

void step_fir(FirBuffer *fir) {
  uint8_t byte = getchar();
  uint8_t step = 2;
  FirSlice mask = (1 << step) - 1;
  for (int i = 0; i < 8 / step; ++i) {
    fir->slice <<= step;
    fir->slice |= byte & mask;
    byte >>= step;
    fir->pos += 1;
    int16_t samp_out = apply_fir(fir->slice, 63);
    putchar(samp_out & 0xff);
    putchar(samp_out >> 8);
  }
}

int16_t apply_fir(FirSlice samples, int index) {
  // invert input samples based on sign bits
  FirSlice samples_inv = samples ^ fir_table[index * FIR_BITS + FIR_BITS - 1];

  int16_t out = 0;
  for (int i = 0; i < FIR_BITS - 1; ++i) {
    FirSlice mask = fir_table[index * FIR_BITS + i];
    int16_t bit_count = bit_counts[index * FIR_BITS + i];
    int16_t total = 2 * popcount(samples_inv & mask) - bit_count;
    out += total << i;
  }
  return out;
}

void gen_firs() {
  for (int i_row = 0; i_row < NUM_FIRS; ++i_row) {
    double width = MIN_FIR_CYCLES + (MAX_FIR_CYCLES - MIN_FIR_CYCLES) *
                                        ((double)i_row / NUM_FIRS);

    for (int x = 0; x < FIR_WIDTH; ++x) {
      // calculate windowed FIR sample
      double t_norm = (double)x / FIR_WIDTH;
      double t = (t_norm * width) - width / 2;
      double sample = sinc_norm(t) * blackman(t_norm);
      int16_t isamp = sample * MAX_VALUE;
      uint16_t slice = isamp;
      if (isamp < 0) {
        // convert twos complement into sign-mag representation
        slice ^= (1 << (FIR_BITS - 1)) - 1;
        ++slice;
      }
      for (int b = 0; b < FIR_BITS; ++b) {
        if (slice & (1 << b)) {
          fir_table[i_row * FIR_BITS + b] |= ((FirSlice)1 << x);
        }
      }
    }
    for (int b = 0; b < FIR_BITS; ++b) {
      bit_counts[i_row * FIR_BITS + b] =
          popcount(fir_table[i_row * FIR_BITS + b]);
    }
  }
}

double sinc_norm(double t) {
  if (t == 0)
    return 1;
  return sin(t * M_PI) / t / M_PI;
}

double blackman(double t) {
  if (t < 0 || t > 1)
    return 0;
  double a0 = 0.35875;
  double a1 = 0.48829;
  double a2 = 0.14128;
  double a3 = 0.01168;
  return a0 - a1 * cos(2 * M_PI * t) + a2 * cos(4 * M_PI * t) -
         a3 * cos(6 * M_PI * t);
}

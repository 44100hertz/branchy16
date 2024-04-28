#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// low bits are in slice[0], lowest bits of that are first bits in.
typedef __uint128_t FirSlice[4];

#define FIR_WIDTH (sizeof(FirSlice) * 8)
#define NUM_FIRS 64
#define FIR_BITS 10
#define ATTENUATION_SHIFT 0
#define MAX_VALUE ((1 << (FIR_BITS - 1)) - 1)
#define MIN_FIR_CYCLES 4
#define MAX_FIR_CYCLES 32

static FirSlice fir_table[FIR_BITS * NUM_FIRS] = {0};
static int16_t bit_counts[FIR_BITS * NUM_FIRS] = {0};

typedef struct {
  FirSlice slice;
  int32_t timer;
  uint16_t pitch;
  uint8_t in_byte;
  uint8_t byte_pos;
} FirBuffer;

void step_fir(FirBuffer *fir);
int16_t apply_fir(FirSlice samples, int index);
void gen_firs(void);
static double sinc_norm(double t);
static double blackman(double t);
static int16_t popcount(__uint128_t slice) {
  return __builtin_popcountl(slice) + __builtin_popcountl(slice >> 64);
}

int main(int argc, char **argv) {
  long len = 0;
  if (argc > 1) {
    len = atol(argv[1]);
  }
  gen_firs();
  FirBuffer fir = {
      .slice = {0},
      .in_byte = 0,
      .byte_pos = 0,
      .timer = 0,
      .pitch = (0 << 12) | 4095,
  };
  for (int i = 0; !len || i < len; ++i) {
    step_fir(&fir);
    if (i % 100 == 0) {
      fir.pitch--;
      if ((fir.pitch & 0xfff) < 2048) {
        fir.pitch += (1 << 12);
        fir.pitch &= 0xf000;
        fir.pitch |= 4095;
      }
    }
  }
}

void step_fir(FirBuffer *fir) {
  int16_t octave = 1 << (fir->pitch >> 12);
  int16_t pitch = fir->pitch & 0x0fff;
  if (pitch < octave)
    pitch = octave;

  // calculate next 32 bits
  uint32_t next_bits = fir->slice[0];
  for (uintptr_t b = 0; b < 32; ++b) {
    // fetch byte if needed
    if (fir->byte_pos == 0) {
      fir->in_byte = getchar();
      fir->byte_pos = 8;
    }

    // subtract octave
    fir->timer -= octave;
    if (fir->timer < 0) {
      // if negative, read bit in from input
      // and increment by timer
      fir->timer += pitch;
      // rotate out and in...
      next_bits = (next_bits << 1) | (fir->in_byte & 1);
      fir->byte_pos--;
      fir->in_byte >>= 1;
    } else {
      // otherwise duplicate last bit
      next_bits = (next_bits << 1) | (next_bits & 1);
    }
  }

  // rotate in two 16-bit slices and write them
  const int8_t stepsize = 16;
  const uintptr_t steps = 32 / stepsize;
  for (uintptr_t s = 0; s < steps; ++s) {
    // rotate 16 bits into slice buffer
    for (intptr_t i = 3; i >= 0; --i) {
      fir->slice[i] <<= stepsize;
      if (i == 0) {
        // rotate 16 bits into first slice
        fir->slice[i] |= next_bits & ((1 << stepsize) - 1);
        next_bits >>= stepsize;
      } else {
        // rotate bits from lower buffer into higher ones
        fir->slice[i] |= (fir->slice[i - 1] >> (128 - stepsize));
      }
    }
    int16_t samp_out = apply_fir(fir->slice, 32);
    putchar(samp_out & 0xff);
    putchar(samp_out >> 8);
  }
}

int16_t apply_fir(FirSlice samples, int index) {
  // invert input samples based on sign bits
  FirSlice samples_inv;
  for (uintptr_t i = 0; i < 4; ++i) {
    samples_inv[i] = samples[i] ^ fir_table[index * FIR_BITS + FIR_BITS - 1][i];
    //   printf("samples: %064lb%064lb\n", (long)(samples[i] >> 64),
    //          (long)samples[i]);
    //   printf("mask:    %064lb%064lb\n", (long)(samples_inv[i] >> 64),
    //          (long)samples_inv[i]);
  }

  int32_t out = 0;
  for (int i = 0; i < FIR_BITS - 1; ++i) {
    int16_t total = -bit_counts[index * FIR_BITS + i];
    for (uintptr_t b = 0; b < 4; ++b) {
      __uint128_t mask = fir_table[index * FIR_BITS + i][b];
      total += 2 * popcount(samples_inv[b] & mask);
    }
    out += total << i;
  }
  return out >> ATTENUATION_SHIFT;
}

void gen_firs() {
  for (uintptr_t i_row = 0; i_row < NUM_FIRS; ++i_row) {
    double width = MIN_FIR_CYCLES + (MAX_FIR_CYCLES - MIN_FIR_CYCLES) *
                                        ((double)i_row / NUM_FIRS);

    for (uintptr_t x = 0; x < FIR_WIDTH; ++x) {
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
      for (uintptr_t b = 0; b < FIR_BITS; ++b) {
        if (slice & (1 << b)) {
          fir_table[i_row * FIR_BITS + b][x / 128] |=
              ((__uint128_t)1 << (x % 128));
        }
      }
    }
    for (uintptr_t b = 0; b < FIR_BITS; ++b) {
      for (uintptr_t bb = 0; bb < 4; ++bb) {
        bit_counts[i_row * FIR_BITS + b] +=
            popcount(fir_table[i_row * FIR_BITS + b][bb]);
      }
    }
  }
}

double sinc_norm(double t) {
  if (t == 0.0)
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

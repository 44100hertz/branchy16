#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef uint32_t FirSlice;

#define FIR_WIDTH 32
#define NUM_FIRS 64
// max value is roughly between 1 << 11 and 1 << 12,
// because in theory 32 samples can match the kernel exactly
// and result in overflow with anything more than 11 bits,
// but practically speaking you get *almost* twice that.
#define MAX_VALUE 3200
#define MIN_FIR_CYCLES 4
#define MAX_FIR_CYCLES 8

int16_t fir_table[FIR_WIDTH * NUM_FIRS] = {0};

typedef struct {
  FirSlice slice;
  long pos;
} FirBuffer;

void step_fir(FirBuffer *fir);
int16_t apply_fir(FirSlice samples, int index);
void gen_firs();
static double sinc_norm(double t);
static double blackman(double t);

bool print = false;
int main(int _argc, char **_argv) {
  gen_firs();
  FirBuffer fir = {
      .slice = 0,
      .pos = 0,
  };
  while (true) {
    step_fir(&fir);
  }
}

void step_fir(FirBuffer *fir) {
  uint8_t byte = getchar();
  uint8_t step = 1;
  FirSlice mask = (1 << step) - 1;
  for (int i = 0; i < 8 / step; ++i) {
    fir->slice = (fir->slice << step) | (byte & mask);
    byte >>= step;
    fir->pos += 1;
    int16_t samp_out = apply_fir(fir->slice, 0);
    putchar(samp_out & 0xff);
    putchar(samp_out >> 8);
  }
}

int16_t apply_fir(FirSlice samples, int index) {
  int16_t out = 0;
  for (int i = 0; i < FIR_WIDTH; ++i) {
    int16_t weight = fir_table[index * FIR_WIDTH + i];
    if (samples & 0x1) {
      out += weight;
    } else {
      out -= weight;
    }
    samples >>= 1;
  }
  return out;
}

void gen_firs() {
  for (int i = 0; i < NUM_FIRS; ++i) {
    double width = MIN_FIR_CYCLES +
                   (MAX_FIR_CYCLES - MIN_FIR_CYCLES) * ((double)i / NUM_FIRS);

    for (int x = 0; x < FIR_WIDTH; ++x) {
      // calculate windowed FIR sample
      double t_norm = (double)x / FIR_WIDTH;
      double t = (t_norm * width) - width / 2;
      double sample = sinc_norm(t) * blackman(t_norm);
      int16_t isamp = MAX_VALUE * sample;
      fir_table[i * FIR_WIDTH + x] = isamp;
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
  return 0.42 - 0.5 * cos(2 * M_PI * t) + 0.08 * cos(4 * M_PI * t);
}

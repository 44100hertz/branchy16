#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint8_t pos;
  uint8_t out;
} ByteWriter;

static ByteWriter bw = {0};

void push_bit(ByteWriter *bw, bool set) {
  bw->out = (bw->out >> 1) | (set ? 0x80 : 0);
  ++bw->pos;
  if (bw->pos == 8) {
    putchar(bw->out);
    bw->out = 0;
    bw->pos = 0;
  }
}

double lerp(double a, double b, double t) { return a + (b - a) * t; }

void encode_delta_sigma_2(double amp, double drive, double noise_level_db) {
  const double noise_level = pow(10, noise_level_db / 10);

  // attenuation of error feedback and step attenuation of sample
  const double atten = (1.0 - drive);

  double qe_0 = 0;
  double qe_1 = 0;

  while (1) {
    uint8_t b_low = getchar();
    uint8_t b_high = getchar();
    double sample = (int16_t)(b_low | (b_high << 8)) / (double)INT16_MAX;
    sample *= amp;
    sample += ((rand() / (float)RAND_MAX) - 0.5) * noise_level; // dithering

    qe_0 = qe_0 * atten + sample;
    qe_1 = qe_1 * atten + qe_0;
    bool high = qe_1 > 0.0;
    push_bit(&bw, high);

    qe_0 -= (high ? 1 : -1) * atten;
    qe_1 -= (high ? 1 : -1) * atten;
  }
}

int main(int argc, char **argv) { encode_delta_sigma_2(1.0, 0.0, -24); }

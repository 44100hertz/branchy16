#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  uint8_t pos;
  uint8_t out;
} ByteWriter;

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

void encode_delta_sigma_order(double drive, double amp) {
  ByteWriter bw = {0};

  double qe_0 = 0;
  double qe_1 = 0;

  while (1) {
    uint8_t b_low = getchar();
    uint8_t b_high = getchar();
    double sample = (int16_t)(b_low | (b_high << 8)) / (double)INT16_MAX;

    qe_0 += sample * amp * (1.0 + drive);
    qe_1 = (qe_0 + qe_1) * (1.0 - drive) + drive * sample * amp;
    bool high = qe_1 > 0.0;
    push_bit(&bw, high);

    qe_0 -= (high ? 1 : -1);
    qe_1 -= (high ? 1 : -1);
  }
}

int main(int argc, char **argv) { encode_delta_sigma_order(0.5, 1.0); }

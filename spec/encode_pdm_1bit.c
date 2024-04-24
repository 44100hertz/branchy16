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

void encode_delta_sigma_order_1(double err_correction, double amp) {
  ByteWriter bw = {0};
  double qe = 0;
  while (1) {
    uint8_t low = getchar();
    uint8_t high = getchar();
    short sample = low | (high << 8);

    qe += sample / (double)INT16_MAX * amp;
    push_bit(&bw, qe > 0);
    qe -= (qe > 0 ? 1.0 / 2 : -1.0 / 2) * err_correction;
  }
}

void encode_delta_sigma_order_2(double amp) {
  ByteWriter bw = {0};

  double qe_0 = 0;
  double qe_1 = 0;

  while (1) {
    uint8_t b_low = getchar();
    uint8_t b_high = getchar();
    double sample = (int16_t)(b_low | (b_high << 8)) / (double)INT16_MAX;

    qe_0 += sample * amp;
    qe_1 += qe_0;
    bool high = qe_1 > 0.0;
    push_bit(&bw, high);

    qe_0 -= (high ? 1 : -1);
    qe_1 -= (high ? 1 : -1);
  }
}

void encode_delta_sigma(bool order_2, double err_correction, double amp) {
  if (order_2) {
    encode_delta_sigma_order_2(amp);
  } else {
    encode_delta_sigma_order_1(err_correction, amp);
  }
}

int main(int argc, char **argv) { encode_delta_sigma(true, 1.0, 1.0); }

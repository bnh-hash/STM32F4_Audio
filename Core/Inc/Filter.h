#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>
#include <math.h>

#define BUFFER_SIZE      (3840*4)
#define SAMPLING_FREQ    48000


// 1. önce yapıyı tanımla (Derleyici önce bunun ne olduğunu öğrenmeli)
typedef struct {
    uint32_t raw_adc[2];     // [0]: Volume, [1]: Ring Mod
    float volume;
    float ring_mod_freq;
} potans_t;

// 2. bu yapıyı kullanan değişkenleri yaz
extern potans_t potans_val;

// DSP ve Potansiyometre Fonksiyon Prototipleri
void potans_update(potans_t *p);
void filter_apply_volume(int16_t *input_buf, uint32_t length, float volume);
void filter_apply_ring(int16_t *input_buf, uint32_t length, float *carrier_phase, float carrier_freq, uint32_t sampling_freq);

#endif /* FILTER_H */


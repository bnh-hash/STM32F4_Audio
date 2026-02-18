/*
 * Filter.c
 *
 *  Created on: Feb 17, 2026
 *      Author: Gökçe
 */

#include "filter.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- Volume (Ses Seviyesi) Filtresi ---
// BU DOSYA DONANIMDAN HABERSİZDİR. SADECE MATEMATİK YAPAR.
void filter_apply_volume(int16_t *input_buf, int16_t *output_buf, uint32_t length, float volume) {
    for (uint32_t i = 0; i < length; i++) {
        // Giriş sinyalini (input_buf) parametre olarak gelen volume ile çarpıp
        // Çıkış sinyaline (output_buf) yazar.
        output_buf[i] = (int16_t)((float)input_buf[i] * volume);
    }
}

// --- Ring Modulator Filtresi (İleride kullanmak istersen) ---
void filter_apply_ring(int16_t *input_buf, int16_t *output_buf, uint32_t length, float *carrier_phase, float carrier_freq, uint32_t sampling_freq) {
    float phase_inc = (2.0f * M_PI * carrier_freq) / (float)sampling_freq;

    for (uint32_t i = 0; i < length; i += 2) {
        float modulation = sinf(*carrier_phase);

        output_buf[i]     = (int16_t)((float)input_buf[i] * modulation);
        output_buf[i + 1] = (int16_t)((float)input_buf[i+1] * modulation);

        *carrier_phase += phase_inc;
        if (*carrier_phase >= 2.0f * M_PI) *carrier_phase -= 2.0f * M_PI;
    }
}

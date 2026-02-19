/*
 * filter.c
 * Author: Gökçe
 */

#include "filter.h"
#include <math.h>
#include <string.h> // memcpy için gerekli

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Maksimum geçici tampon boyutu (app_main.c'deki BUFFER_SIZE'a eşit veya ondan büyük olmalı)
#define MAX_TEMP_BUF_SIZE 2048

/* * GLOBAL GEÇİCİ TAMPON
 * 'static' anahtar kelimesi bu diziyi sadece filter.c dosyasından erişilebilir yapar (Güvenlik).
 * Fonksiyonun dışında olduğu için Stack'te değil, doğrudan RAM'de (BSS) kalıcı yer tutar.
 */
static int16_t temp_buffer[MAX_TEMP_BUF_SIZE];


// --- Volume (Ses Seviyesi) Filtresi ---
void filter_apply_volume(int16_t *input_buf, uint32_t length, float volume) {
    for (uint32_t i = 0; i < length; i++) {
    	input_buf[i] = (int16_t)((float)input_buf[i] * volume);
    }
}


// --- Ring Modulator Filtresi ---
void filter_apply_ring(int16_t *input_buf, uint32_t length, float *carrier_phase, float carrier_freq, uint32_t sampling_freq) {
    float phase_inc = (2.0f * M_PI * carrier_freq) / (float)sampling_freq;

    // Güvenlik: Gelen veri uzunluğu maksimum tamponumuzdan büyükse sistemi koru
    if (length > MAX_TEMP_BUF_SIZE) {
        length = MAX_TEMP_BUF_SIZE;
    }

    for (uint32_t i = 0; i < length; i += 2) {
        float modulation = sinf(*carrier_phase);

        // 1. İşlemi global temp_buffer üzerinde güvenle yapıyoruz
        temp_buffer[i]     = (int16_t)((float)input_buf[i] * modulation);
        temp_buffer[i + 1] = (int16_t)((float)input_buf[i+1] * modulation);

        *carrier_phase += phase_inc;
        if (*carrier_phase >= 2.0f * M_PI) *carrier_phase -= 2.0f * M_PI;
    }

    // 2. İşlem bitince temp_buffer'daki veriyi asıl diziye (input_buf) geri yapıştırıyoruz
    memcpy(input_buf, temp_buffer, length * sizeof(int16_t));
}

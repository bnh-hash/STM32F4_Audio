/*
 * filter.c
 * Author: Gökçe
 */

#include "filter.h"
#include <math.h>
#include <string.h> // memcpy için gerekli

/* --- Makrolar ve Sabitler --- */
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Maksimum geçici tampon boyutu (Yarım transfer için 960 int16_t yeterli, 2048 güvenli bir sınır)
#define MAX_TEMP_BUF_SIZE 2048

/* --- Global ve Statik Değişkenler --- */

/* * GLOBAL GEÇİCİ TAMPON
 * 'static' anahtar kelimesi bu diziyi sadece filter.c dosyasından erişilebilir yapar (Güvenlik).
 * Fonksiyonun dışında olduğu için Stack'te değil, doğrudan RAM'de (BSS) kalıcı yer tutar.
 */
static int16_t temp_buffer[MAX_TEMP_BUF_SIZE];

// Struct değişkenini burada yaratıyoruz (filter.h içinde extern ile duyuruldu)
potans_t potans_val = {0};

/* --- Fonksiyonlar --- */

// --- Potansiyometre Güncelleme ---
void potans_update(potans_t *p) {
    // 1. VOLUME HESABI (0-4095 -> 0.0-1.0)
    float target_vol = (float)p->raw_adc[0] / 4095.0f;
    // Yumuşatma: %90 eski değer, %10 yeni değer (Hışırtı önleyici)
    p->volume = (p->volume * 0.90f) + (target_vol * 0.10f);

    // 2. RING MOD FREKANS HESABI (0-4095 -> 10Hz-2000Hz)
    float target_freq = 10.0f + ((float)p->raw_adc[1] / 4095.0f) * 1990.0f;
    p->ring_mod_freq = (p->ring_mod_freq * 0.90f) + (target_freq * 0.10f);
}

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

    // i+=2 yapıyoruz çünkü ses STEREO (Sol ve Sağ kanal ardışık gelir)
    for (uint32_t i = 0; i < length; i += 2) {
        float modulation = sinf(*carrier_phase);

        // 1. İşlemi global temp_buffer üzerinde güvenle yapıyoruz
        // Sol ve sağ kanala aynı modülasyonu uyguluyoruz
        temp_buffer[i]     = (int16_t)((float)input_buf[i] * modulation);
        temp_buffer[i + 1] = (int16_t)((float)input_buf[i+1] * modulation);

        // Fazı artır ve 2*PI'yi aşarsa sıfırla (Taşıyıcı sinyal dalgası)
        *carrier_phase += phase_inc;
        if (*carrier_phase >= 2.0f * M_PI) {
            *carrier_phase -= 2.0f * M_PI;
        }
    }

    // 2. İşlem bitince temp_buffer'daki veriyi asıl diziye (input_buf) geri yapıştırıyoruz
    memcpy(input_buf, temp_buffer, length * sizeof(int16_t));
}

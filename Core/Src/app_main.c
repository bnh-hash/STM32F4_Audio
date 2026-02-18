/*
 * app_main.c
 *
 * Created on: Feb 13, 2026
 * Author: Gökçe
 */

#include "app_main.h"
#include "stm32f4_discovery_audio.h"
#include "filter.h" // Modüler filtre kütüphaneni dahil et
#include <math.h>

/* --- Tanımlamalar --- */
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define RING_BUFFER_SIZE (2048)
#define SAMPLING_FREQ    48000

#define NOTE_B3   246.94f
#define NOTE_CS4  277.18f
#define NOTE_D4   293.66f
#define NOTE_E4   329.63f

/* --- Dış Değişkenler --- */
extern ADC_HandleTypeDef hadc1;

/* --- Değişkenler --- */
int16_t Audio_Ring_Buffer[RING_BUFFER_SIZE];  // Ham ses
int16_t Filter_Ring_Buffer[RING_BUFFER_SIZE]; // İşlenmiş ses

volatile float current_phase = 0.0f;
volatile float target_frequency = 0.0f;

// Bu değişken Main ve Callback arasında köprüdür.
// Volatile olması şarttır, çünkü kesme (interrupt) içinde okunur.
volatile float global_volume = 0.5f;

uint32_t t_short = 200;

/* --- Yardımcı Fonksiyon: Akıllı Bekleme ve ADC Okuma --- */
/*
 * Bu fonksiyon HAL_Delay yerine geçer.
 * Bekleme süresi boyunca boş durmaz, sürekli ADC (Pot) okur.
 * Böylece melodi çalarken volüm takılmadan değişir.
 */
void Smart_Delay_With_ADC(uint32_t delay_ms) {
    uint32_t start_tick = HAL_GetTick();

    while ((HAL_GetTick() - start_tick) < delay_ms) {
        // 1. ADC Başlat
        HAL_ADC_Start(&hadc1);

        // 2. Çevrimi Bekle (Kısa timeout)
        if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK) {
            uint32_t raw_val = HAL_ADC_GetValue(&hadc1);

            // 3. Değeri Normalize Et (0.0 - 1.0)
            float instant_vol = (float)raw_val / 4095.0f;

            // 4. Yumuşatma (Smoothing) - Main içinde yapıldığı için sorun yok
            global_volume = (global_volume * 0.9f) + (instant_vol * 0.1f);
        }
        HAL_ADC_Stop(&hadc1);

        // Çok sık okumamak için minik bir bekleme (opsiyonel)
        HAL_Delay(1);
    }
}

/* --- DSP Fonksiyonu 1: Ham Ses Üretimi --- */
void Generate_Sine_Segment(int16_t *buffer, uint32_t start_index, uint32_t length, float freq) {
    float phase_increment = (2.0f * M_PI * freq) / (float)SAMPLING_FREQ;

    for (uint32_t i = start_index; i < start_index + length; i += 2) {
        current_phase += phase_increment;
        if (current_phase >= 2.0f * M_PI) current_phase -= 2.0f * M_PI;

        // Ses maksimum genlikte (20000) üretilir. Volüm burada yok!
        int16_t val = (freq == 0) ? 0 : (int16_t)(20000.0f * sinf(current_phase));

        buffer[i]     = val;
        buffer[i + 1] = val;
    }
}

/* --- DMA Callbacks (Çok Hızlı Çalışmalı) --- */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
    // ADC okuma YOK. Sadece global değişkeni kullan.
    // filter.c dosyasındaki fonksiyonu çağırıyoruz
    filter_apply_volume(Audio_Ring_Buffer, Filter_Ring_Buffer, RING_BUFFER_SIZE / 2, global_volume);

    Generate_Sine_Segment(Audio_Ring_Buffer, 0, RING_BUFFER_SIZE / 2, target_frequency);
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
    // Pointer aritmetiği ile tamponun ikinci yarısını gönderiyoruz
    filter_apply_volume(&Audio_Ring_Buffer[RING_BUFFER_SIZE / 2],
                        &Filter_Ring_Buffer[RING_BUFFER_SIZE / 2],
                        RING_BUFFER_SIZE / 2,
                        global_volume);

    Generate_Sine_Segment(Audio_Ring_Buffer, RING_BUFFER_SIZE / 2, RING_BUFFER_SIZE / 2, target_frequency);
}

/* --- Uygulama Başlatma --- */
void App_Init(void) {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(50);

    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 150, SAMPLING_FREQ) != AUDIO_OK) {
        Error_Handler();
    }

    Generate_Sine_Segment(Audio_Ring_Buffer, 0, RING_BUFFER_SIZE, 0);
    BSP_AUDIO_OUT_Play((uint16_t*)Filter_Ring_Buffer, RING_BUFFER_SIZE * sizeof(int16_t));
}

/* --- Ana Melodi Döngüsü --- */
void App_Loop(void) {
    // --- İLK 3 TEKRAR ---
    for(int i=0; i<2; i++) {
        float verse[] = {NOTE_CS4, NOTE_CS4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_E4};
        for(int j=0; j<7; j++) {
            target_frequency = verse[j];
            // HAL_Delay yerine Smart_Delay kullanıyoruz ki pot çalışsın
            Smart_Delay_With_ADC(t_short);
        }
    }

    // --- SON SATIR ---
    float chorus_end[] = {NOTE_CS4, NOTE_CS4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_CS4};
    for(int j=0; j<7; j++) {
        target_frequency = chorus_end[j];
        Smart_Delay_With_ADC(t_short);
    }

    target_frequency = NOTE_B3;
    Smart_Delay_With_ADC(t_short * 2);

    target_frequency = 0;
    Smart_Delay_With_ADC(1000);
}

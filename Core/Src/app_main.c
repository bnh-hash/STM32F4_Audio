/*
 * app_main.c
 *
 * Created on: Feb 13, 2026
 * Author: Gökçe
 */

#include "app_main.h"
#include "stm32f4_discovery_audio.h"
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
extern ADC_HandleTypeDef hadc1; // main.c'deki handle

/* --- Değişkenler --- */
int16_t Audio_Ring_Buffer[RING_BUFFER_SIZE];
int16_t Filter_Ring_Buffer[RING_BUFFER_SIZE];
volatile float current_phase = 0.0f;
volatile float target_frequency = 0.0f;
volatile float volume_factor = 0.0f; // Mevcut ses seviyesi

uint32_t t_short = 200;

/* --- DSP Fonksiyonu: Ses Üretimi ve Temiz Volume Kontrolü --- */
void Generate_Sine_Segment(int16_t *buffer, uint32_t start_index, uint32_t length, float freq) {

    // 1. ADC'den Pot Değerini Oku
    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK) {
        float raw_adc = (float)HAL_ADC_GetValue(&hadc1) / 4095.0f;

        // 2. Zipper Noise Engelleyici (Smoothing Filter)
        // Yeni değeri yavaşça uygula ki seste ani sıçramalar/cızırtılar olmasın
        volume_factor = (volume_factor * 0.95f) + (raw_adc * 0.05f);
    }
    HAL_ADC_Stop(&hadc1);

    float phase_increment = (2.0f * M_PI * freq) / (float)SAMPLING_FREQ;

    for (uint32_t i = start_index; i < start_index + length; i += 2) {
        current_phase += phase_increment;
        if (current_phase >= 2.0f * M_PI) current_phase -= 2.0f * M_PI;

        // 3. Genlik Sınırlandırma (Clipping'i önlemek için 20000 civarı güvenlidir)
        int16_t val = (freq == 0) ? 0 : (int16_t)(20000.0f * volume_factor * sinf(current_phase));

        buffer[i]     = val; // Sol kanal
        buffer[i + 1] = val; // Sağ kanal
    }
}

/* --- DMA Callbacks --- */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
    for (uint32_t i = 0; i < RING_BUFFER_SIZE/2 ; i++) {
        Filter_Ring_Buffer[i] = Audio_Ring_Buffer[i];
    }
    Generate_Sine_Segment(Audio_Ring_Buffer, 0, RING_BUFFER_SIZE / 2, target_frequency);
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
    for (uint32_t i = RING_BUFFER_SIZE/2; i < RING_BUFFER_SIZE ; i++) {
        Filter_Ring_Buffer[i] = Audio_Ring_Buffer[i];
    }
    Generate_Sine_Segment(Audio_Ring_Buffer, RING_BUFFER_SIZE / 2, RING_BUFFER_SIZE / 2, target_frequency);
}

/* --- Uygulama Başlatma --- */
void App_Init(void) {
    // DAC Donanım Reset
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(50);

    // Audio Donanımını Başlat
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 85, SAMPLING_FREQ) != AUDIO_OK) {
        Error_Handler();
    }

    // İlk tamponu boşalt ve başlat
    Generate_Sine_Segment(Audio_Ring_Buffer, 0, RING_BUFFER_SIZE, 0);
    BSP_AUDIO_OUT_Play((uint16_t*)Filter_Ring_Buffer, RING_BUFFER_SIZE * sizeof(int16_t));
}

/* --- Ana Melodi Döngüsü --- */
void App_Loop(void) {
    for(int i=0; i<2; i++) {
        float verse[] = {NOTE_CS4, NOTE_CS4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_E4, NOTE_E4};
        for(int j=0; j<7; j++) {
            target_frequency = verse[j]; HAL_Delay(t_short);
        }
    }

    float chorus_end[] = {NOTE_CS4, NOTE_CS4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_CS4};
    for(int j=0; j<7; j++) {
        target_frequency = chorus_end[j]; HAL_Delay(t_short);
    }
    target_frequency = NOTE_B3;  HAL_Delay(t_short * 2);
    target_frequency = 0;        HAL_Delay(1000);
}

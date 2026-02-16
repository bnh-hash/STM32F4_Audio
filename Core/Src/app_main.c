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

#define RING_BUFFER_SIZE (8192)
#define SAMPLING_FREQ    48000

#define NOTE_B3   246.94f
#define NOTE_CS4  277.18f
#define NOTE_D4   293.66f
#define NOTE_E4   329.63f

/* --- Dış Değişkenler --- */
extern ADC_HandleTypeDef hadc1;

/* --- Değişkenler --- */
int16_t Audio_Ring_Buffer[RING_BUFFER_SIZE];  // Ham ses burada üretilir
int16_t Filter_Ring_Buffer[RING_BUFFER_SIZE]; // İşlenmiş ses burada saklanır
volatile float current_phase = 0.0f;
volatile float target_frequency = 0.0f;
volatile float volume_factor = 0.0f;

uint32_t t_short = 200;

/* --- DSP Fonksiyonu 1: Ham Ses Üretimi (Ses Sabit) --- */
void Generate_Sine_Segment(int16_t *buffer, uint32_t start_index, uint32_t length, float freq) {
    float phase_increment = (2.0f * M_PI * freq) / (float)SAMPLING_FREQ;

    for (uint32_t i = start_index; i < start_index + length; i += 2) {
        current_phase += phase_increment;
        if (current_phase >= 2.0f * M_PI) current_phase -= 2.0f * M_PI;

        // Ses hep maksimum seviyede üretiliyor (20000 sabit)
        int16_t val = (freq == 0) ? 0 : (int16_t)(20000.0f * sinf(current_phase));

        buffer[i]     = val;
        buffer[i + 1] = val;
    }
}

/* --- DSP Fonksiyonu 2: Filtre/Volume İşlemi (Pot Burada Devrede) --- */
void Apply_Volume_Filter(int16_t *input_buf, int16_t *output_buf, uint32_t start_index, uint32_t length) {

    // ADC Okuma ve Yumuşatma
    HAL_ADC_Start(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK) {
        float raw_adc = (float)HAL_ADC_GetValue(&hadc1) / 4095.0f;
        volume_factor = (volume_factor * 0.9f) + (raw_adc * 0.1f);
    }
    HAL_ADC_Stop(&hadc1);

    // Ham sesi al, pot çarpanıyla işle ve çıkış tamponuna yaz
    for (uint32_t i = start_index; i < start_index + length; i++) {
        output_buf[i] = (int16_t)((float)input_buf[i] * volume_factor);
    }
}

/* --- DMA Callbacks --- */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
    // 1. Önce üretilmiş olan ham sesi filtrele (Volümü uygula)
    Apply_Volume_Filter(Audio_Ring_Buffer, Filter_Ring_Buffer, 0, RING_BUFFER_SIZE / 2);

    // 2. Bir sonraki tur için ham sesi (sabit seviyede) üret
    Generate_Sine_Segment(Audio_Ring_Buffer, 0, RING_BUFFER_SIZE / 2, target_frequency);
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
    // 1. İkinci yarıyı filtrele
    Apply_Volume_Filter(Audio_Ring_Buffer, Filter_Ring_Buffer, RING_BUFFER_SIZE / 2, RING_BUFFER_SIZE / 2);

    // 2. Bir sonraki tur için ham sesi üret
    Generate_Sine_Segment(Audio_Ring_Buffer, RING_BUFFER_SIZE / 2, RING_BUFFER_SIZE / 2, target_frequency);
}

/* --- Uygulama Başlatma --- */
void App_Init(void) {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(50);

    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 90, SAMPLING_FREQ) != AUDIO_OK) {
        Error_Handler();
    }

    // Başlangıçta tamponları hazırla
    Generate_Sine_Segment(Audio_Ring_Buffer, 0, RING_BUFFER_SIZE, 0);
    // Çalarken Filter_Ring_Buffer (işlenmiş ses) kullanılır
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

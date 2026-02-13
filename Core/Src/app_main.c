/*
 * app_main.c
 *
 *  Created on: Feb 13, 2026
 *      Author: Gökçe
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

/* --- Değişkenler --- */
int16_t Audio_Ring_Buffer[RING_BUFFER_SIZE];
int16_t Filter_Ring_Buffer[RING_BUFFER_SIZE];
volatile float current_phase = 0.0f;
volatile float target_frequency = 0.0f;

uint32_t t_short = 200;
uint32_t t_gap   = 4;

/* --- DSP Fonksiyonu --- */
void Generate_Sine_Segment(int16_t *buffer, uint32_t start_index, uint32_t length, float freq) {
    float phase_increment = (2.0f * M_PI * freq) / (float)SAMPLING_FREQ;
    for (uint32_t i = start_index; i < start_index + length; i += 2) {
        current_phase += phase_increment;
        if (current_phase >= 2.0f * M_PI) current_phase -= 2.0f * M_PI;

        int16_t val = (freq == 0) ? 0 : (int16_t)(5000 * sinf(current_phase));
        buffer[i]     = val;
        buffer[i + 1] = val;
    }
}

/* --- DMA Callbacks --- */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	//for loop kullanılarak audio ring buffer'dan filter ring buffer'a veri kopyalanacak.
    for (uint32_t i = 0; i < RING_BUFFER_SIZE/2 ; i++)
    {
    	Filter_Ring_Buffer[i] = Audio_Ring_Buffer[i];
    }
    Generate_Sine_Segment(Audio_Ring_Buffer, 0, RING_BUFFER_SIZE / 2, target_frequency);

}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	//for loop kullanılarak audio ring buffer'dan filter ring buffer'a veri kopyalanacak.
	for (uint32_t i = RING_BUFFER_SIZE/2; i < RING_BUFFER_SIZE ; i++)
	    {
	    	Filter_Ring_Buffer[i] = Audio_Ring_Buffer[i];
	    }

    Generate_Sine_Segment(Audio_Ring_Buffer, RING_BUFFER_SIZE / 2, RING_BUFFER_SIZE / 2, target_frequency);
}


/* --- Uygulama Başlatma --- */
void App_Init(void) {
    // DAC Reset
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(50);

    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 100, SAMPLING_FREQ) != AUDIO_OK) {
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
            target_frequency = verse[j]; HAL_Delay(t_short);
        }
    }

    // --- SON SATIR ---
    float chorus_end[] = {NOTE_CS4, NOTE_CS4, NOTE_D4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_CS4};
    for(int j=0; j<7; j++) {
        target_frequency = chorus_end[j]; HAL_Delay(t_short);
    }
    target_frequency = NOTE_B3;  HAL_Delay(t_short * 2);
    target_frequency = 0;        HAL_Delay(1000);
}



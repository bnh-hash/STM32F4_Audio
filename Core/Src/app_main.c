/*
 * app_main.c
 *
 * Created on: Feb 19, 2026
 * Author: Gökçe
 */

#include "app_main.h"
#include "stm32f4_discovery_audio.h"
#include "filter.h"
#include <math.h>

/* --- Tanımlamalar --- */
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define BUFFER_SIZE      (3840)
#define SAMPLING_FREQ    48000

/* --- Dış Değişkenler --- */
extern ADC_HandleTypeDef hadc1;
// USB kütüphanesi tarafından yönetilen ana ses tamponu
extern uint8_t USB_Rx_Buffer[BUFFER_SIZE * 2];

/* --- Global Değişkenler (usbd_audio_if.c ile paylaşılacak) --- */
volatile float global_volume = 0.5f;       // USB Host (Bilgisayar) tarafından kontrol edilecek
volatile float carrier_phase = 0.0f;       // DSP Filtresi için faz takibi
volatile float filter_intensity = 1.0f;    // ADC (Potansiyometre) tarafından kontrol edilecek

/* --- Uygulama Başlatma --- */
void App_Init(void) {
    // 1. Ses Çipi (CS43L22) Resetleme Dizisi
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(50);

    // 2. Ses Donanımını (BSP) Başlat
    // Not: Başlangıçta 70 seviyesinde açıyoruz, USB bağlandığında PC bunu güncelleyecek.
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 70, SAMPLING_FREQ) != AUDIO_OK) {
        Error_Handler();
    }

    // USB'den veri gelene kadar sistemi bekleme modunda bırakıyoruz.
    // Çalma işlemi usbd_audio_if.c içindeki AUDIO_CMD_START ile tetiklenecek.
}

/* --- Ana Melodi Döngüsü --- */
void App_Loop(void) {
    // USB Audio işlemleri DMA ve Kesmeler (Interrupts) ile arka planda çalışır.
    // Ana döngüde sadece ADC (Potansiyometre) okuması yaparak filtre efektini ayarlıyoruz.

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        uint32_t raw_val = HAL_ADC_GetValue(&hadc1);

        // 0-4095 değerini 0.5f - 5.0f aralığında bir çarpana çeviriyoruz (Efekt şiddeti)
        float instant_intensity = 0.5f + ((float)raw_val / 4095.0f) * 4.5f;

        // Yumuşatma (Smoothing) filtresi ile potansiyometre gürültüsünü engelle
        filter_intensity = (filter_intensity * 0.95f) + (instant_intensity * 0.05f);
    }
    HAL_ADC_Stop(&hadc1);

    HAL_Delay(500); // İşlemciyi %100 meşgul etmemek için küçük bir bekleme
}

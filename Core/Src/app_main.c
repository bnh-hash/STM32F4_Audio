/*
 * app_main.c
 * Created on: Feb 19, 2026
 * Author: Gökçe
 */

#include "app_main.h"
#include "stm32f4_discovery_audio.h"
#include "filter.h"
#include <math.h>


/* --- Dış Değişkenler --- */
extern ADC_HandleTypeDef hadc1;

/* --- Global Değişkenler --- */
// potans_val struct'ı filter.c içinde tanımlandığı için burada 'extern' ile erişiyoruz
volatile float carrier_phase = 0.0f; // Ring modülasyonu faz takibi
uint8_t USB_Rx_Buffer[3840];
/* --- ADC Yardımcı Fonksiyonu --- */
/**
 * @brief Belirtilen kanalı aktif eder ve bir kez okuma yapar.
 * @param channel: ADC_CHANNEL_1, ADC_CHANNEL_2 vb.
 */
uint32_t ADC_Read_Helper(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        return 0;
    }

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint32_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    return val;
}

/* --- Uygulama Başlatma --- */
void App_Init(void) {
    // 1. Ses Çipi (CS43L22) Resetleme
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(50);

    // 2. Ses Donanımını Başlat (Başlangıç sesi %70)
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 100, SAMPLING_FREQ) != AUDIO_OK) {
        Error_Handler();
    }
}
/* --- Ana Döngü --- */
void App_Loop(void) {
    /* * 1. ADIM: Potansiyometrelerin ham değerlerini oku.
     * Şematiğine göre PA1(CH1) ve PA2(CH2) kullandığını varsayıyoruz.
     */
    potans_val.raw_adc[0] = ADC_Read_Helper(ADC_CHANNEL_1); // Volume Potu
    potans_val.raw_adc[1] = ADC_Read_Helper(ADC_CHANNEL_2); // Ring Mod Frekans Potu

    /* * 2. ADIM: Okunan ham değerleri fiziksel birimlere (0.0-1.0 ve Hz) dönüştür.
     * Bu fonksiyon filter.c dosyasında yer alacak.
     */
    potans_update(&potans_val);

    /* * 3. ADIM: İşlemciyi boşa yormamak ve ADC'nin kararlı çalışması için küçük bir ara.
     */
    HAL_Delay(20);
}

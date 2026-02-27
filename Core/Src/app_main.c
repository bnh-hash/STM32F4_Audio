/*
 * app_main.c
 * Created on: Feb 19, 2026
 * Author: Gökçe
 * Revize: 1 Dry + 4 Efekt Modlu Seçici
 */

#include "filter.h"
#include "audio_stream.h"
#include "app_main.h"
#include "stm32f4_discovery_audio.h"
#include <math.h>

/* --- Donanim Tanimlamalari --- */
#define USER_BUTTON_PIN  GPIO_PIN_0
#define USER_BUTTON_PORT GPIOA

// LED Tanimlamalari (PD12-PD15)
#define LED_GREEN_PIN    GPIO_PIN_12
#define LED_ORANGE_PIN   GPIO_PIN_13
#define LED_RED_PIN      GPIO_PIN_14
#define LED_BLUE_PIN     GPIO_PIN_15
#define LED_PORT         GPIOD

/* --- Global Değişkenler --- */
volatile float carrier_phase = 0.0f;
filter_params_t app_filter_params;

// Guncel Ses Modu (0-4)
// 0: Full Dry (LEDs OFF)
// 1: Delay, 2: RingMod, 3: AutoWah, 4: Bitcrusher
static uint8_t current_audio_mode = 0;

/* --- LED Kontrol Yardimcisi --- */
void Update_LEDs(uint8_t mode) {
    // Once tum LED'leri sondur
    HAL_GPIO_WritePin(LED_PORT, LED_GREEN_PIN | LED_ORANGE_PIN | LED_RED_PIN | LED_BLUE_PIN, GPIO_PIN_RESET);

    // Moda gore ilgili LED'i yak
    switch (mode) {
        case 0: // Dry Mode
            // Hicbir LED yanmaz
            break;
        case 1: // Delay Mode
            HAL_GPIO_WritePin(LED_PORT, LED_ORANGE_PIN, GPIO_PIN_SET);
            break;
        case 2: // Ring Mod Mode
            HAL_GPIO_WritePin(LED_PORT, LED_RED_PIN, GPIO_PIN_SET);
            break;
        case 3: // Auto Wah Mode
            HAL_GPIO_WritePin(LED_PORT, LED_BLUE_PIN, GPIO_PIN_SET);
            break;
        case 4: // Bitcrusher Mode
            HAL_GPIO_WritePin(LED_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
            break;
    }
}

/* --- Mod Parametrelerini Yukle --- */
void Set_Audio_Mode(uint8_t mode) {
    // TEMIZLIK: Once tum efektleri kapat
    app_filter_params.ring_mod_enable = 0;
    app_filter_params.wah_enable      = 0;
    app_filter_params.lpf_enable      = 0;
    app_filter_params.hpf_enable      = 0;
    app_filter_params.bitcrush_enable = 0;
    app_filter_params.delay_enable    = 0;
    app_filter_params.master_volume   = 1.0f;

    switch (mode) {
        case 0: // --- MODE 0: FULL DRY ---
            // Tum enable flag'leri zaten yukarida 0 yapildi.
            break;

        case 1: // --- MODE 1: DELAY ---
            app_filter_params.delay_enable = 1;
            app_filter_params.delay_feedback = 0.6f;
            app_filter_params.delay_mix = 0.4f;
            break;

        case 2: // --- MODE 2: RING MODULATION ---
            app_filter_params.ring_mod_enable = 1;
            app_filter_params.ring_mod_freq = 300.0f;
            app_filter_params.ring_mod_intensity = 0.4f;
            break;

        case 3: // --- MODE 3: AUTO WAH ---
            app_filter_params.wah_enable = 1;
            app_filter_params.wah_sensitivity = 0.9f;
            app_filter_params.wah_mix = 0.8f;
            break;

        case 4: // --- MODE 4: BITCRUSHER ---
            app_filter_params.bitcrush_enable = 1;
            app_filter_params.bitcrush_amount = 0.3f;
            app_filter_params.master_volume = 1.1f; // Bitcrush'ta kaybolan enerjiyi telafi et
            break;
    }

    // Guncel parametreleri filtre motoruna gonder
    Filter_Set_Params(&app_filter_params);

    // Gostergeyi guncelle
    Update_LEDs(mode);
}

/* --- Uygulama Başlatma --- */
void App_Init(void) {
    // 1. Ses Çipi Resetleme
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(50);

    // 2. Ses Donanımını Başlat
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, 85, AUDIO_SAMPLE_RATE) != AUDIO_OK) {
        Error_Handler();
    }

    // 3. Baslangic Modu (Mode 0 - Dry)
    current_audio_mode = 0;
    Set_Audio_Mode(current_audio_mode);

    // 4. Filtre API'sini Baslat
    Filter_Init(&app_filter_params);
}

/* --- Buton Kontrol Fonksiyonu --- */
void Check_Filter_Button(void) {
    if (HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN) == GPIO_PIN_SET) {
        HAL_Delay(50); // Debounce
        if (HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN) == GPIO_PIN_SET) {

            // Modu artir
            current_audio_mode++;

            // 4. moddan sonra (toplam 5 durum) basa don (0, 1, 2, 3, 4 -> 0)
            if (current_audio_mode > 4) {
                current_audio_mode = 0;
            }

            // Yeni modu uygula
            Set_Audio_Mode(current_audio_mode);

            // Butonun birakilmasini bekle (Sonsuz döngü koruması)
            while (HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN) == GPIO_PIN_SET) {
                HAL_Delay(10);
            }
        }
    }
}

/* --- Ana Döngü --- */
void App_Loop(void) {
    Check_Filter_Button();

    // (Opsiyonel) Ileride ekleyecegin potansiyometre okumalari buraya gelecek:
    // uint32_t pot = ADC_Read_Helper(ADC_CHANNEL_1);
    // Process_Smart_Knob((float)pot / 4095.0f);

    HAL_Delay(10);
}

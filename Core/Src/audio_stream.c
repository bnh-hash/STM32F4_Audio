#include <filter.h>
#include "audio_stream.h"
#include "app_main.h"
#include "usbd_audio_if.h"
#include <string.h>

extern USBD_HandleTypeDef hUsbDeviceFS;
extern volatile float carrier_phase;

static uint8_t Ring_Buffer[RING_BUFFER_SIZE];
static volatile uint32_t ring_write_ptr = 0;
static volatile uint32_t ring_read_ptr = 0;
static volatile int32_t ring_available_bytes = 0;

// --- TX BUFFER ---
int16_t Audio_Tx_Buffer[TX_FULL_SAMPLES];

// --- DISARIYA ACILAN FONKSIYONLAR ---

void AudioStream_Init(void) {
    AudioStream_Reset();
}

void AudioStream_Reset(void) {
    ring_write_ptr = 0;
    ring_read_ptr = 0;
    ring_available_bytes = 0;
    memset(Audio_Tx_Buffer, 0, sizeof(Audio_Tx_Buffer));
    memset(Ring_Buffer, 0, sizeof(Ring_Buffer));
}

// USB'den gelen veriyi buraya atiyoruz
void AudioStream_Write_USB_Packet(uint8_t* pbuf, uint32_t len) {
    // Tampon tasmak uzereyse yeni paketi cope at (Head Drop)
    // Bu, pointerlarin karismasindan daha guvenlidir.
    if ((ring_available_bytes + len) > RING_BUFFER_SIZE) {
        return;
    }

    for (uint32_t i = 0; i < len; i++) {
        Ring_Buffer[ring_write_ptr] = pbuf[i];
        ring_write_ptr = (ring_write_ptr + 1) % RING_BUFFER_SIZE;
    }

    // Atomik islem simulasyonu (Kesme girerse sorun olmasin)
    __disable_irq();
    ring_available_bytes += len;
    __enable_irq();
}

uint8_t AudioStream_Is_Ready_To_Play(void) {
    // Tamponun %50'si dolmadan calmayi baslatma (Pre-Buffering)
    if (ring_available_bytes >= TARGET_LEVEL) {
        return 1;
    }
    return 0;
}

// --- AKILLI OKUMA VE DUZELTME (CORE LOGIC) ---
static void Process_Audio_Chunk(int16_t* pOutputBuffer, uint32_t n_samples) {
    uint32_t bytes_needed = n_samples * 2; // int16 * sample sayisi

    // 1. DUZELTME MEKANIZMASI (DRIFT CORRECTION)
    // Bu islem veriyi kopyalamadan ONCE yapilir.

    // Durum A: Tampon cok DOLU (STM32 yavas, PC hizli) -> HIZLANMALIYIZ
    if (ring_available_bytes > (TARGET_LEVEL + DRIFT_THRESHOLD))
    {
        // Okuma pointerini 4 byte (1 ornek) ileri itiyoruz.
        // Sanki 1 ornegi okumusuz gibi atliyoruz (Drop).
        ring_read_ptr = (ring_read_ptr + (AUDIO_CHANNELS * BYTES_PER_SAMPLE)) % RING_BUFFER_SIZE;

        __disable_irq();
        ring_available_bytes -= (AUDIO_CHANNELS * BYTES_PER_SAMPLE);
        __enable_irq();
    }
    // Durum B: Tampon cok BOS (STM32 hizli, PC yavas) -> YAVASLAMALIYIZ
    else if (ring_available_bytes < (TARGET_LEVEL - DRIFT_THRESHOLD))
    {
        // Okuma pointerini 4 byte (1 ornek) geri cekiyoruz.
        // Bir sonraki dongude ayni ornegi tekrar okuyacagiz (Repeat).
        // (Not: Dizi baslangicinda eksiye dusmemek icin +SIZE ekliyoruz)
        ring_read_ptr = (ring_read_ptr - (AUDIO_CHANNELS * BYTES_PER_SAMPLE) + RING_BUFFER_SIZE) % RING_BUFFER_SIZE;

        __disable_irq();
        ring_available_bytes += (AUDIO_CHANNELS * BYTES_PER_SAMPLE);
        __enable_irq();
    }

    // 2. KOPYALAMA ISLEMI (Underrun Korumali)
    if (ring_available_bytes < bytes_needed) {
        // Tamponda yeterli veri yoksa SESSISZLIK bas (Citirtiyi onlemek icin memset 0)
        memset(pOutputBuffer, 0, bytes_needed);
        // Pointerlari ellemiyoruz, veri gelmesini bekliyoruz.
    }
    else {
        // Ring Buffer -> Output Buffer
        uint8_t* pOut8 = (uint8_t*)pOutputBuffer;
        for (uint32_t i = 0; i < bytes_needed; i++) {
            pOut8[i] = Ring_Buffer[ring_read_ptr];
            ring_read_ptr = (ring_read_ptr + 1) % RING_BUFFER_SIZE;
        }

        __disable_irq();
        ring_available_bytes -= bytes_needed;
        __enable_irq();
    }
}

// HT Kesmesi Logic'i
void AudioStream_Process_Half_Transfer(void) {
    USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_HALF);

    // Audio_Tx_Buffer'in ILK yarisina yazacagiz
    int16_t* pSafeZone = &Audio_Tx_Buffer[0];

    // Veriyi Ring Buffer'dan cek ve Drift Duzeltmesi yap
    Process_Audio_Chunk(pSafeZone, TX_HALF_SAMPLES);

    // DSP BURADA (Dilerseniz acin)
    Filter_Apply(pSafeZone, TX_HALF_SAMPLES);
}

// TC Kesmesi Logic'i
void AudioStream_Process_Full_Transfer(void) {
    USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_FULL);

    // Audio_Tx_Buffer'in IKINCI yarisina yazacagiz
    int16_t* pSafeZone = &Audio_Tx_Buffer[TX_HALF_SAMPLES];

    // Veriyi Ring Buffer'dan cek ve Drift Duzeltmesi yap
    Process_Audio_Chunk(pSafeZone, TX_HALF_SAMPLES);

    // DSP BURADA
    Filter_Apply(pSafeZone, TX_HALF_SAMPLES);
}

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_audio_if.c
  * @version        : v1.0_Cube
  * @brief          : Generic media access layer.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"
#include "stm32f4_discovery_audio.h" // BSP Ses Kütüphanesi
#include "filter.h"                  // Senin DSP Kütüphanen
#include "app_main.h"                // Global değişkenler için

/* USER CODE BEGIN PV */
extern I2S_HandleTypeDef hi2s3;
extern USBD_HandleTypeDef hUsbDeviceFS;

// app_main.c'den gelen global DSP ve ses kontrol değişkenleri
extern volatile float global_volume;
extern volatile float carrier_phase;
extern volatile float filter_intensity;
extern uint8_t USB_Rx_Buffer[];

volatile uint8_t play_state = 0;    // 0: Durdu, 1: Doluyor, 2: Çalıyor
volatile uint32_t fill_counter = 0; // 1ms'lik paket sayacı
/* USER CODE END PV */

/* Private functions ---------------------------------------------------------*/

static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
  return (USBD_OK);
}

static int8_t AUDIO_DeInit_FS(uint32_t options)
{
  return (USBD_OK);
}

/**
  * @brief  Handles AUDIO command (Oynat/Durdur)
  */
static int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
  switch(cmd)
  {
    case AUDIO_CMD_START:
      // Bilgisayar "Oynat" dediğinde I2S ve CS43L22 DAC çipini başlat
      // size byte cinsindendir, ancak bizim BSP fonksiyonları genellikle byte bekler.
    	fill_counter = 0;
    	play_state = 1;
      break;

    case AUDIO_CMD_STOP:
      // Bilgisayar sesi durdurduğunda DAC'ı uyku moduna al
    	play_state = 0;
      BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
      break;
  }
  return (USBD_OK);
}

/**
  * @brief  Controls AUDIO Volume (Bilgisayardan ses açıp kısma)
  */
static int8_t AUDIO_VolumeCtl_FS(uint8_t vol)
{
  // vol (0-100 aralığı) bilgisayardan gelir.

  // Ayrıca donanımsal DAC sesini de güncelleyebiliriz
  BSP_AUDIO_OUT_SetVolume(vol);

  return (USBD_OK);
}

static int8_t AUDIO_MuteCtl_FS(uint8_t cmd)
{
  BSP_AUDIO_OUT_SetMute(cmd);
  return (USBD_OK);
}

static int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd)
{
	if (play_state == 1)
	  {
		  fill_counter++;
		  if (fill_counter >= 10)
		  {
			  play_state = 2;
			  BSP_AUDIO_OUT_Play((uint16_t*)pbuf, size * 2);
		  }
	  }
  return (USBD_OK);
}

static int8_t AUDIO_GetState_FS(void)
{
  return (USBD_OK);
}

/* --- DMA KESMELERİ VE DSP FİLTRE UYGULAMASI --- */
/* Bu fonksiyonlar USB verisi I2S'e aktarılırken tamponun yarısı ve
   tamamı bittiğinde tetiklenir. Filtrelerimizi burada veri I2S'e gitmeden
   MİLİSANİYELER İÇİNDE sese uygulayacağız. */

/**
  * @brief  Tamponun ilk yarısı çalınırken, yeni gelen ilk yarıyı işle
  */
void HalfTransfer_CallBack_FS(void)
{
  // 1. USB Feedback Senkronizasyonu (Çıtırtıları önler)
  USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_HALF);

  // 2. DSP Parametrelerini hesapla (ADC'den okunan intensity ile)
  float robot_freq = 1000.0f * filter_intensity;

  // 3. Veri göstericisini ayarla (Tamponun başı)
  // pData int16_t (16-bit) formatına cast ediliyor.
  int16_t* pData = (int16_t*)USB_Rx_Buffer;

  // BUFFER_SIZE'ın yarısı kadar örnek (sample) işlenecek
  uint32_t sample_count = (3840 / 2); // 3840 tanımını app_main.c'den al veya makro yap

  // 4. DSP Filtrelerini Uygula
  //filter_apply_ring(pData, sample_count, (float*)&carrier_phase, robot_freq, 48000);
  //filter_apply_volume(pData, sample_count, global_volume);
}

/**
  * @brief  Tamponun ikinci yarısı çalınırken, yeni gelen ikinci yarıyı işle
  */
void TransferComplete_CallBack_FS(void)
{
  // 1. USB Feedback Senkronizasyonu
  USBD_AUDIO_Sync(&hUsbDeviceFS, AUDIO_OFFSET_FULL);

  // 2. DSP Parametreleri
  float robot_freq = 1000.0f * filter_intensity;

  // 3. Veri göstericisini ayarla (Tamponun ortası)
  uint32_t sample_count = (3840 / 2);
  int16_t* pData = (int16_t*)&USB_Rx_Buffer[sample_count * 2]; // byte cinsinden offset

  // 4. DSP Filtrelerini Uygula
  //filter_apply_ring(pData, sample_count, (float*)&carrier_phase, robot_freq, 48000);
  //filter_apply_volume(pData, sample_count, global_volume);
}


USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops_FS =
{
  AUDIO_Init_FS,
  AUDIO_DeInit_FS,
  AUDIO_AudioCmd_FS,
  AUDIO_VolumeCtl_FS,
  AUDIO_MuteCtl_FS,
  AUDIO_PeriodicTC_FS,
  AUDIO_GetState_FS,
};

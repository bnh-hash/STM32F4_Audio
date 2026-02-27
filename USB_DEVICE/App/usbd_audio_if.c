/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_audio_if.c
  * @brief          : Generic media access layer.
  * MODULER YAPI: USB Arayuzu ve Donanim Koprusu
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_audio_if.h"
<<<<<<< Updated upstream
#include "stm32f4_discovery_audio.h" // BSP Ses Kütüphanesi
#include "filter.h"                  // Senin DSP Kütüphanen
#include "app_main.h"                // Global değişkenler için
=======
#include "stm32f4_discovery_audio.h"
#include "audio_stream.h" // YENI MOTOR DOSYAMIZ
>>>>>>> Stashed changes

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;

// Oynatma Durum Makinesi
typedef enum {
    AUDIO_STATE_STOPPED = 0,
    AUDIO_STATE_BUFFERING,
    AUDIO_STATE_PLAYING
} AudioState_TypeDef;

static volatile AudioState_TypeDef current_audio_state = AUDIO_STATE_STOPPED;

/* USER CODE END PV */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the AUDIO media low layer over USB FS IP
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Volume: Initial volume level (0..100)
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_Init_FS(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
  AudioStream_Init();

  current_audio_state = AUDIO_STATE_STOPPED;

  return (USBD_OK);
}

/**
  * @brief  De-Initializes the AUDIO media low layer
  * @param  options: Reserved for future use
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_DeInit_FS(uint32_t options)
{
  BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
  current_audio_state = AUDIO_STATE_STOPPED;
  return (USBD_OK);
}

/**
  * @brief  Handles AUDIO command.
  * @param  pbuf: Pointer to buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_AudioCmd_FS(uint8_t* pbuf, uint32_t size, uint8_t cmd)
{
  switch(cmd)
  {
    case AUDIO_CMD_START:
      // Stream motorunu sifirla
      AudioStream_Reset();
      // Durumu "Tampon Doluyor"a getir. Hemen calmiyoruz!
      current_audio_state = AUDIO_STATE_BUFFERING;
      break;

    case AUDIO_CMD_STOP:
      current_audio_state = AUDIO_STATE_STOPPED;
      BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
      break;
  }
  return (USBD_OK);
}

/**
  * @brief  Controls AUDIO Volume.
  * @param  vol: Volume level (0..100)
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_VolumeCtl_FS(uint8_t vol)
{
  BSP_AUDIO_OUT_SetVolume(vol);
  return (USBD_OK);
}

/**
  * @brief  Controls AUDIO Mute.
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_MuteCtl_FS(uint8_t cmd)
{
  BSP_AUDIO_OUT_SetMute(cmd);
  return (USBD_OK);
}

/**
  * @brief  AUDIO_PeriodicTC_FS
  * @param  cmd: Command opcode
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_PeriodicTC_FS(uint8_t *pbuf, uint32_t size, uint8_t cmd)
{
  // Eger durmussak hicbir sey yapma
  if (current_audio_state == AUDIO_STATE_STOPPED) {
      return (USBD_OK);
  }

  // 1. Gelen Paketi Motora Teslim Et (Ring Buffer'a Yaz)
  AudioStream_Write_USB_Packet(pbuf, size);

  // 2. Durum Kontrolu: Eger Buffer Doluyorsa ve Yeterli Seviyeye Geldiyse
  if (current_audio_state == AUDIO_STATE_BUFFERING)
  {
      if (AudioStream_Is_Ready_To_Play())
      {
          current_audio_state = AUDIO_STATE_PLAYING;
          BSP_AUDIO_OUT_Play((uint16_t*)Audio_Tx_Buffer, TX_FULL_SAMPLES * 2);
      }
  }

  return (USBD_OK);
}

/**
  * @brief  Gets AUDIO State.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t AUDIO_GetState_FS(void)
{
  return (USBD_OK);
}

/* --- BSP CALLBACK BRIDGE (DONANIM KOPRUSU) --- */
/* DMA Kesmeleri Tetiklendiginde Burasi Calisir */

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
<<<<<<< Updated upstream
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
=======
  // Eger calmiyorsak bosuna islem yapma
  if (current_audio_state == AUDIO_STATE_PLAYING) {
      AudioStream_Process_Half_Transfer();
  }
>>>>>>> Stashed changes
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
<<<<<<< Updated upstream
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
=======
  // Eger calmiyorsak bosuna islem yapma
  if (current_audio_state == AUDIO_STATE_PLAYING) {
      AudioStream_Process_Full_Transfer();
  }
>>>>>>> Stashed changes
}

/* --- LINKER HATASINI COZEN YAPI --- */
/* USB Device Library bu yapiyi 'extern' olarak arar. */

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

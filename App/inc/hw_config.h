/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : hw_config.h
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
 #include "stm32f10x.h"
#include <stdbool.h> 

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define BULK_MAX_PACKET_SIZE  0x00000040
//#define USART_RX_DATA_SIZE   128 //2048 //debugga
#define MAX_SIZE_PACKET_GPRS_SIM900      1360
#define SIZE_PACKET_GPRS_OPER            2100 //2048
#define USB_RING_TX_DATA_SIZE                 2048
#define USB_RING_RX_DATA_SIZE                 2100

//#define BRIDGE_USB_GSM
//#define BRIDGE_USB_GPS

/* Exported functions ------------------------------------------------------- */
void Set_System(void);
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void Get_SerialNum(void);
void MAL_Config(void);


//vcp
void USB_To_USART_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes);
void Handle_USBAsynchXfer (void);
void USART_Config_Default(void);
bool USART_Config(void);
//void USART_To_USB_Send_Data(u8 byte);


void USBWriteStr(const char * buf);
void USBWriteLen(u8 * buf, u16 len);
void USBWriteChar(u8 ch);
void USBDebWrite(u32 dig);
//u16  USBRead(u8 * buf);
u16  USBReadLen(u8 * buf, u8 len);
u16  USBReadWhole(u8 * buf);
void handleTimerUSBBoot(void);

u8  USBDetectPin(void);
void USBIniPin(void);
void USBCommonIni(void);

void USB_To_Outside_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes);

#if defined (VER_3)
  void USB_PUPControl( u8 flag );
#endif

#if defined (USE_STM3210B_EVAL) || defined (USE_STM3210E_EVAL)
void USB_Disconnect_Config(void);
#endif /* USE_STM3210B_EVAL or USE_STM3210E_EVAL */
/* External variables --------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif  /*__HW_CONFIG_H*/

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

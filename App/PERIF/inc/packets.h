/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : spi_flash.h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Header for spi_flash.c file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PACKETS_H
#define __PACKETS_H
#include "common.h"

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void handleTimerPackets(void);
//void handleTimerPacketsGSM(void);
//void gpsPacketQueueHandle(void);
void rcvPacketHandle(void);
void rcvPacketHandleUSB(void);
void naviPacketHandle(void);
void packetsIni(void);
void sayHello(void);
u8 getInfo(void); // tempora

#ifdef __cplusplus
}
#endif
#endif /* __PACKETS_H */

/******************** (C) COPYRIGHT  ********************
* File Name          : gps.h
* Author             : 
* Version            : 
* Date               : 
* Description        : Header for gps.c file.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS_H
#define __GPS_H

#include "common.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define GPS_PWR_PORT                     GPIOD  
#if defined (VER_3)
   #define GPS_PWR_PORT_PIN                 GPIO_Pin_3
#else
   #define GPS_PWR_PORT_PIN                 GPIO_Pin_12
#endif

#define RCC_APB2Periph_GPIO_GPS_PWR      RCC_APB2Periph_GPIOD

#define GPS_PPS_PORT                     GPIOA  
#if defined (VER_3)
   #define GPS_PPS_PORT_PIN                 GPIO_Pin_8
#else
   #define GPS_PPS_PORT_PIN                 GPIO_Pin_15
#endif
#define RCC_APB2Periph_GPIO_GPS_PPS      RCC_APB2Periph_GPIOA

#define GPS_PRIM_RECEIVE_BUFFER_SIZE     768
#define GPS_HANDLE_NMEA_BUFFER_SIZE      GPS_PRIM_RECEIVE_BUFFER_SIZE
//#define GPS_SENDING_BUFFER_SIZE		     512

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void gpsInit(void);
void gpsHandling(void);
void gps_usart_decr_timer(void);
void gps_rx_get_byte(u8 byte);
void calcDistance(void);
u8   getFlagGPSChanges(void);
u8   getGLONASSStateValid(void);	// 1 - ok valid, 0 - no valid
u16  getGpsPacket(u8 * buf);
u16  zeroMileage(u8 * buf);
#ifdef __cplusplus
}
#endif
#endif /* __GPS_H */

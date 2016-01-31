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
#ifndef __PROTOCOL_ASCII_H
#define __PROTOCOL_ASCII_H
#include "common.h"

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct _PGIOInfo
{
  u32 uID;
  u8  flagStart;
  u32 orderMsgNumber;
  u8  stateInput;
  u8  stateOutputs; 
  u16 stateADC[4];   //1,2 - ADC converting , 3,4 - supply power & battery voltage - should present 
  					 // in view XX.X
  u16 stateThermo[8];
  u16 stateFuel[2];
}PGIOInfo;

typedef struct _GPRMCInfo
{
  //time hhmmss.hh
  u8 timeDigits[6];
  u8 stateValidGPS;
  //5547.9890,N,04907.9424,E - coord
  //u8 *longDigits[9];
  double longitude;
  u8 longLetter;
  double latitude;
  u8 latLetter;
  //000.00050 speed in miles
  double speedMiles;
  //271.3 - course 
  double direction;
  //290311 - date
  u8 dateDigits[6];

}GPRMCInfo;

typedef struct _GNGSAInfo
{
  u8 GLNNumber[2];
  u8 GPSNumber[2];
}GNGSAInfo;

typedef struct _MLGInfo
{
  u32  mileage;		 //mileage in meters
  //double mileageExact;
  u16  speedKM;
}MLGInfo;

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

u16  gprmcPassed(u16 len, u8 * buf_tx);
u16  gngsaPassed(u16 len, u8 * buf_tx);
u16  mlgPassed(u16 len, u8 * buf_tx);
u16  pgioPassed(u16 len, u8 * buf_tx);
u8   findLen(const u8 *buf, u16 len);



#ifdef __cplusplus
}
#endif
#endif /* __PROTOCOL_ASCII_H */

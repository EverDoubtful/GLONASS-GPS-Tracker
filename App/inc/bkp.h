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
#ifndef __BKP_H
#define __BKP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
/* MAP OF REGISTERS-----------------------------------------------------------*/
//indexes - in BKPDataReg Array
// 0 - RTC Check
// 1,2 - pgioInfo.orderMsgNumber(u32)
// 3,4 - position in notsend.txt file 
// 5,6 - mileage 
#define BKP_DR_MSG_NUMBER1       1
#define BKP_DR_MSG_NUMBER2       2
#define BKP_DR_INDEX_POSWR_N1    3
#define BKP_DR_INDEX_POSWR_N2    4
#define BKP_DR_MILEAGE_N1        5
#define BKP_DR_MILEAGE_N2        6
#define BKP_DR_INDEX_POSREAD_N1  7
#define BKP_DR_INDEX_POSREAD_N2  8
#define BKP_DR_INNER_STATE_N1    9
#define BKP_DR_INNER_STATE_N2    10
	   //debugga 
//#define BKP_DR_INDEX_POSFI_N1    11
//#define BKP_DR_INDEX_POSFI_N2    12


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
u16 BKPReadReg(u8 index);
void BKPWriteReg(u8 index, u16 data);
void BKPInit(void);
/* External variables --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif  /* __BKP_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

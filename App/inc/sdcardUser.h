/******************** (C) COPYRIGHT 2011  ********************
* File Name          : sdcardUser.h
* Author             : 
* Version            : 
* Date               : 
* Description        : 
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SDCARD_USER_H
#define __SDCARD_USER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
 #include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
//todo  : make bit fields
//bits for saveSDInfo()
/*bit 0,1 : wait or already sended*/
#define     SD_NOTSEND            0x01
#define     SD_SENDED             0x02
/*bit 1,2 : ascii or binary*/
#define     SD_ASCII_PROTOCOL     0x04
#define     SD_BINARY_PROTOCOL    0x08
/*bit 3,4 : write/read - 0/1*/
#define     SD_WRITE              0x10
#define     SD_READ               0x20
///*bit 2 : navi - bad or good signal*/
//#define     SD_BAD_SIGNAL         0x10
//#define     SD_GOOD_SIGNAL        0x20
typedef enum 
{
   SD_TYPE_MSG_LOG = 0,
   SD_TYPE_MSG_GSM
}sdTypeMsg;

/* Exported functions ------------------------------------------------------- */
void saveSDInfo(u8 * bufExt,  u16 nSize, u8 flags, sdTypeMsg typeMsg);	   // 
u16  readSDInfo(u8 * bufExt, u16 len);
void initSD(void);
#if 0
//u16 readSDBlock(u8 * bufExt, u16 len);
u16 readSDBlock( u16 len );
//u32 saveSDBlock(u8 * bufExt,  u16 nSize );
u32 saveSDBlock( u16 nSize );
#endif
/* External variables --------------------------------------------------------*/



#ifdef __cplusplus
}
#endif

#endif  /*__SDCARD_USER_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

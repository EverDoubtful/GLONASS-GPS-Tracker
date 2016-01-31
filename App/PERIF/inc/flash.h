/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : .h


 Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H
#define __FLASH_H
#include "common.h"

#ifdef __cplusplus
 extern "C" {
#endif


//----------------------SETTINGS  sector------------------------
#define FLASH_PAGE_SIZE	    		2048
#define ADDRESS_LAST_PAGE  			0x807F800  // f103VE
//#define ADDRESS_LAST_PAGE  		0x80FF800  // f103VG
#define START_ADDRESS_FW  			((u32)0x08040000)  //second half 	of whole size of flash(256KB for F103VE)
#define PAGE_SIZE          			(FLASH_PAGE_SIZE + 10)
#define UPGRADE_BUF_SIZE     		2048   //2058
#define START_ADDRESS_SETTINGS  	(ADDRESS_LAST_PAGE - FLASH_PAGE_SIZE)  // pre-last page with settings
#define ADDRESS_PAGE_CALIBR_KOEF 	(START_ADDRESS_SETTINGS - FLASH_PAGE_SIZE)  
//----------------------END OF SETTINGS sector------------------------

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void read_page(u32 adr, u8 *buf, u16 nSize);
u8   write_firm_page(u32 adr, u8 *buf);
u8   addPortion(u16 len, u8 *buf);
u8   addPage(u16 len, u8 *buf);
void UpdatingFlash(void);
void write_firm_pageFTP(u8 sign);
u8   write_firm_page(u32 adr, u8 *buf);
u8   blEnter(void);
u8   blCheckCRC(u8 * buf);
u8   systemRestartDelayed(void);
void system_restart(void);
u8   getVersion(u8 * buf);

#ifdef __cplusplus
}
#endif
#endif /* __FLASH_H */

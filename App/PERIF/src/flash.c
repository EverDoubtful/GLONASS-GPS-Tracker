/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : gps.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : This file provides a set of functions needed to manage the
*                      communication between SPI peripheral and SPI M25P64 FLASH.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "dio_led.h"
#include "bkp.h"
#include "hw_config.h"
#include "flash.h"
#include "crc.h"
#include "hand_conf.h"
#include "bin_handle.h"
#include <string.h>
#include "protocol_bin.h"

u8      flagUSBBootStarted = 0;
//void    setTimerUSBBoot(u32 val);
u32     timerValueUSBBoot;
u8      write_firm_page(u32 adr, u8 *buf);
u8      UpgradeBuffer[UPGRADE_BUF_SIZE];
u32     cntUSBBootBytes;
u32     adrUSBBoot = START_ADDRESS_FW;
//----------------------BOOT sector------------------------
u8   updateFlag;
extern  u16 byteCntPGSize;
u8  cntPageSaved;

extern t_innerState innerState;
//----------------------END OF BOOT SECTOR------------------------
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u8 addPage(u16 len, u8 *buf)
{
    u8  code = 0;
    u32 crc, crcCalc;
	if((len - 4) == UPGRADE_BUF_SIZE)   // -4 (CRC), we read the whole page
	   {
      	  memcpy(&UpgradeBuffer[cntUSBBootBytes], buf, (len-4));
		  buf += UPGRADE_BUF_SIZE;		  
          write_firm_page(adrUSBBoot,UpgradeBuffer);	 
          adrUSBBoot += FLASH_PAGE_SIZE;
  		  read_page(START_ADDRESS_FW + cntPageSaved * FLASH_PAGE_SIZE , UpgradeBuffer, FLASH_PAGE_SIZE); 
          cntPageSaved++;
  		  crc = *(buf) | (*(buf+1)<< 8) | (*(buf+2)<< 16) | (*(buf+3)<< 24);
		  crcCalc = calcBlock(UpgradeBuffer,FLASH_PAGE_SIZE);
		  if(crc == crcCalc)  
		    code  = 1;
		  buf -= UPGRADE_BUF_SIZE;
		  *(u16 *)buf = cntPageSaved;
		  buf += 2;
		  *buf++ = code;

	   }
	else   //error
	   {
	     *(u16 *)buf = 0xffff; 
	   }
	//USBWriteStr("addP:");
	//USBDebWrite(cntPageSaved);
	//USBWriteStr("\r\n");
    return 3; 

}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u8 addPortion(u16 len, u8 *buf)
{
      u8 templen;
	  if((cntUSBBootBytes + len) <= UPGRADE_BUF_SIZE)
      	  memcpy(&UpgradeBuffer[cntUSBBootBytes], buf, len);		  
      if(cntUSBBootBytes != FLASH_PAGE_SIZE )   /* we are reading page*/
    	     cntUSBBootBytes += len;
	  *(u16 *)buf = cntUSBBootBytes;
	  buf += 2;
      templen = 2; 

      if(cntUSBBootBytes == FLASH_PAGE_SIZE)
       {
         cntUSBBootBytes = 0;
         write_firm_page(adrUSBBoot,UpgradeBuffer);	 
         adrUSBBoot += FLASH_PAGE_SIZE;
         cntPageSaved++;
       }
      //if(cntUSBBootBytes >= UPGRADE_BUF_SIZE)	/* Protect just in case */
    	//	cntUSBBootBytes = 0;
	  cntUSBBootBytes %= UPGRADE_BUF_SIZE;
	  return templen;

}
/*******************************************************************************/
/* first 2 bytes of data - pacgeNum, next 4bytes -crcCode    			   */
///* If we want change buffer by info we should remove pointer 5 bytes right(pil+cmd+aux+len)*/
/*******************************************************************************/
u8 blCheckCRC(u8 * buf)
{
  u8 code = 0;
  //u32 crc, crcCalc;
  u16 pageNum;
  ///* If we want change buffer by info we should remove pointer 5 bytes right(pil+cmd+aux+len)*/
  //buf += SIZE_HEADER_PACKET;
  pageNum = *(u16 *)buf; 
  buf += 2;
  //crc = *(buf) | (*(buf+1)<< 8) | (*(buf+2)<< 16) | (*(buf+3)<< 24);
  buf -= 2;
  read_page(START_ADDRESS_FW + pageNum * FLASH_PAGE_SIZE , UpgradeBuffer, FLASH_PAGE_SIZE); 
  //crcCalc = CRC32WideFast(0xffffffff, FLASH_PAGE_SIZE, UpgradeBuffer); 
  //crcCalc = calcBlock(UpgradeBuffer,FLASH_PAGE_SIZE);
  //if(crc == crcCalc) 
  if( (*(buf) | (*(buf+1)<< 8) | (*(buf+2)<< 16) | (*(buf+3)<< 24)) == calcBlock(UpgradeBuffer,FLASH_PAGE_SIZE)) 
    code  = 1;
  *buf++ = code;
  return 1; 

}
/*******************************************************************************/
/* first 2 bytes of data - pacgeNum, next 4bytes -crcCode    			   */
///* If we want change buffer by info we should remove pointer 5 bytes right(pil+cmd+aux+len)*/
/*******************************************************************************/
u8 getVersion(u8 * buf)
{

  *buf++      = VERSION_MAIN;
  *buf++      = VERSION_SUB;
  *(u16 *)buf = VERSION_BUILD;
  buf += 2;

  return 4; 
}


/*******************************************************************************/
/*     			   */
/*******************************************************************************/
u8 blEnter(void)
{
  cntUSBBootBytes = 0;
  adrUSBBoot = START_ADDRESS_FW;
  cntPageSaved = 0;
  innerState.bootloaderStarted = 1;
  return 0; 
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u8  systemRestartDelayed(void)
{
  u32 Address = 0x00;
  u32 Data;
  volatile FLASH_Status FLASHStatus;

  FLASHStatus = FLASH_ErasePage(ADDRESS_LAST_PAGE);
  Address = ADDRESS_LAST_PAGE;
  Data = 0 | (cntPageSaved << 8);
  if( FLASHStatus == FLASH_COMPLETE)
    {
	  while((Address < (ADDRESS_LAST_PAGE+FLASH_PAGE_SIZE)) && (FLASHStatus == FLASH_COMPLETE))
	  {
	    FLASHStatus = FLASH_ProgramWord(Address,Data );
	    Address = Address + 4;
		Data = 0xFFFFFFFF;
	  }
    }

    // restart system
	innerState.needRestart = 1;
	return 0; 
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
//todo - should change delayed restart - to answer for the command
void  system_restart(void)
{
  u32 Address = 0x00;
  u32 Data;
  volatile FLASH_Status FLASHStatus;
  FLASHStatus = FLASH_ErasePage(ADDRESS_LAST_PAGE);
  Address = ADDRESS_LAST_PAGE;
  //Data = 0xFFFFFF00;
  Data = 0 | (cntPageSaved << 8);
  if( FLASHStatus == FLASH_COMPLETE)
    {
	  while((Address < (ADDRESS_LAST_PAGE+FLASH_PAGE_SIZE)) && (FLASHStatus == FLASH_COMPLETE))
	  {
	    FLASHStatus = FLASH_ProgramWord(Address,Data );
	    Address = Address + 4;
		Data = 0xFFFFFFFF;
	  }
    }
    __disable_irq();
    // restart system
    NVIC_SystemReset();
}

/*******************************************************************************/
/* 	That's for FTP Upgrading																		   */
/*******************************************************************************/

void UpdatingFlash(void)
{
  if(updateFlag)
   {
	  // 1- for the first page, 2 - for the others 
      write_firm_pageFTP(updateFlag);
	  updateFlag = 0;
   }
}
/*******************************************************************************/
/* 	That's for FTP Upgrading																		   */
/*******************************************************************************/

void write_firm_pageFTP(u8 sign)
{
  static u16 pageCnt = 0;
  volatile FLASH_Status FLASHStatus;
  u32 startAdres = 0,Address;
  u32 data;
  u16 byteCnt = 0;
  if(sign == 1)
    pageCnt = 0;

 /*  FLASH Word program*/
  Address	= START_ADDRESS_FW + pageCnt * FLASH_PAGE_SIZE;
  startAdres = Address;
  /*handle if quantity of bytes does not equal to pageSize(not full page)*/
  if(byteCntPGSize != (FLASH_PAGE_SIZE))
    {
      while(byteCntPGSize < (FLASH_PAGE_SIZE))
  	      UpgradeBuffer[byteCntPGSize++] = 0xFF;
    }
  byteCntPGSize = 0;
  FLASHStatus = FLASH_ErasePage(Address);
  if( FLASHStatus == FLASH_COMPLETE)
    {
	  while((Address < (startAdres+FLASH_PAGE_SIZE)) && (FLASHStatus == FLASH_COMPLETE))
	  {
		data = *((u32 *)&UpgradeBuffer[byteCnt]);
	    FLASHStatus = FLASH_ProgramWord(Address,data );
	    Address = Address + 4;
		byteCnt += 4;
	  }
    }
  pageCnt++;
  cntPageSaved = pageCnt;
}


/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleTimerUSBBoot(void)
{
    if(timerValueUSBBoot)
	  {
	   timerValueUSBBoot--;
	   if(!timerValueUSBBoot )
		  {
	        flagUSBBootStarted = 0;
			adrUSBBoot = START_ADDRESS_FW;
		  }
	  }
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
//void setTimerUSBBoot(u32 val)
//{
//   	timerValueUSBBoot = val;
//}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

u8 write_firm_page(u32 adr, u8 *buf)
{
  u16 byteCnt = 0;
  u32 adres = adr;
  u32 data;
  volatile FLASH_Status FLASHStatus;

 /*  FLASH Word program*/
  FLASHStatus = FLASH_ErasePage(adr);
  if( FLASHStatus == FLASH_COMPLETE)
    {
	  while((adres < (adr+FLASH_PAGE_SIZE)) && (FLASHStatus == FLASH_COMPLETE))
	  {
		data = *((u32 *)&buf[byteCnt]);
	    FLASHStatus = FLASH_ProgramWord(adres,data );
	    adres = adres + 4;
		byteCnt += 4;
	  }
    }
  return FLASHStatus;
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void read_page(u32 adr, u8 * buf, u16 nSize)
{
  u16 cntCycles = 0;
  u32 * bl = (u32*)buf;
  while(cntCycles < nSize)  //  FLASH_PAGE_SIZE
    {
      *bl++ = *(vu32*)adr; 
      adr += 4;         
      cntCycles += 4; 
    }
}






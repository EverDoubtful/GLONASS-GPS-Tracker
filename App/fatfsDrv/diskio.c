
#include "stm32f10x.h" 
#include "ffconf.h" 
#include "diskio.h" 
#include "sdSDIODrv.h"
#include "rtc.h"
//SD_CardInfo SDCardInfo2; 



/*-------------------------------------------------------------------------- 

   Public Functions 

---------------------------------------------------------------------------*/ 


/*-----------------------------------------------------------------------*/ 
/* Initialize Disk Drive                                                 */ 
/*-----------------------------------------------------------------------*/ 

DSTATUS disk_initialize ( 
BYTE drv /* Physical drive number (0) */ 
) 
{ 
  NVIC_InitTypeDef NVIC_InitStructure; 

  SD_Init(); 
  //SD_GetCardInfo(&SDCardInfo2); 
  //SD_SelectDeselect((uint32_t) (SDCardInfo2.RCA << 16)); 
  //SD_EnableWideBusOperation(SDIO_BusWide_4b); 

  //SD_SetDeviceMode(SD_DMA_MODE); 
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
  NVIC_Init(&NVIC_InitStructure); 
   
return 0; 
} 


/*-----------------------------------------------------------------------*/ 
/* Get Disk Status                                                       */ 
/*-----------------------------------------------------------------------*/ 

DSTATUS disk_status ( 
BYTE drv /* Physical drive number (0) */ 
) 
{ 
return 0; 
} 


/*-----------------------------------------------------------------------*/ 
/* Read Sector(s)                                                        */ 
/*-----------------------------------------------------------------------*/ 

DRESULT disk_read ( 
	BYTE drv, /* Physical drive number (0) */ 
	BYTE *buff, /* Pointer to the data buffer to store read data */ 
	DWORD sector, /* Start sector number (LBA) */ 
	BYTE count /* Sector count (1..255) */ 
) 
{ 
uint32_t Memory_Offset; 
Memory_Offset = sector * 512; 

if(count == 1) 
	SD_ReadBlock( (uint8_t *)buff, Memory_Offset, 512); 
else
    SD_ReadMultiBlocks((uint8_t *)buff, Memory_Offset, 512, count);

return RES_OK; 
} 

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
#if 0
DRESULT disk_read (
	BYTE drv,		/* Physical drive number (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	DRESULT res;
	if ( drv || !count )
		return RES_PARERR;

	if ( count == 1 )
		res = (DRESULT)SD_SectorRead( sector, buff );
	else
		res = (DRESULT)SD_SectorsRead( sector, buff, count );

	if ( res == 0x00 )
		return RES_OK;
	return RES_ERROR;
}
#endif
/*-----------------------------------------------------------------------*/ 
/* Write Sector(s)                                                       */ 
/*-----------------------------------------------------------------------*/ 

DRESULT disk_write ( 
BYTE drv, /* Physical drive number (0) */ 
const BYTE *buff, /* Pointer to the data to be written */ 
DWORD sector, /* Start sector number (LBA) */ 
BYTE count /* Sector count (1..255) */ 
) 
{ 
uint32_t Memory_Offset; 
Memory_Offset = sector * 512; 
if(count == 1)
    SD_WriteBlock((uint8_t *)buff, Memory_Offset , 512); 
else
    SD_WriteMultiBlocks((uint8_t *)buff, Memory_Offset , 512, count); 

return RES_OK; 
} 

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	DRESULT res;
	if ( drv || !count )
		return RES_PARERR;

	if ( count == 1 )
		res = (DRESULT)SD_SectorWrite( sector, buff );
	else
		res = (DRESULT)SD_SectorsWrite( sector, buff, count );

	if ( res == 0 )
		return RES_OK;
	return RES_ERROR;
}
#endif
/*-----------------------------------------------------------------------*/ 
/* Get current time                                                      */ 
/*-----------------------------------------------------------------------*/ 
DWORD get_fattime () 
{ 
  //return getGLONASSTime();
  return getRealTime();
  /*
return ((2012UL-1980) << 25)       // Year = 2006 
| (6UL << 21)       // Month = Feb 
| (19UL << 16)       // Day = 9 
| (7U << 11)       // Hour = 22 
| (4U << 5)       // Min = 30 
| (53U >> 1)       // Sec = 0 
; 
  */
} 



/*-----------------------------------------------------------------------*/ 
/* Miscellaneous Functions                                               */ 
/*-----------------------------------------------------------------------*/ 
  
DRESULT disk_ioctl ( 
BYTE drv, // Physical drive number (0)  
BYTE ctrl, // Control code 
void *buff // Buffer to send/receive control data 
) 
{ 
DRESULT res = RES_OK; 
uint32_t status = SD_TRANSFER_BUSY; //SD_NO_TRANSFER; 
//uint32_t status = NAND_READY; 



switch (ctrl) { 
case CTRL_SYNC : /// Make sure that no pending write process 
status = SD_GetTransferState(); 
if (status == SD_TRANSFER_BUSY) 
//status = FSMC_NAND_GetStatus(); 
//if (status == NAND_READY) 
{res = RES_OK;} 
else{res = RES_ERROR;} 
break; 

case GET_SECTOR_COUNT :   // Get number of sectors on the disk (DWORD) 
*(DWORD*)buff = 131072; // 4*1024*32 = 131072 
res = RES_OK; 
break; 

case GET_SECTOR_SIZE :   // Get R/W sector size (WORD)  
*(WORD*)buff = 512; 
res = RES_OK; 
break; 

case GET_BLOCK_SIZE :     // Get erase block size in unit of sector (DWORD) 
*(DWORD*)buff = 32; 
res = RES_OK; 
  } 
   
return res; 
}

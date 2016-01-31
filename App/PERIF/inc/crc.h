/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : .h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Header for spi_flash.c file.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CRC_H
#define __CRC_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
u32 calcBlockCont(u8 * buf, u32 size);
u32 calcBlock(u8 * buf, u32 size);
u32 CalcCRC32(u8 *buffer, u32 size);
u32 CalcCRC32Soft(u8 *buffer, u32 size);
u32 CRC32WideFast(u32 Crc, u32 Size, u8 *buf);
#ifdef __cplusplus
}
#endif
#endif /* __CRC_H */

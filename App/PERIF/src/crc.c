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
#include "crc.h"
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u32 calcBlockCont(u8 * buf, u32 size)
{
  vu32 CRCValue = 0;
  u32 cnt,i;
  static u8 fAlrReset = 0;
  cnt = size >> 2;
  if(!fAlrReset)	//debugga
   {
     CRC_ResetDR();
	 fAlrReset = 1;
   }
  while(cnt--)
    {
         CRCValue = ( buf[3] ) | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
         //CRCValue = *(u32 *)buf;
         CRCValue = CRC_CalcCRC(CRCValue);
         buf += 4;
    }
  i = size & 3;
  if(i)
    {
	  if(i == 3)
	    {
		  CRCValue = ( 0 ) | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
		}
	  else if(i == 2)
	    {
		  CRCValue = ( 0 ) | (0 << 8) | (buf[1] << 16) | (buf[0] << 24);
		}
	  else if(i == 1)
	    {
		  CRCValue = ( 0 ) | (0 << 8) | (0 << 16) | (buf[0] << 24);
		}
	  CRCValue = CRC_CalcCRC(CRCValue);
    }
  return  CRCValue;
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u32 calcBlock(u8 * buf, u32 size)
{
  vu32 CRCValue = 0;
  u32 cnt,i;
  cnt = size >> 2;
  CRC_ResetDR();
  while(cnt--)
    {
         CRCValue = ( buf[3] ) | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
         //CRCValue = *(u32 *)buf;
         CRCValue = CRC_CalcCRC(CRCValue);
         buf += 4;
    }
  i = size & 3;
  if(i)
    {
	  if(i == 3)
	    {
		  CRCValue = ( 0 ) | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
		}
	  else if(i == 2)
	    {
		  CRCValue = ( 0 ) | (0 << 8) | (buf[1] << 16) | (buf[0] << 24);
		}
	  else if(i == 1)
	    {
		  CRCValue = ( 0 ) | (0 << 8) | (0 << 16) | (buf[0] << 24);
		}
	  CRCValue = CRC_CalcCRC(CRCValue);
    }
  return  CRCValue;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

u32 CalcCRC32(u8 *buffer, u32 size)
{
  u32 i, j;
  u32 ui32;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE);
  CRC->CR=1;
  i = size >> 2;
  while(i--)
  {
    ui32=*((u32 *)buffer);

	//ui32 = ( buffer[3] ) | (buffer[2] << 8) | (buffer[1] << 16) | (buffer[0] << 24);
    buffer += 4;
    ui32=__RBIT(ui32);//reverse the bit order of input data
    CRC->DR=ui32;
  }
  ui32=CRC->DR;
  ui32=__RBIT(ui32);//reverse the bit order of output data
  i = size & 3;
  while(i--)
  {
    ui32 ^= (u32)*buffer++;
	//ui32 =__RBIT(ui32);//reverse the bit order of output data debug
    for(j=0; j<8; j++)
      if (ui32 & 1)
        ui32 = (ui32 >> 1) ^ 0xEDB88320;
      else
        ui32 >>= 1;
  }
  //ui32=__RBIT(ui32);//reverse the bit order of output data debug
  ui32^=0xffffffff;//xor with 0xffffffff
  //ui32=__RBIT(ui32);//reverse the bit order of output data debug
  return ui32;//now the output is compatible with windows/winzip/winrar
}
/******************* (C)  *****END OF FILE****/
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

u32 CalcCRC32Soft(u8 *buffer, u32 size)
{
  u32 j;
  u32 ui32 = 0xffffffff;
  while(size--)
  {
    ui32 ^= (u32)*buffer++;
    for(j=0; j<8; j++)
      if (ui32 & 1)
        ui32 = (ui32 >> 1) ^ 0xEDB88320;
      else
        ui32 >>= 1;
  }
  //ui32=__RBIT(ui32);//reverse the bit order of output data debug
  ui32^=0xffffffff;//xor with 0xffffffff
  ui32=__RBIT(ui32);//reverse the bit order of output data debug
  return ui32;//now the output is compatible with windows/winzip/winrar
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

u32 CRC32WideFast(u32 Crc, u32 Size, u8 *buf)
{
    
    static const u32 CrcTable[16] = { // Nibble lookup table for 0x04C11DB7 polynomial
        0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
        0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD };
    u8  i = Size % 4;
    Size = Size >> 2; // /4
    
    while(Size--)
      {

        

        //Crc = Crc ^ *((u32 *)buf); // Apply all 32-bits
        Crc = Crc ^ (( buf[3] ) | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24));
        buf += 4;
        
        // Process 32-bits, 4 at a time, or 8 rounds
        
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
     }
  if(i)
    {
	  if(i == 3)
	    {
		  Crc = Crc ^( ( 0 ) | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24));
		}
	  else if(i == 2)
	    {
		  Crc = Crc ^( ( 0 ) | (0 << 8) | (buf[1] << 16) | (buf[0] << 24));
		}
	  else if(i == 1)
	    {
		  Crc = Crc ^( ( 0 ) | (0 << 8) | (0 << 16) | (buf[0] << 24));
		}
        // Process 32-bits, 4 at a time, or 8 rounds
        
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
        Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
	  
    }
     
    //value = ((Crc & 0xff) << 24) | ((Crc & 0xff00) << 8)|((Crc & 0xff0000) >> 8)|((Crc & 0xff000000) >> 24);
    
    return(Crc);
}














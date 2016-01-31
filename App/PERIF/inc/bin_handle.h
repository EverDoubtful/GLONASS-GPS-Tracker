/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : .h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Header for spi_flash.c file.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BIN_HANDLE_H
#define __BIN_HANDLE_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define  SIZE_EXT_BIN     128

/* COMMANDS */
#define CMD_ALL_INFO      1  //Packets
#define CMD_WRITE_CONFIG  2 
#define CMD_READ_CONFIG   3
#define CMD_ACCEL         4
#define CMD_DEBUG         5
#define CMD_PSW_CHECK     6
#define CMD_TU            7
#define CMD_QUITTANCE     8
#define CMD_GET_BALANCE   9

          /*Bootloader cmd*/
#define CMD_BL_ENTER         10
#define CMD_BL_PORTION       11
#define CMD_BL_CHECK_CRC     12
#define CMD_BL_RESTART       13
#define CMD_GET_VERSION      14
		  /* GPS */
#define CMD_GET_GPS_PACKET   15
          /* ADC */
#define CMD_SET_CALIBR_KOEF	 16
        /* CUSD get my own phone number*/
#define CMD_GET_PHONE_NUMBER 17

#define CMD_GET_RTC_TIME     18
#define CMD_MK_ZERO_MILEAGE  19  
#define CMD_CANCEL_ALARM     20
/*Bootloader cmd (continue)*/
#define CMD_BL_ADD_PAGE      21
/*END OF COMMANDS*/

#define PILOT_BYTE        0xFE  /* For USB */
#define MIN_BYTES_USB     9   /* pilot + cmd +aux + len(2) + crc(4)*/
#define MIN_BYTES_GSM     12   /* blockID(4) + cmd + aux + len(2) + crc(4)*/

/*AUX BITS*/
#define AUX_SELF_INTV 0
#define AUX_REPLY     1

#define AUX_OPENED    0
#define AUX_CYPHER    1

#define AUX_SHORT     0  //usual(short)  or long packet
#define AUX_LONG      1

//#define AUX_REALTIME  0
//#define AUX_HISTORY   1

/*SRC INTRF BITS */
#define INTRF_GSM     1
#define INTRF_USB     2
#define INTRF_RS_485  3
#define INTRF_SPI     4
#define INTRF_I2C     5
///* QUERY OR REPLY*/
//#define QUERY         0
//#define REPLY         1

//sizeof(wrap.cmd) + sizeof(wrap.aux) + sizeof(wrap.len) + sizeof(wrap.crc)
#define  SIZE_WRAP_PIL     1
#define  SIZE_WRAP_CMD     1
#define  SIZE_WRAP_AUX     1
#define  SIZE_WRAP_LEN     2
#define  SIZE_WRAP_CRC     4
#define  SIZE_WRAP_BLOCKID 4
#define  SIZE_HEADER_PACKET  SIZE_WRAP_PIL + SIZE_WRAP_CMD + SIZE_WRAP_AUX + SIZE_WRAP_LEN

__packed typedef struct _aux_info
{
  u8  reply    : 1;
  u8  cypher   : 1;
  u8  longPack : 1;
  u8  intrf    : 5;
}aux_info;

__packed typedef struct _cmd_info
{
  u8  cmd;
  aux_info aux;
  u16 len;
  u32 crc;
}cmd_info;



/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
u16  stuffData(u8 * buf, u16 nSize);
u16  unstuffData(u8 * bufSrc, u16 nSize);
u16  prepareForSend(u8 * bufSrc, u16 nSize, u8 flags, u8 cmd, u8 nrLong);
u16  prepareForSendUSB(u8 * bufSrc, u16 nSize, u8 bitLong);
u16  getCmd(u8 * buf);
u16  getCmdUSB(u8 * buf);

u16  genInfo( u8 *buf, u8 flag);

u16 debScan(u8 *buf);

#ifdef __cplusplus
}
#endif
#endif /* __BIN_HANDLE_H */

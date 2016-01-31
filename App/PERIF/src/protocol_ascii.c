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
#include <nmea/nmea.h>
#include <stdio.h>
#include "dio_led.h"
#include "adc.h"
#include "gps.h"
#include "bkp.h"
#include "protocol_ascii.h"
#include "hand_conf.h"
#include <string.h>
#include "hw_config.h"

GPRMCInfo gprmcInfo;
GNGSAInfo gngsaInfo;
MLGInfo   mlgInfo;
PGIOInfo  pgioInfo;

extern nmeaINFO info;
extern u32      mileage;
extern configSet config;
#if defined (VER_3)
extern float  koefVPower;
#endif
u16 cnt_tx;   //should transform from global to local var(later)

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

u16 gprmcPassed(u16 len, u8 * buf_tx)
{
    u8 *buf_tx_ptr,crc_code;
	buf_tx_ptr = &buf_tx[len];
	gprmcInfo.longitude = info.lon;
	gprmcInfo.latitude = info.lat;
	gprmcInfo.longLetter = (info.lon > 0)?'E':'W';
	gprmcInfo.latLetter = (info.lat > 0)?'N':'S';
	gprmcInfo.stateValidGPS	= (info.sig == NMEA_SIG_BAD)?'V':'A';
	gprmcInfo.speedMiles = info.speed / 1.62;
	gprmcInfo.direction	 = info.direction;	   //testing 231.7

	int2char((char * )gprmcInfo.timeDigits,info.utc.hour,2,10);
	int2char((char * )&gprmcInfo.timeDigits[2],info.utc.min,2,10);
	int2char((char * )&gprmcInfo.timeDigits[4],info.utc.sec,2,10);

	int2char((char * )gprmcInfo.dateDigits,info.utc.day,2,10);
	int2char((char * )&gprmcInfo.dateDigits[2],(info.utc.mon+1),2,10);  //+1 - for debugging
	int2char((char * )&gprmcInfo.dateDigits[4],info.utc.year,2,10);

	//$GPRMC,092611,A,5548.0474,N,04906.4238,E,00.08,215.1,291111,,*19
	*buf_tx_ptr++ = USD_CODE;
	//bufCopy(GPRMC_STRING,buf_tx_ptr,sizeof(GPRMC_STRING));
	memcpy(buf_tx_ptr,GPRMC_STRING,sizeof(GPRMC_STRING));
	buf_tx_ptr += sizeof(GPRMC_STRING)-1;
	*buf_tx_ptr++ = COMMA_CODE;

	//bufCopy(gprmcInfo.timeDigits,buf_tx_ptr,sizeof(gprmcInfo.timeDigits));
	memcpy(buf_tx_ptr,gprmcInfo.timeDigits,sizeof(gprmcInfo.timeDigits));
	buf_tx_ptr += sizeof(gprmcInfo.timeDigits);
	*buf_tx_ptr++ = '.';
	*buf_tx_ptr++ = '0';
	*buf_tx_ptr++ = '0';

    *buf_tx_ptr++ = COMMA_CODE;
    *buf_tx_ptr++ = gprmcInfo.stateValidGPS;
    *buf_tx_ptr++ = COMMA_CODE;

	cnt_tx = snprintf((char * )buf_tx_ptr,10,"%4.4f",gprmcInfo.latitude);
	buf_tx_ptr += cnt_tx; 
    *buf_tx_ptr++ = COMMA_CODE;
    *buf_tx_ptr++ = gprmcInfo.latLetter;
    *buf_tx_ptr++ = COMMA_CODE;

	if((gprmcInfo.longitude < 99999.9999) && (gprmcInfo.stateValidGPS == 'A'))
	   *buf_tx_ptr++ = '0';
	cnt_tx = snprintf((char * )buf_tx_ptr,11,"%5.4f",gprmcInfo.longitude);
	buf_tx_ptr += cnt_tx; 
    *buf_tx_ptr++ = COMMA_CODE;
    *buf_tx_ptr++ = gprmcInfo.longLetter;
    *buf_tx_ptr++ = COMMA_CODE;

	cnt_tx = snprintf((char * )buf_tx_ptr,7,"%3.2f",gprmcInfo.speedMiles);
	buf_tx_ptr += cnt_tx; 
    *buf_tx_ptr++ = COMMA_CODE;
	cnt_tx = snprintf((char * )buf_tx_ptr,6,"%3.1f",gprmcInfo.direction);
	buf_tx_ptr += cnt_tx; 

    *buf_tx_ptr++ = COMMA_CODE;
	//bufCopy(gprmcInfo.dateDigits,buf_tx_ptr,sizeof(gprmcInfo.dateDigits));
	memcpy(buf_tx_ptr,gprmcInfo.dateDigits,sizeof(gprmcInfo.dateDigits));
	buf_tx_ptr += sizeof(gprmcInfo.dateDigits);
    *buf_tx_ptr++ = COMMA_CODE;
    *buf_tx_ptr++ = COMMA_CODE;
    *buf_tx_ptr++ = COMMA_CODE;
    *buf_tx_ptr++ = 'A';

    *buf_tx_ptr++ = ASTERISK_CODE;
    //CRC calculation
	*buf_tx_ptr++ = 'E';
    *buf_tx_ptr++ = 'E';
    *buf_tx_ptr++ = CR_CODE;
    *buf_tx_ptr = NL_CODE;
	cnt_tx = findLen((const u8*)&buf_tx[len+1],MAX_SIZE_PACKET_GPRS_SIM900 ); //GPS_SENDING_BUFFER_SIZE
	crc_code = crcNavi((const u8*)&buf_tx[len+1],cnt_tx);
	int2char((char * )&buf_tx[cnt_tx+len+2],(crc_code >> 4) & 0x0F,1,16);
	int2char((char * )&buf_tx[cnt_tx+len+3],(crc_code & 0x0F),1,16);
	cnt_tx += 6;
	return cnt_tx;
}
//-------------------------------------------------------------------------------------------
u16 gngsaPassed(u16 len, u8 * buf_tx)
{
    u8 *buf_tx_ptr,crc_code;
	buf_tx_ptr = &buf_tx[len];
	//$GNGSA,04,02*5A
   	int2char((char * )gngsaInfo.GPSNumber,info.satinfo.inuseGPS,2,10);
   	int2char((char * )gngsaInfo.GLNNumber,info.satinfo.inuseGLN,2,10);

	*buf_tx_ptr++ = USD_CODE;
	//bufCopy(GNGSA_STRING,buf_tx_ptr,sizeof(GNGSA_STRING));
	memcpy(buf_tx_ptr,GNGSA_STRING,sizeof(GNGSA_STRING));
	buf_tx_ptr += sizeof(GNGSA_STRING)-1;
	*buf_tx_ptr++ = COMMA_CODE;
	//bufCopy(gngsaInfo.GLNNumber,buf_tx_ptr,sizeof(gngsaInfo.GLNNumber));
	memcpy(buf_tx_ptr,gngsaInfo.GLNNumber,sizeof(gngsaInfo.GLNNumber));
	buf_tx_ptr += sizeof(gngsaInfo.GLNNumber);
    *buf_tx_ptr++ = COMMA_CODE;

	//bufCopy(gngsaInfo.GPSNumber,buf_tx_ptr,sizeof(gngsaInfo.GPSNumber));
	memcpy(buf_tx_ptr,gngsaInfo.GPSNumber,sizeof(gngsaInfo.GPSNumber));
	buf_tx_ptr += sizeof(gngsaInfo.GPSNumber);

    *buf_tx_ptr++ = ASTERISK_CODE;
    //CRC calculation
	*buf_tx_ptr++ = 'E';
    *buf_tx_ptr++ = 'E';
    *buf_tx_ptr++ = CR_CODE;
    *buf_tx_ptr = NL_CODE;
	cnt_tx = findLen((const u8*)&buf_tx[len+1],MAX_SIZE_PACKET_GPRS_SIM900);
	crc_code = crcNavi((const u8*)&buf_tx[len+1],cnt_tx);
	int2char((char * )&buf_tx[cnt_tx+len+2],(crc_code >> 4) & 0x0F,1,16);
	int2char((char * )&buf_tx[cnt_tx+len+3],(crc_code & 0x0F),1,16);
	cnt_tx += 6;
	return cnt_tx;

}
//-------------------------------------------------------------------------------------------

u16 mlgPassed(u16 len, u8 * buf_tx)
{
    u8 i = 0,crc_code;
    u8 *buf_tx_ptr;

	buf_tx_ptr = &buf_tx[len];
	mlgInfo.speedKM = info.speed;    //info.speed;
	*buf_tx_ptr++ = USD_CODE;
	//bufCopy(MLG_STRING,buf_tx_ptr,sizeof(MLG_STRING));
	memcpy(buf_tx_ptr,MLG_STRING, sizeof(MLG_STRING));
	buf_tx_ptr += sizeof(MLG_STRING)-1;
	*buf_tx_ptr++ = COMMA_CODE;

    mlgInfo.mileage = mileage; 
	i = getLenNum(mlgInfo.mileage);
	int2char((char * )buf_tx_ptr,mlgInfo.mileage,i,10);
	buf_tx_ptr += i;
	*buf_tx_ptr++ = COMMA_CODE;

	i = 3;
	if(mlgInfo.speedKM < 10) i = 1;
	else if(mlgInfo.speedKM < 100) i = 2;
	int2char((char * )buf_tx_ptr,mlgInfo.speedKM,i,10);
	buf_tx_ptr += i;
	 
    *buf_tx_ptr++ = ASTERISK_CODE;
    //CRC calculation
	*buf_tx_ptr++ = 'E';
    *buf_tx_ptr++ = 'E';
    *buf_tx_ptr++ = CR_CODE;
    //*buf_tx_ptr = NL_CODE;
    *buf_tx_ptr++ = NL_CODE;	 // added ++ for finish 0
    //*buf_tx_ptr = 0;	         // added finish 0

	cnt_tx = findLen((const u8*)&buf_tx[len+1],MAX_SIZE_PACKET_GPRS_SIM900);
	crc_code = crcNavi((const u8*)&buf_tx[len+1],cnt_tx);
	int2char((char * )&buf_tx[cnt_tx+len+2],(crc_code >> 4) & 0x0F,1,16);
	int2char((char * )&buf_tx[cnt_tx+len+3],(crc_code & 0x0F),1,16);
	cnt_tx += 6;
	return cnt_tx;

}
//-------------------------------------------------------------------------------------------

u16 pgioPassed(u16 len, u8 * buf_tx)
{
    u8 i,k,crc_code;
    u8 *buf_tx_ptr;
	buf_tx_ptr = &buf_tx[len];
	pgioInfo.uID = config.idBlock;
	pgioInfo.flagStart = 'C';
	pgioInfo.orderMsgNumber	+= 1;
	BKPWriteReg(BKP_DR_MSG_NUMBER1,pgioInfo.orderMsgNumber);
	BKPWriteReg(BKP_DR_MSG_NUMBER2,pgioInfo.orderMsgNumber >> 16);
	 
	pgioInfo.stateInput		= readStateInput();  //stub
	pgioInfo.stateOutputs	= readStateOutput();  //stub
	pgioInfo.stateADC[0]    = ain_read(0);
	pgioInfo.stateADC[1]    = ain_read(1);
	pgioInfo.stateADC[2]    = ain_read(2);
	pgioInfo.stateADC[3]    = ain_read(3);
	pgioInfo.stateThermo[0]	= 0;
	pgioInfo.stateThermo[1]	= 0;
	pgioInfo.stateThermo[2]	= 0;
	pgioInfo.stateThermo[3]	= 0;
	pgioInfo.stateThermo[4]	= 0;
	pgioInfo.stateThermo[5]	= 0;
	pgioInfo.stateThermo[6]	= 0;
	pgioInfo.stateThermo[7]	= 0;
	pgioInfo.stateFuel[0]	= 0;
	pgioInfo.stateFuel[1]	= 0;

	*buf_tx_ptr++ = USD_CODE;
	//bufCopy(PGIO_STRING,buf_tx_ptr,sizeof(PGIO_STRING));
	memcpy(buf_tx_ptr,PGIO_STRING, sizeof(PGIO_STRING));
	buf_tx_ptr += sizeof(PGIO_STRING)-1;
	*buf_tx_ptr++ = COMMA_CODE;

	int2char((char * )buf_tx_ptr,pgioInfo.uID,6,10);
	buf_tx_ptr += 6;  //should exact
	*buf_tx_ptr++ = COMMA_CODE;
	*buf_tx_ptr++ = pgioInfo.flagStart;
	*buf_tx_ptr++ = COMMA_CODE;
	int2char((char * )buf_tx_ptr,pgioInfo.orderMsgNumber,8,10);
    buf_tx_ptr += 8;  //should exact
	*buf_tx_ptr++ = COMMA_CODE;
	//*buf_tx_ptr++ = pgioInfo.stateInput;
	int2char((char * )buf_tx_ptr,pgioInfo.stateInput,2,16);
	buf_tx_ptr += 2;
	*buf_tx_ptr++ = COMMA_CODE;
	//*buf_tx_ptr++ = pgioInfo.stateOutputs;
	int2char((char * )buf_tx_ptr,pgioInfo.stateOutputs,2,16);
	buf_tx_ptr += 2;
	*buf_tx_ptr++ = COMMA_CODE;

	cnt_tx = snprintf((char * )buf_tx_ptr,5,"%2.2f",((double)pgioInfo.stateADC[0])*koefADC);
	buf_tx_ptr += cnt_tx; //cnt_tx; 
	*buf_tx_ptr++ = COMMA_CODE;
	cnt_tx = snprintf((char * )buf_tx_ptr,5,"%2.2f",((double)pgioInfo.stateADC[1])*koefADC);
	buf_tx_ptr += cnt_tx; //cnt_tx; 
	*buf_tx_ptr++ = COMMA_CODE;

	cnt_tx = snprintf((char * )buf_tx_ptr,5,"%2.1f",((double)pgioInfo.stateADC[2]) * koefVPower);
	buf_tx_ptr += cnt_tx; //cnt_tx; 
	*buf_tx_ptr++ = COMMA_CODE;
	cnt_tx = snprintf((char * )buf_tx_ptr,5,"%2.1f",((double)pgioInfo.stateADC[3]) * koefVBAT);
	buf_tx_ptr += cnt_tx; //cnt_tx; 
	*buf_tx_ptr++ = COMMA_CODE;

	for(k = 0; k < 8; k++)
	  {
			i = getLenNum(pgioInfo.stateThermo[k]);
			int2char((char * )buf_tx_ptr,pgioInfo.stateThermo[k],i,10);
		    buf_tx_ptr += i;  //should exact
			*buf_tx_ptr++ = COMMA_CODE;
	  }

	i = getLenNum(pgioInfo.stateFuel[0]);
	int2char((char * )buf_tx_ptr,pgioInfo.stateFuel[0],i,10);
    buf_tx_ptr += i;  //should exact
	*buf_tx_ptr++ = COMMA_CODE;
	i = getLenNum(pgioInfo.stateFuel[1]);
	int2char((char * )buf_tx_ptr,pgioInfo.stateFuel[1],i,10);
    buf_tx_ptr += i;  //should exact
    *buf_tx_ptr++ = ASTERISK_CODE;
    //CRC calculation
	*buf_tx_ptr++ = 'E';
    *buf_tx_ptr++ = 'E';
    *buf_tx_ptr++ = CR_CODE;
    *buf_tx_ptr = NL_CODE;
	cnt_tx = findLen((const u8*)&buf_tx[len+1],MAX_SIZE_PACKET_GPRS_SIM900);
	crc_code = crcNavi((const u8*)&buf_tx[len+1],cnt_tx);
	int2char((char * )&buf_tx[cnt_tx+len+2],(crc_code >> 4) & 0x0F,1,16);
	int2char((char * )&buf_tx[cnt_tx+len+3],(crc_code & 0x0F),1,16);
	cnt_tx += 6;

	//crc_ending(len);

	return cnt_tx;

}
//-------------------------------------------------------------------------------------------
u8 findLen(const u8 *buf, u16 len)	 //should transfer to gps.c
{
   u8 res = 0;
   while(len--)
     {
      if( (*buf == ASTERISK_CODE) && (*(buf+3) == CR_CODE) && (*(buf+4) == NL_CODE))
	     return (res);
	  res++;
	  buf++;
	 }
   return 0;
}

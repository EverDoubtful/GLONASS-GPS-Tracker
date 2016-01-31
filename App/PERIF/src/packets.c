/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : gps.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : This file provides a set of functions needed to manage the
*                      communication between SPI peripheral and SPI M25P64 FLASH.
*/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <string.h>
#include "stdlib.h"
#include "gps.h"
#include "fifoPackets.h"
#include "packets.h"
#include "packetsInner.h"
#include "sdcardUser.h"
#include "rtc.h"
#include "hand_conf.h"
#include "protocol_ascii.h"
#include "protocol_bin.h"
#include "hw_config.h"
#include "bin_handle.h"
#include "dio_led.h"
#include "crc.h"


u32  gpsTimerPacketValue;
u8   gpsFlagTimerReady;

node_struct nodesNavi[FIFO_PACKET_NAVI_SIZE];
fifo_packet fifoNaviPack;

extern u8       flagNoGSM;
extern RTC_t    rtc;
	   //u8       bufRxOperGSM[MAX_SIZE_PACKET_GPRS_SIM900];
	   u8       bufRxOperGSM[SIZE_PACKET_GPRS_OPER];
	   
extern nmeaINFO info;
extern u16      anglePrev;
extern u32      mileage;



/*Receive buffer of packets from  GSM*/
node_struct   nodesRcvPack[FIFO_PACKET_RECV_SIZE];
fifo_packet   fifoRecBufPack;
//u8 bufRcvPacket[MAX_SIZE_PACKET_GPRS_SIM900];
/*Receive buffer of bytes from USB*/
u8 bufRcvUSB[USB_RING_RX_DATA_SIZE];   /*operational buffer of USB*/
volatile u32 timerUSBPacketCheckValue;
volatile u8  usbFlagTimerReady;
volatile u32 timerGSMPacketCheckValue;
volatile u8  gsmFlagTimerReady;

#define    GLOBAL_RESET_24HOUR_SEC       86400000 
volatile u32 timerGLOBAL_RESET_24HOURValue;


volatile u32 timerUSBRcvNewPacketValue;
volatile u8  flagUSBNewPacket;

static u8 flagNAVIAppeared = 0;
static u8 flagGSMAppeared = 0;
static u8 flagVelLimitHandled = 0;


extern bin_info  binInfo;
extern configSet config;
extern t_innerState innerState;

u8 flagEnablePutPacket = 0;

u16 cap;  //tempora
// ------------------------------------------------------------------------------
u8 getInfo(void)
{
  return cap;
}
// ------------------------------------------------------------------------------
void packetsIni(void)
{
	fifoPacketInit(&fifoNaviPack,nodesNavi,sizeof(nodesNavi)/sizeof(nodesNavi[0]));
	fifoPacketInit(&fifoRecBufPack,nodesRcvPack,sizeof(nodesRcvPack)/sizeof(nodesRcvPack[0]));

	setTimerGPSPacket(ONE_SEC * 5);
    setTimerPackets(MS_100);
    setTimerPacketsGSM(MS_100);

	binInfo.fos.systemStart = 1;
	timerGLOBAL_RESET_24HOURValue = GLOBAL_RESET_24HOUR_SEC;
}
// ------------------------------------------------------------------------------

void sayHello(void)
{
  flagEnablePutPacket = 1;
}
// ------------------------------------------------------------------------------
void naviPacketHandle(void)
{

   u16 lenData;
   
   static u32 cntEnablePutPacket = 0;
   //u16 cap;
   static u16 localCNT = 0;
   static u8 localCNT2 = 0;
//   u8 sign;
   static u8 dayCurr = 0;
   //static u16 metersCounter = 0;
   static u32 mileagePrevLoc = 0;
   u16  debugga;
   u8 i = 0;
   u8 dumb;
   //static u8 debFl = 0;  //delete later
   //u16    teLen;
   if(gpsFlagTimerReady) 
     {
	    gpsFlagTimerReady = 0;
	    setTimerGPSPacket(ONE_SEC/2);	   //TIME_INTRV_GPS_QUERY
 	    
		cap = fifoPacketCount(&fifoNaviPack);

		/* Analyse if packets > 0.75 of whole buffer and there is no GSM - we put packets in file
		analyse every second  */
		if( (cap > (FIFO_PACKET_NAVI_SIZE * 0.75)) && (flagNoGSM) )	   // 
		  {
		     memset(bufRxOperGSM,0,sizeof(bufRxOperGSM));
             lenData = fifoPacketGet(&fifoNaviPack, bufRxOperGSM , &dumb);
			 //debFl ^= 1;
			 //if(debFl)
			 //  lenData += 5;
			 saveSDInfo(bufRxOperGSM, lenData, SD_NOTSEND, SD_TYPE_MSG_GSM);
		  }
		/* Analyse if packets < 0.25 of whole buffer and there is GSM - we get packets from file
		analyse every second  */ //should set flag of time setting
	    //if( /*(cap < (FIFO_PACKET_NAVI_SIZE * 0.25)) && */ (!flagNoGSM) )	   // 
	    if( ( !flagNoGSM ) && (!innerState.bootloaderStarted) )	    
		  {
		    lenData = 1;
		    while((i < (FIFO_PACKET_NAVI_SIZE - cap-2)) && (lenData))
			 {
			     i++;
			     memset(bufRxOperGSM,0,sizeof(bufRxOperGSM));
				 lenData = readSDInfo(bufRxOperGSM, sizeof(bufRxOperGSM));
				 debugga = lenData; 
				 if(debugga)
				   {
					   debugga = 0;
				   }
				 if(lenData)
				    fifoPacketPut(&fifoNaviPack,bufRxOperGSM, lenData, 0);
			 }
			   
		  }

		/* Make some records about GLONASS Appear/disappear */
		if(info.sig == NMEA_SIG_BAD)  /* No, we loose GPS */
		   {
		      if(flagNAVIAppeared)
				  {
			         flagNAVIAppeared = 0;
					 saveSDInfo((u8 *)"GPS LOOSE ",strlen((const char *)"GPS LOOSE "), SD_SENDED, SD_TYPE_MSG_LOG);
					 //saveSDInfo((u8 *)readRTCTime(&rtc),strlen((const char *)readRTCTime(&rtc)), SD_SENDED, SD_TYPE_MSG_LOG);
			      }
			  binInfo.fos.naviLost = 1;
	       }
	    else 					      /* Yes, we got GPS and ...*/
		   {
		     binInfo.fos.naviLost = 0;
		     if(config.flagTrackControl)  /*Advanced track control is on*/
			   {
			      
			      if(!flagNAVIAppeared)
					  {
				         flagNAVIAppeared = 1;
						 saveSDInfo((u8 *)"GPS FIND ",strlen((const char *)"GPS FIND "), SD_SENDED, SD_TYPE_MSG_LOG);
				      }
	
				  /* Handle speed limit */
				  if(info.speed > config.velocityLimit)
				     {
					   if(!flagVelLimitHandled)
						  {
						    /* Here we make added packets*/ 
							flagVelLimitHandled = 1;
							flagEnablePutPacket = 1;
							binInfo.fos.vehicleSpeedExceed = 1;
						  }
					 }
				   else if(info.speed < (config.velocityLimit - 3))
					    flagVelLimitHandled = 0;
				  /* Handle angle limit */
				  if((info.speed > 5) && (abs(anglePrev - info.direction) > config.angleLimit) )
				     {
						    /* Here we make added packets*/ 
							flagEnablePutPacket = 1;
							binInfo.fos.courseChanged = 1;
					 }
				   anglePrev = info.direction;
				  /* Handle distance limit */
				  if( (mileage - mileagePrevLoc) > config.distanceLimit ) 
				     {
				  			mileagePrevLoc = mileage;  //
						    /* Here we make added packets*/ 
							flagEnablePutPacket = 1;   //temporarily commented
							binInfo.fos.distPassed = 1;
					 }
				  
				}

		   }
		/* Make some records about GPRS Appear/disappear */
		if(flagNoGSM)  /* No, we loose GPRS */
		   {
		      if(flagGSMAppeared)
				  {
			         flagGSMAppeared = 0;
					 saveSDInfo((u8 *)"GPRS LOOSE ",strlen((const char *)"GPRS LOOSE "), SD_SENDED, SD_TYPE_MSG_LOG);
					 //saveSDInfo((u8 *)readRTCTime(&rtc),strlen((const char *)readRTCTime(&rtc)), SD_SENDED, SD_TYPE_MSG_LOG);
			      }
			  binInfo.fos.GPRSLost = 1;
		   }
	    else					      /* Yes, we got GPRS */
		   {
		      if(!flagGSMAppeared)
				  {
			         flagGSMAppeared = 1;
					 saveSDInfo((u8 *)"GPRS FIND ",strlen((const char *)"GPRS FIND "), SD_SENDED, SD_TYPE_MSG_LOG);
					 //saveSDInfo((u8 *)readRTCTime(&rtc),strlen((const char *)readRTCTime(&rtc)), SD_SENDED, SD_TYPE_MSG_LOG);
			      }
		   }
		
		
		/* If no GPS and there is GSM - we made packets */
		if(++localCNT2 >= 20)		 //every 10 sec
		  {
		    localCNT2 = 0; 
     		if(info.sig == NMEA_SIG_BAD)
			  {
			    if(!flagNoGSM)
			      {
					flagEnablePutPacket = 0;
					cntEnablePutPacket++;
					if(cntEnablePutPacket == 1)		  //10 sec
						flagEnablePutPacket = 1;
					else if(cntEnablePutPacket == 2)  //20
						flagEnablePutPacket = 1;
					else if(cntEnablePutPacket == 4)  //40
						flagEnablePutPacket = 1;
					else if(cntEnablePutPacket == 8)	//80
						flagEnablePutPacket = 1;
					else if(cntEnablePutPacket  == 16)	  //160
						flagEnablePutPacket = 1;
					else if((cntEnablePutPacket  % 30) == 0)	  //300
						flagEnablePutPacket = 1;
					//else if((cntEnablePutPacket % 55) == 0)	  //550
					//	flagEnablePutPacket = 1;
			      }
			   }
		   }
		//analyse in interval (5 sec)  (*2 - because localCnt happened every 0.5 s)
		if(++localCNT >= ((config.intervalSendingPower * 2)))
		  {

			localCNT = 0;
			 
			if(info.sig != NMEA_SIG_BAD)   /* Yes, we got GPS */
			  {
			    cntEnablePutPacket = 0;
			    flagEnablePutPacket = 1;
			  }
          }	//if(++localCNT >= ((config.intervalSendingPower * 2)))	
		
		if(flagEnablePutPacket)
			  {
			    flagEnablePutPacket = 0;
			    /* Here we make main stream of packets*/ 
				memset(bufRxOperGSM,0,sizeof(bufRxOperGSM));
		        lenData = prepareBinData(bufRxOperGSM, 1);	             /* initial collect of data*/

 			    //fifoPacketPut(&fifoNaviPack,bufRxOperGSM, lenData);
				/* Here we save all good(GPS Valid) packets*/
				//if(getGLONASSStateValid())
				  //{
					if(dayCurr != info.utc.hour) /* and correct time*/
					  {
					     dayCurr = info.utc.hour;
					     rtcCorrectTimeFromGLONASS(&rtc, &info);
					  }
				    saveSDInfo(bufRxOperGSM, lenData, SD_SENDED, SD_TYPE_MSG_GSM);
				  //}
				i = 0;
				if(innerState.bootloaderStarted)
				   i = 0x02;
			    lenData = prepareForSend(bufRxOperGSM,lenData, i, CMD_ALL_INFO, 0);   /* filling cmd,uid,len crc ...*/
			   }	 //if(flagEnablePutPacket)
		   
	 }
}
/*******************************************************************************/
/* 	Execute from GSM 								           */
/*******************************************************************************/
void rcvPacketHandle(void)
{
  u8 cap, dumb;
  //u8 sign;
  u16 lenData;
  //u16 index = 0;
  if(gsmFlagTimerReady)
    {
	  gsmFlagTimerReady = 0;
	  setTimerPacketsGSM(MS_10);
	  //todo : here i should make timer every 100ms
	  cap = fifoPacketCount(&fifoRecBufPack);
	  if(cap)
	   {
	     /*get raw binary packet*/
	     lenData = fifoPacketGet(&fifoRecBufPack, bufRxOperGSM , &dumb);
		 /*Handling of all kinds*/
	     /*Get cmd,id,len, check crc and execute*/
	      if( lenData >= MIN_BYTES_GSM )
	        {
	
	           if(
	            (( bufRxOperGSM[3] ) | (bufRxOperGSM[2] << 8) | (bufRxOperGSM[1] << 16) | (bufRxOperGSM[0] << 24) == config.idBlock) &&   
	            (*(u32*)&bufRxOperGSM[lenData-4] == calcBlock(bufRxOperGSM,lenData-4))
	             )  
	            {
	               /*Get cmd,id,len and execute*/
	               lenData = getCmd(bufRxOperGSM);
	            }
	        }
	   }
    }
}
/*******************************************************************************/
/* 	Execute from USB 								           */
/*******************************************************************************/
void rcvPacketHandleUSB(void)
{
  //u32 crc = 1; //temporarily
  u16  lenData;
  static u16 index = 0;
  if(usbFlagTimerReady)
    {
      usbFlagTimerReady = 0;
      setTimerPackets(MS_1 * 5);
	  if(flagUSBNewPacket)
	     {
		    flagUSBNewPacket = 0;
			index = 0;
		 }
       /*get raw binary packet*/
      lenData = USBReadWhole(&bufRcvUSB[index]);
	  //lenData = USBReadWhole(bufRcvUSB);	//debugga
	  index += lenData;
	  index %= sizeof(bufRcvUSB);
	  if(lenData)
	       setTimerUSBRcvNewPacket(ONE_SEC);

      if( lenData >= MIN_BYTES_USB )
        {
           if(
            (bufRcvUSB[0] == PILOT_BYTE) &&   
            (*(u32*)&bufRcvUSB[index-4] == calcBlock(bufRcvUSB,index-4))
             )  
            {
			   index = 0;
               /*Get cmd,id,len, check crc and execute*/
        	   lenData = getCmdUSB(bufRcvUSB);
               /* put handled packet into the send packet buffer*/
			   USBWriteLen(bufRcvUSB,lenData);
            }
        }
    }
}  
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleTimerPackets(void)
{
    if(timerUSBPacketCheckValue)
	  {
	   timerUSBPacketCheckValue--;
	   if(!timerUSBPacketCheckValue )
	       usbFlagTimerReady = 1; 
	  }
    if(timerGSMPacketCheckValue)
	  {
	   timerGSMPacketCheckValue--;
	   if(!timerGSMPacketCheckValue )
	       gsmFlagTimerReady = 1; 
	  }
    if(gpsTimerPacketValue)
	  {
	   gpsTimerPacketValue--;
	   if(!gpsTimerPacketValue )
	      gpsFlagTimerReady = 1;
	  }
    if(timerUSBRcvNewPacketValue)
	  {
	   timerUSBRcvNewPacketValue--;
	   if(!timerUSBRcvNewPacketValue )
	      flagUSBNewPacket = 1;
	  }
    if(timerGLOBAL_RESET_24HOURValue)
	  {
	   timerGLOBAL_RESET_24HOURValue--;
	   if(!timerGLOBAL_RESET_24HOURValue )
	     {
	      //innerState.needRestart = 1;
 		   __disable_irq();
	       // restart system
	       NVIC_SystemReset();
		 }
	  }
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerPackets(u32 val)
{
   	timerUSBPacketCheckValue = val;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerPacketsGSM(u32 val)
{
   	timerGSMPacketCheckValue = val;
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerUSBRcvNewPacket(u32 val)
{
   	timerUSBRcvNewPacketValue = val;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerGPSPacket(u32 val)
{
   	gpsTimerPacketValue = val;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/


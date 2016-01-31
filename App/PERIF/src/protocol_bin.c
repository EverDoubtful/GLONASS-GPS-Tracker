/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : gps.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : This file provides a set of functions needed to manage the
*                      communication between SPI peripheral and SPI M25P64 FLASH.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

//SHOULD DIVIDE hardware and software ports of GPS


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "gps.h"
#include <nmea/nmea.h>
#include <stdio.h>
#include <string.h>
#include "dio_led.h"
#include "adc.h"
#include "bkp.h"
#include "fifoPackets.h"
#include "hw_config.h"
#include "sdcardUser.h"
#include "rtc.h"
#include "stdlib.h"
#include "protocol_bin.h"

bin_info  binInfo;
//sat_info  satInfo;
extern nmeaINFO info;
extern u32      mileage;
extern char     cspnBuf[];   //16
#if defined (VER_3)
extern float  koefVPower;
#endif
extern volatile u16 gpsHndlCnt;  //deb

t_innerState innerState;

u32 timerValuePrBin;
volatile u8  flagPrBin = 1;
void setTimerPrBin(u32 val);

const u8 * statesBlock[] = {
							  "Modem ON",
							  "Modem OFF",
                              "Initialising",
                              "Data mode",
							  "Voice mode",
							  "Reinitialiasing",
							  "FTP Upgrading",
							  "USB Upgrading",
							  "GPRS Upgrading",
							  "Connected"

							};

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u16 quittance(void)
{

  innerState.fwUpdated = 0;
  innerState.alarm     = 0;

  return 0;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void executeDelayedCmd(void)
{
  if(flagPrBin)
    {
	  flagPrBin = 0;
	  setTimerPrBin(MS_100 * 1);
      if(innerState.goRestart)	
		{	  
		   __disable_irq();
	
	       // restart system
	       NVIC_SystemReset();
		}
	  if(innerState.needRestart) 
	    {
		   innerState.needRestart = 0;
		   innerState.goRestart = 1;
		   setTimerPrBin(MS_100 * 1);
		}
    }
}
/*******************************************************************************/
/* 	flag: 0 - do not get msg number  and do not increase	, 1 - yes	   */
/*******************************************************************************/

u16 prepareBinData(u8 * bufExt, u8 flag)
{
  u16 len = 0;
  float temp;
  u8 tempo;
  static u16 cntOpName = 0;
  u8 *buf_tx_ptr = bufExt;

  /*  Constant part */
  if(flag)	   //query by GSM
     {
	  binInfo.orderMsgNumber += 1;
	  *(u32*)buf_tx_ptr = binInfo.orderMsgNumber;
	  buf_tx_ptr += sizeof(binInfo.orderMsgNumber);
	  len += sizeof(binInfo.orderMsgNumber);
	  BKPWriteReg(BKP_DR_MSG_NUMBER1,binInfo.orderMsgNumber);
	  BKPWriteReg(BKP_DR_MSG_NUMBER2,binInfo.orderMsgNumber >> 16);
	 }
  /* flags of system */
  
  //binInfo.fos.distPassed = 1;
  //binInfo.fos.digInputChanged = 0;
  //binInfo.fos.anInputChanged = 0;
  //binInfo.fos.naviLost = 0;
  //binInfo.fos.GPRSLost = 0;
  binInfo.fos.cellNameChanged = innerState.flCellNameChanged;
  innerState.flCellNameChanged = 0;
  //binInfo.fos.vehicleStop = 0;
  //binInfo.fos.vehicleStart = 0;
  //binInfo.fos.vehicleSpeedExceed = 0;
  binInfo.fos.extPowerOff = 0;
  binInfo.fos.lowBattery = 0;
  //binInfo.fos.systemStart = 0;
  binInfo.fos.axisXChange = 0;
  binInfo.fos.axisYChange = 0;
  binInfo.fos.axisZChange = 0;
  binInfo.fos.tempGaugeExceed = 0;
  //binInfo.fos.courseChanged = 0;
  binInfo.fos.activeSIMCard = innerState.activeSIMCard;
  binInfo.fos.sysHealth = 1;
  binInfo.fos.SDPresence = !innerState.flagSDDisappeared;
  *(u32*)buf_tx_ptr = *(u32*)&binInfo.fos;
  buf_tx_ptr += sizeof(binInfo.fos);
  len +=  sizeof(binInfo.fos);
  *(u32*)&binInfo.fos = 0;

  /* bitsOfPresence */
  *(u32*)&binInfo.bop = 0;

  binInfo.bop.digInput = 1;
  if(innerState.alarm)
     binInfo.bop.alarm = 1;
  binInfo.bop.digOutput = 1;
  binInfo.bop.stADC0 = 1;
  binInfo.bop.stADC1 = 1;
  binInfo.bop.stADC2 = 1;
  binInfo.bop.stADC3 = 1;
#if defined (VER_3)
  binInfo.bop.stADC4 = 0;
  binInfo.bop.stADC5 = 0;
  binInfo.bop.stADC6 = 0;
  binInfo.bop.stADC7 = 0;
#endif
  binInfo.bop.stateThermo1 = 0;
  binInfo.bop.stateThermo2 = 0;
  binInfo.bop.stateThermo3 = 0;
  binInfo.bop.stateThermo4 = 0;
  binInfo.bop.stateThermo5 = 0;
  binInfo.bop.stateThermo6 = 0;
  binInfo.bop.stateThermo7 = 0;
  binInfo.bop.stateThermo8 = 0;
  binInfo.bop.stateFuel1 = 0;
  binInfo.bop.stateFuel2 = 0;
  binInfo.bop.mileageChange = getFlagGPSChanges();
  //binInfo.bop.mileageChange = 1;   //deb
  
  //binInfo.bop.operatorName = 0;
  if(flag)
    {
     if(!(cntOpName++ % 50))
	    {
		  binInfo.bop.operatorName = 1;
		}
     else  if(binInfo.fos.cellNameChanged)
	  {
	    binInfo.bop.operatorName = 1;
		binInfo.fos.cellNameChanged = 0;
	  }
    }
  else
    binInfo.bop.operatorName = 1;

  //binInfo.bop.sat = (info.sig == NMEA_SIG_BAD)? 0 : 1;
  binInfo.bop.sat = 1;
  if(!flag)
    binInfo.bop.curDataState = 1;
  binInfo.bop.gsmLocation = innerState.flGotGSMLocation;

  *(u32*)buf_tx_ptr = *(u32*)&binInfo.bop;
  buf_tx_ptr += sizeof(binInfo.bop);
  len +=  sizeof(binInfo.bop); 

  /* Variable part */
  //digInputPrev = binInfo.stateInput;                                          
  

  if(binInfo.bop.digInput)
    {
       binInfo.stateInput = readStateInput();
	   //binInfo.stateInput = innerState.flagSDDisappeared | (innerState.flagSDPhysAppeared << 1);   //debugga
					 
       *buf_tx_ptr++ = binInfo.stateInput;
       len++;
    }
  if(binInfo.bop.digOutput)
    {
       binInfo.stateOutputs = readStateOutput();
       *buf_tx_ptr++ = binInfo.stateOutputs;
       len++;

    }
  /*power*/
  if(binInfo.bop.stADC0)
    {
	  binInfo.stateADC[0] = ((float)(ain_read(0))) * koefVPower * 100;
	  //binInfo.stateADC[0] = ain_read(2);
      *(u16*)buf_tx_ptr = binInfo.stateADC[0];
      buf_tx_ptr += 2;
      len += 2;
    }
  /* Battery */
  if(binInfo.bop.stADC1)
    {
      binInfo.stateADC[1] = ain_read(1)*koefVBAT*100;
	  //binInfo.stateADC[1] = ain_read(3);
      *(u16*)buf_tx_ptr = binInfo.stateADC[1];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stADC2)  //temperature
    {
      binInfo.stateADC[2] = ain_read(8);
	  //binInfo.stateADC[2] = (1.43 - ain_read(8)*koefStdADC )/4.3 + 25;
      *(u16*)buf_tx_ptr = binInfo.stateADC[2];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stADC3)
    {
      binInfo.stateADC[3] = ain_read(9); //*koefStdADC*100;
	  //binInfo.stateADC[3] = ain_read(5);
      *(u16*)buf_tx_ptr = binInfo.stateADC[3];
      buf_tx_ptr += 2;
      len += 2;
    }

#if defined (VER_3)
  if(binInfo.bop.stADC4)
    {
      binInfo.stateADC[4] = ain_read(4)*koefStdADC*100;
      *(u16*)buf_tx_ptr = binInfo.stateADC[4];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stADC5)
    {
      binInfo.stateADC[5] = ain_read(5)*koefStdADC*100;
      *(u16*)buf_tx_ptr = binInfo.stateADC[5];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stADC6)
    {
      binInfo.stateADC[6] = ain_read(6)*koefStdADC*100;
      *(u16*)buf_tx_ptr = binInfo.stateADC[6];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stADC7)
    {
      binInfo.stateADC[7] = ain_read(7)*koefStdADC*100;
      *(u16*)buf_tx_ptr = binInfo.stateADC[7];
      buf_tx_ptr += 2;
      len += 2;
    }
#endif

  if(binInfo.bop.stateThermo1)
    {
      binInfo.stateThermo[0] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateThermo[0];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stateThermo2)
    {
      binInfo.stateThermo[1] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateThermo[1];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stateThermo3)
    {
      binInfo.stateThermo[2] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateThermo[2];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stateThermo4)
    {
      binInfo.stateThermo[3] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateThermo[3];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stateThermo5)
    {
      binInfo.stateThermo[4] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateThermo[4];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stateThermo6)
    {
      binInfo.stateThermo[5] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateThermo[5];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stateThermo7)
    {
      binInfo.stateThermo[6] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateThermo[6];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stateThermo8)
    {
      binInfo.stateThermo[7] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateThermo[7];
      buf_tx_ptr += 2;
      len += 2;
    }

  if(binInfo.bop.stateFuel1)
    {
      binInfo.stateFuel[0] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateFuel[0];
      buf_tx_ptr += 2;
      len += 2;
    }
  if(binInfo.bop.stateFuel2)
    {
      binInfo.stateFuel[1] = 0;
      *(u16*)buf_tx_ptr = binInfo.stateFuel[1];
      buf_tx_ptr += 2;
      len += 2;
    }

  if (binInfo.bop.mileageChange)
    {
      binInfo.mileage = mileage;
      *(u32*)buf_tx_ptr = binInfo.mileage;
      buf_tx_ptr += sizeof(binInfo.mileage);
      len += sizeof(binInfo.mileage);
    }
  if(binInfo.bop.operatorName)
    {
	  tempo = strlen(cspnBuf);
	  memcpy(buf_tx_ptr, cspnBuf, tempo );
	  buf_tx_ptr += tempo;
	  *buf_tx_ptr++ = 0;
      len += tempo+1;
	}

  if(binInfo.bop.sat) 
    {
	  if(info.sig != NMEA_SIG_BAD)
	    {
	      binInfo.sat.timeLabel     = rtcGetNaviTime(&info);     //0x12345678;
		  binInfo.sat.signs = 0;
		  ///*
		  if(info.lon < 0)
		    {
			   info.lon *= (-1);
			   binInfo.sat.signs |= 0x01;
			}
		  temp = info.lon - (u32)info.lon;
		                                //deg						   min
		  binInfo.sat.longitude     = (((u32)info.lon)/100 << 24) | (((u32)info.lon)%100 << 16) | (u16)(temp * 10000);
			//*/
		  //binInfo.sat.longitude = (float)info.lon;

		  if(info.lat < 0)
		    {
			   info.lat *= (-1);
			   binInfo.sat.signs |= 0x02;
			}
		  temp = info.lat - (u32)info.lat;
		  binInfo.sat.latitude     = (((u32)info.lat) << 16) | (u32)(temp * 10000);
		  
	      binInfo.sat.altitude      = info.elv;
	      binInfo.sat.direction     = info.direction;
	      binInfo.sat.speedKM       = info.speed;
	      binInfo.sat.naviNumGLN    = info.satinfo.inuseGLN;
	      binInfo.sat.naviNumGPS    = info.satinfo.inuseGPS;
	   }
	      memcpy(buf_tx_ptr, &binInfo.sat, sizeof(binInfo.sat));
	      //buf_tx_ptr = (u8*)&binInfo.sat;
	      buf_tx_ptr += sizeof(binInfo.sat);
	      len += sizeof(binInfo.sat);
	      //len = sizeof(binInfo.sat.timeLabel);
	      //len = sizeof(binInfo.sat.naviSatNumber); 
	    
    }
   if(binInfo.bop.alarm)
	 {
       binInfo.alarm = innerState.alarm;
       *(u32 *)buf_tx_ptr = binInfo.alarm;
	   buf_tx_ptr += 4;
       len += 4;
	 } 

   if(!flag)  //by USB
	   if(binInfo.bop.curDataState)
	     {
		  tempo = strlen((char *)statesBlock[innerState.dataSt]);
		  //tempo = sizeof(statesBlock[innerState.dataSt]);
		  memcpy(buf_tx_ptr, statesBlock[innerState.dataSt], tempo );
		  buf_tx_ptr += tempo;
		  *buf_tx_ptr++ = 0;
	      len += tempo+1;
	     } 
   if(binInfo.bop.gsmLocation)
	 {
       memcpy(buf_tx_ptr, &binInfo.gsm_loc, sizeof(binInfo.gsm_loc));
	   buf_tx_ptr += sizeof(binInfo.gsm_loc);
       len += sizeof(binInfo.gsm_loc);
	 } 

  return len;   
}

//-------------------------------------------------------------------------------------------
void handleTimerPrBin(void)
{
    if(timerValuePrBin)
	  {
	   timerValuePrBin--;
	   if(!timerValuePrBin )
	      flagPrBin = 1;
	  }
}
// ------------------------------------------------------------------------------
void setTimerPrBin(u32 val)
{
   	timerValuePrBin = val;
}










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
#include "hand_conf.h"
#include "crc.h"
#include "flash.h"
#include "protocol_ascii.h"
#include "protocol_bin.h"
#include "bin_handle.h"
#include "gsm.h"
#include "sdSDIODrv.h"

u16      anglePrev;
double   mileageExact;
u32      mileageSaved;
u32      orderMsgNumber;
//extern u32      indexPosFI;
//extern u32      indexFLLen;
configSet config;

extern MLGInfo   mlgInfo;
extern PGIOInfo  pgioInfo;
extern bin_info  binInfo;
extern t_innerState innerState;

u16 chanADCPrev[4];
u8  digInputPrev = 0;
u8  digOutputPrev = 0;
extern nmeaINFO info;
u32 timerValueAnChanging;
u32 timerValueAllowChanging;
#if defined (VER_3)
extern float  koefVPower;
#endif
u8  flagAnChanging = 1;
u8  flagWasStart;
u8  chooseProtocol;

/*inner declarations*/
void setTimerAllowCh(u32 val);

/*******************************************************************************/
/* 	should make timer and set individual bit when 10% changing happened																		   */
/*******************************************************************************/
u8 getProtocol(void) 
{
  return chooseProtocol;
}
/*******************************************************************************/
/* 	should make timer and set individual bit when 10% changing happened																		   */
/*******************************************************************************/
void handleADCChange(void)
{
  u8 i;
  u16 temp;
  //static u8 cnt = 0;
  static u16 tempPrev = 0;
  static u16 toffVolt = 0;
  static u16 trVolt = 0;	//Transformed turn-oFF Voltage
  static u16 deltaVolt = 0;	//delta = 0.2V
  static u8 flagSlow = 0;

  if(toffVolt != config.turnOffVoltage)
     {
	   toffVolt = config.turnOffVoltage; 
	   trVolt = toffVolt / (koefVPower * 10);
	   deltaVolt = 0.2 / koefVPower;
	 }
  /*Handling turnOff Voltage*/

   temp = ain_read(0);    
   if( temp >= trVolt )
      {
		 if((tempPrev - temp) < ( temp * 0.05 ))
		    flagSlow = 1;
		 else
		    flagSlow = 0;
	  }
   else
      {
	    if((tempPrev - temp) < ( temp * 0.05 ))
		    flagSlow = 1;
		//if( (temp < (trVolt - deltaVolt)) && (flagSlow) )
		//    led_dn(BOARD_LED_XOR);
      }

   tempPrev = temp;

  /* 10 persent differ */
  for (i  = 0; i < 4; i++)
    {
	  temp = ain_read(i);
	  if(abs(chanADCPrev[i] - temp) > (0.1 * temp))
		  binInfo.fos.anInputChanged = 1;
      chanADCPrev[i] = temp;
	}

  /*Handling turnOff Voltage*/
  /*
   temp = ain_read(0);   // koefVPower * 10; 
   temp3 = temp;
   if( (tempPrev - temp) < ( temp * 0.05 ) )
      {
	   temp *= koefVPower * 10;
	   if(temp < (config.turnOffVoltage - 2))	//-2 is a delta 0.2V
		  {
			  cnt++;
			  if(cnt == 5)
			    {
				  //led_dn(BOARD_LED_XOR);
				  cnt = 0;
				}
		  }
		else
		  {
		    cnt = 0;
		  }
	 }
	tempPrev = temp3;
	*/

	

}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleDigInOutChange(void)
{
  u8 temp;
  /* Digit Input changed*/
  temp = readStateInput();
  if(temp != digInputPrev)
    binInfo.fos.digInputChanged = 1; 
  digInputPrev = temp;
  /* Digit Output changed*/
  temp = readStateOutput();
  if(temp != digOutputPrev)
    binInfo.fos.digOutputChanged = 1; 
  digOutputPrev = temp;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleVehicleMovement(void)
{
   static u8 cnt = 0;
   static u8 flagStartStop = 0;  //0 - stop, 1 - start
   if(info.speed < 3)                //stopping
     {
	    if(flagWasStart)
		 {
		   if(flagStartStop)
		     cnt = 0;
		   flagStartStop = 0;
		   if(cnt++ == 60)	  // 60 = 3sec * 1000ms/50ms
		     {
	           binInfo.fos.vehicleStop = 1;
			   flagWasStart = 0;
			   cnt = 0;
			 }
	     }
	 }
   else                              //moving
     {
	    if(!flagWasStart)
		 {
		   if(!flagStartStop)
		     cnt = 0;
		   flagStartStop = 1;
		   if(cnt++ == 60)
		     {
	           binInfo.fos.vehicleStart = 1;
			   flagWasStart = 1;
			   cnt = 0;
			 }
		 }
	 }
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleFOS(void)
{
   static u8 step = 0;
   static u8 cntSDPres = 0;
   //static u8 flagWasUSB = 0;
   static u8 flagWasNotUSB = 1;
   //static u16 a = 0;
   if(flagAnChanging)
     {

	   flagAnChanging = 0;
	   setTimerAnChanging(MS_100 * 5);

	   handleDigInOutChange();  //every 500 ms

	   if(!(step % 2))  // every second step(500ms * 2 = 1000ms)
		  handleADCChange();

	   handleVehicleMovement();

       if(innerState.allowedChange)
	      {
		  }

	   
	   if(!(step % 2))  // every second step(500ms * 2 = 1000ms)
	     {
		   /*Handling SD Presence or not*/
		   if(innerState.flagSDPhysAppeared)
		     {
	            SD_Detect();
			    if(cntSDPres++ >= 10)  //1000ms/500 = 2(per 1sec); 5sec
				   {
				     cntSDPres = 0;
					 innerState.flagSDDisappeared = 0;
					 innerState.flagSDPhysAppeared = 0;
					 SD_Init(); 
				   }
			 }
		   else
			  cntSDPres = 0;
		   /*Handling USB connecting*/
	#if defined (VER_3)
		   if(USBDetectPin())   /*got connect*/
		     {
			   if(flagWasNotUSB) 
			     {
				   flagWasNotUSB = 0;
				   //flagWasUSB  = 1;
				   //repeat ini
				
				   USBCommonIni();   
				 }
			   
		     }
			else                /*got disconnect*/
			 {
				//if(flagWasUSB)
				  { 
				     //flagWasUSB = 0;
					 //flagWasNotUSB = 1;
					 //make deInit
				  } 
			 }
	#endif
		 }


		step++;
     }
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleUSBPresent(void)
{
	//#if defined (VER_3)
    static u8 flagWasNotUSB = 1;
	if(flagWasNotUSB)
      if(USBDetectPin())   /*got connect*/
		     {
				   flagWasNotUSB = 0;
				   USBCommonIni();   
		     }
	//#endif
}
	
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void readGPSSettings(u8 chooseProtocol)
{
	u32 temp;
  	orderMsgNumber  = BKPReadReg(BKP_DR_MSG_NUMBER1) | ( (u32)(BKPReadReg(BKP_DR_MSG_NUMBER2) << 16)  );
  	mileageSaved    = BKPReadReg(BKP_DR_MILEAGE_N1)  | ( (u32)(BKPReadReg(BKP_DR_MILEAGE_N2) << 16)  );
  	//indexPosFI      = BKPReadReg(BKP_DR_INDEX_POSFI_N1)  | ( (u32)(BKPReadReg(BKP_DR_INDEX_POSFI_N2) << 16) );
  	//indexFLLen      = BKPReadReg(BKP_DR_INDEX_POSFL_N1)  | ( (u32)(BKPReadReg(BKP_DR_INDEX_POSFL_N2) << 16) );
  	temp            = BKPReadReg(BKP_DR_INNER_STATE_N1)  | ( (u32)(BKPReadReg(BKP_DR_INNER_STATE_N1) << 16) );

	innerState.activeSIMCard = 0;
	if(temp & 0x01)
	   innerState.activeSIMCard = 1;

	innerState.currentServer = 0;
	if(temp & 0x02)
	   innerState.currentServer = 1;

	if(chooseProtocol == PROTOCOL_BINARY)
	  {
	    binInfo.orderMsgNumber = orderMsgNumber;
		binInfo.mileage = mileageSaved; 
	  }
	 else
	  {
	    pgioInfo.orderMsgNumber = orderMsgNumber;
		mlgInfo.mileage = mileageSaved; 
	  }
}
/*******************************************************************************/
/*     			   */
/*******************************************************************************/
u8 checkPsw(u8 * buf)
{
  u8 res = 0;
  if(!strncmp((char *)buf, (char *)config.pswConfig, strlen((char *)config.pswConfig)))
    {
      innerState.allowedChange = 1;
	  res = 1;
	}
  *buf++ = res;
  return 1; //1 - sizeof cntUSBBootBytes
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void readConfig(void)
{
    u32 * pAddress;
	u32   data;
	pAddress = (u32 *)ADDRESS_LAST_PAGE;
	data  = *pAddress;
	if(!(data & 0x10000))	// if bit32 == 0
	   innerState.fwUpdated = 1; /*Firmware has been updated*/
  
  chooseProtocol = PROTOCOL_BINARY;  //
  readGPSSettings(chooseProtocol);
  read_page(START_ADDRESS_SETTINGS, (u8 *)&config, sizeof(config));

  if(!innerState.fwUpdated)	  /*usual start*/
     if(config.crc != calcBlock((u8 *)&config, sizeof(config) - sizeof(config.crc)))
        defaultConfig();

  flagWasStart = 0;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u8 writeConfig(u16 nSize, u8 * data)
{
  u8 id, res = 0;
  if(innerState.allowedChange) 
    {
	   id = *data++;
	   res = 1;
	   setTimerAllowCh(ONE_SEC * 60 * 15);
	   switch(id)
		  {
		
		    case  CNF_BLOCK_ID:            config.idBlock = *(u32 *)data;                break;
		    case  CNF_INTERVAL_SEND_PWR:   config.intervalSendingPower = *(u16 *)data;   break;
		    case  CNF_INTERVAL_SEND_BATT:  config.intervalSendingBattery = *(u16 *)data; break;
		    case  CNF_FLAG_TRACK_CONTROL:  config.flagTrackControl = *data;              break;
		    case  CNF_VEL_LIMIT:           config.velocityLimit = *(u16 *)data;          break;
		    case  CNF_ANGLE_LIMIT:         config.angleLimit = *(u16 *)data;             break;
		    case  CNF_DIST_LIMIT:          config.distanceLimit = *(u16 *)data;          break;
		    case  CNF_TURN_OFF_VOLT:       config.turnOffVoltage = *(u16 *)data;         break;
		    case  CNF_MICROPH_SENS :       config.microPhoneSens = *data;  
										   callGSMFunction(1);
						                   break;
		    case  CNF_DYN_VOL      :       config.dynamicLevel = *data;                  
										   callGSMFunction(2);
										   break;
		    case  CNF_ALARM1       :       config.flagAlarm1 = *data;                    break;
		    case  CNF_ALARM2       :       config.flagAlarm2 = *data;                    break;
		    case  CNF_ALARM3       :       config.flagAlarm3 = *data;                    break;
		    case  CNF_ALARM4       :       config.flagAlarm4 = *data;                    break;
		    case  CNF_ALARM_AIN1   :       config.flagAlarmAIN1 = *data;                 break;
		    case  CNF_ALARM_AIN2   :       config.flagAlarmAIN2 = *data;                 break;
		    case  CNF_ALARM_AIN3   :       config.flagAlarmAIN3 = *data;                 break;
		    case  CNF_ALARM_AIN4   :       config.flagAlarmAIN4 = *data;                 break;
		    case  CNF_ALARM_AIN5   :       config.flagAlarmAIN5 = *data;                 break;
		    case  CNF_ALARM_AIN6   :       config.flagAlarmAIN6 = *data;                 break;
		
		
		    case  CNF_APN1:        memset((char *)config.apn[0],0,strlen((char *)config.apn[0]));
		                           strncpy((char *)config.apn[0],(char *)data,nSize);
								   setGSMOperator(0);
								                         												   break;
		    case  CNF_USERNAME1:   memset((char *)config.username[0],0,strlen((char *)config.username[0]));
		                           strncpy((char *)config.username[0],(char *)data,nSize); 
								   setGSMOperator(0);
		              																					   break;
		    case  CNF_PASSWORD1:   memset((char *)config.password[0],0,strlen((char *)config.password[0]));
		                           strncpy((char *)config.password[0],(char *)data,nSize);                 
								   setGSMOperator(0);								
																										   break;
		    case  CNF_BALANCE_SIM1:memset((char *)config.getBalUSSD[0],0,strlen((char *)config.getBalUSSD[0]));
		                           strncpy((char *)config.getBalUSSD[0],(char *)data,nSize);
								   setCUSDQuery(0);                 
		    case  CNF_MY_PHONE_SIM1:memset((char *)config.getPhNumUSSD[0],0,strlen((char *)config.getPhNumUSSD[0]));
		                           strncpy((char *)config.getPhNumUSSD[0],(char *)data,nSize); 
								   setCUSDPhNumQuery(0);                
								   																		   break;
		
		    case  CNF_APN2:        memset((char *)config.apn[1],0,strlen((char *)config.apn[1]));
		                           strncpy((char *)config.apn[1],(char *)data,nSize);                      
								   setGSMOperator(1);
								   																			break;
		    case  CNF_USERNAME2:   memset((char *)config.username[1],0,strlen((char *)config.username[1]));
		                           strncpy((char *)config.username[1],(char *)data,nSize);                 
								   setGSMOperator(1);
								   																			break;
		    case  CNF_PASSWORD2:   memset((char *)config.password[1],0,strlen((char *)config.password[1]));
		                           strncpy((char *)config.password[1],(char *)data,nSize);                 
								   setGSMOperator(1);
								   																			break;
		    case  CNF_BALANCE_SIM2:memset((char *)config.getBalUSSD[1],0,strlen((char *)config.getBalUSSD[1]));
		                           strncpy((char *)config.getBalUSSD[1],(char *)data,nSize); 
								   setCUSDQuery(1);                
		    case  CNF_MY_PHONE_SIM2:memset((char *)config.getPhNumUSSD[1],0,strlen((char *)config.getPhNumUSSD[1]));
		                           strncpy((char *)config.getPhNumUSSD[1],(char *)data,nSize); 
								   setCUSDPhNumQuery(1);                
								   																		   break;
		
		    case  CNF_SERVER1:     memset((char *)config.server[0],0,strlen((char *)config.server[0]));
		                           strncpy((char *)config.server[0],(char *)data,nSize);
								   setGPRSServer(0);
								   																			break;
		    case  CNF_PORT1:       config.port[0] = *(u16 *)data;                                  		 
								   setGPRSServer(0);
								   																			break;
		    case  CNF_SERVER2:     memset((char *)config.server[1],0,strlen((char *)config.server[1]));
		                           strncpy((char *)config.server[1],(char *)data,nSize);                   
								   setGPRSServer(1);
								   																			break;
		    case  CNF_PORT2:       config.port[1] = *(u16 *)data;                           			
									setGPRSServer(1);	 
			 																								break;
		
		    case  CNF_CONTROL_NUM1: memset((char *)config.contrNumber[0],0,strlen((char *)config.contrNumber[0]));
		                            strncpy((char *)config.contrNumber[0],(char *)data,nSize);              break;
		    case  CNF_CONTROL_NUM2: memset((char *)config.contrNumber[1],0,strlen((char *)config.contrNumber[1]));
		                            strncpy((char *)config.contrNumber[1],(char *)data,nSize);              break;
		
		    case  CNF_FTP_ADRES   : memset((char *)config.ftpServer,0,strlen((char *)config.ftpServer));
		                            strncpy((char *)config.ftpServer,(char *)data,nSize);              break;
		    case  CNF_FTP_USERNAME: memset((char *)config.ftpUsername,0,strlen((char *)config.ftpUsername));
		                            strncpy((char *)config.ftpUsername,(char *)data,nSize);                 break;
		    case  CNF_FTP_PASSWORD: memset((char *)config.ftpPassword,0,strlen((char *)config.ftpPassword));
		                            strncpy((char *)config.ftpPassword,(char *)data,nSize);                 break;
		
		    case  CNF_PASSWORD:     memset((char *)config.pswConfig,0,strlen((char *)config.pswConfig));
		                            strncpy((char *)config.pswConfig,(char *)data,nSize);                 break;
		    
		    /*UPGRADING*/                 
		    case  CNF_UPD_CONFIG:  config.crc = calcBlock((u8 *)&config, sizeof(config) - sizeof(config.crc));
		  						   write_firm_page(START_ADDRESS_SETTINGS, (u8 *)&config);               break;
		
		    default:   break;
		  }
  	  }
	 *data++ = res;
  return 2; 
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void defaultConfig(void)
{
 
  config.idBlock = 1;
  config.intervalSendingPower   = 5;
  config.intervalSendingBattery = 30;
  config.flagTrackControl       = 1;
  config.velocityLimit          = 63;
  config.angleLimit             = 15;
  config.distanceLimit          = 1000;
  config.turnOffVoltage         = 110;
  config.microPhoneSens         = 7;
  config.dynamicLevel           = 40;
  config.flagAlarm1             = 0;
  config.flagAlarm2             = 0;
  config.flagAlarm3             = 0;
  config.flagAlarm4             = 0;
  config.flagAlarmAIN1          = 0;
  config.flagAlarmAIN2          = 0;
  config.flagAlarmAIN3          = 0;
  config.flagAlarmAIN4          = 0;
  config.flagAlarmAIN5          = 0;
  config.flagAlarmAIN6          = 0;

  memset((char *)config.apn[0],0,strlen((char *)config.apn[0]));
  strncpy((char *)config.apn[0],(char *)"internet.mts.ru",strlen((char *)"internet.mts.ru"));
  memset((char *)config.username[0],0,strlen((char *)config.username[0]));
  strncpy((char *)config.username[0],(char *)"mts",strlen((char *)"mts"));
  memset((char *)config.password[0],0,strlen((char *)config.password[0]));
  strncpy((char *)config.password[0],(char *)"mts",strlen((char *)"mts"));
  memset((char *)config.getBalUSSD[0],0,strlen((char *)config.getBalUSSD[0]));
  strncpy((char *)config.getBalUSSD[0],(char *)"*100#",strlen((char *)"*100#"));
  memset((char *)config.getPhNumUSSD[0],0,strlen((char *)config.getPhNumUSSD[0]));
  strncpy((char *)config.getPhNumUSSD[0],(char *)"*100#",strlen((char *)"*100#"));

  memset((char *)config.apn[1],0,strlen((char *)config.apn[1]));
  strncpy((char *)config.apn[1],(char *)"internet.mts.ru",strlen((char *)"internet.mts.ru"));
  memset((char *)config.username[1],0,strlen((char *)config.username[1]));
  strncpy((char *)config.username[1],(char *)"mts",strlen((char *)"mts"));
  memset((char *)config.password[1],0,strlen((char *)config.password[1]));
  strncpy((char *)config.password[1],(char *)"mts",strlen((char *)"mts"));
  memset((char *)config.getBalUSSD[1],0,strlen((char *)config.getBalUSSD[1]));
  strncpy((char *)config.getBalUSSD[1],(char *)"*100#",strlen((char *)"*100#"));
  memset((char *)config.getPhNumUSSD[1],0,strlen((char *)config.getPhNumUSSD[1]));
  strncpy((char *)config.getPhNumUSSD[1],(char *)"*100#",strlen((char *)"*100#"));
  /* main(1) and reserv(2) servers adresses and ports*/
  memset((char *)config.server[0],0,strlen((char *)config.server[0]));
  strncpy((char *)config.server[0],(char *)"85.233.64.176",strlen((char *)"85.233.64.176"));
  config.port[0] = 8084;
  memset((char *)config.server[1],0,strlen((char *)config.server[1]));
  strncpy((char *)config.server[1],(char *)"85.233.64.176",strlen((char *)"85.233.64.176"));
  config.port[1] = 8084;
  /* control numbers - from allowed take control*/
  memset((char *)config.contrNumber[0],0,strlen((char *)config.contrNumber[0]));
  strncpy((char *)config.contrNumber[0],(char *)"+79372823296",strlen((char *)"+79372823296"));
  memset((char *)config.contrNumber[1],0,strlen((char *)config.contrNumber[1]));
  strncpy((char *)config.contrNumber[1],(char *)"+79178603622",strlen((char *)"+79178603622"));
  /*FTP PARAMS*/
  memset((char *)config.ftpServer,0,strlen((char *)config.ftpServer));
  strncpy((char *)config.ftpServer,(char *)"85.233.64.176",strlen((char* )"85.233.64.176"));              
  memset((char *)config.ftpUsername,0,strlen((char *)config.ftpUsername));
  strncpy((char *)config.ftpUsername,(char *)"gsm",strlen((char* )"gsm"));                
  memset((char *)config.ftpPassword,0,strlen((char *)config.ftpPassword));
  strncpy((char *)config.ftpPassword,(char *)"qwe",strlen((char* )"qwe")); 
  /*password*/ 
  //memset((char *)config.pswConfig,0,strlen((char *)config.pswConfig));
  memset((char *)config.pswConfig,0,sizeof((char *)config.pswConfig));
  strncpy((char *)config.pswConfig,(char *)"admin",strlen((char* )"admin")); 

  config.crc = calcBlock((u8 *)&config, sizeof(config) - sizeof(config.crc));

  write_firm_page(START_ADDRESS_SETTINGS, (u8 *)&config);

}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleTimerAnChanging(void)
{
    if(timerValueAnChanging)
	  {
	   timerValueAnChanging--;
	   if(!timerValueAnChanging )
	      flagAnChanging = 1;
	  }

    if(timerValueAllowChanging)
	  {
	   timerValueAllowChanging--;
	   if(!timerValueAllowChanging )
	      innerState.allowedChange = 0;
	  }

}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerAnChanging(u32 val)
{
   	timerValueAnChanging = val;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerAllowCh(u32 val)
{
   	timerValueAllowChanging = val;
}








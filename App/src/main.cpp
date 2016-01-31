/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Mass Storage demo main file
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
 
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "dio_led.h"
#include "adc.h"
#include "gps.h"
#include "gprs_modem.h"
#include "gsm.h"
#include "button.h"
#include "rs_485.h"
#include "sdcardUser.h"
#include "rtc.h"
#include "bkp.h"
//#include "fifoPackets.h"
#include "ftp.h"
#include "packets.h"
#include "protocol_bin.h"
#include "crc.h"
#include "ff.h"
#include "hand_conf.h"
#include "ais326dq.h"
#include "alarm.h"
#include "ibutton.h"
#include "dac.h"
#include "rfmodem.h"
#include "sdSDIODrv.h"
#include <string.h>
#include "flash.h"
#include "timers.h"
#include <stdio.h>
//#include "fifo.h"  //deb
#include "waveplayer.h"
//#include <stdlib.h>
#include <nmea/nmea.h>		//debug

/* Private function prototypes -----------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

struct ais326dq_data_t ais326dq_out;

//#define ADDRESS_VECTOR_FLASH  0 //0x08008000   
#define ADDRESS_VECTOR_FLASH  0x08008000   

//----------------------Temp sector------------------------
u8 debugPacketStop = 0;
//u8 debSign = 1;
//----------------------END OF TEMP SECTOR------------------------

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 signUSBMass;  // 1- mass storage, 0 - virtual com port

RTC_t rtc;
//double temp, temp2;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************/
extern t_innerState innerState;
/*******************************************************************************/
void NVIC_Configuration(void)
{ 
	#ifdef  VECT_TAB_RAM  
	  /* Set the Vector Table base location at 0x20000000 */ 
	  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#else  /* VECT_TAB_FLASH  */
	  /* Set the Vector Table base location at 0x08000000 */ 
	  NVIC_SetVectorTable(NVIC_VectTab_FLASH, ADDRESS_VECTOR_FLASH );   	   //0x08008000
	#endif
}
/*******************************************************************************
* Function Name  : main.
* Description    : Main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
u16 a = 0;
int main(void)
{
//***********************************DEBUG********************************************/
#if 0

nmeaPOS p1,p2;
nmeaINFO info;
info.lat = 5547.1206;
info.lon = 4906.2111;
nmea_info2pos(&info, &p1);
info.lat = 5547.1221;
info.lon = 4906.208;
nmea_info2pos(&info, &p2);

m += 23;

u32 t    = nmea_distance(&p1, &p2);
if(m)
#endif
//***********************************END OF DEBUG********************************************/
  NVIC_Configuration();	//for  all peripheria
  if (SysTick_Config(SystemCoreClock / 1000))  //1ms
     { 
       /* Capture error */ 
       while (1);
     }
  Delay(500);


  USBIniPin();
  	  
  signUSBMass = USBDetectPin();
  signUSBMass = 0;  //deb
  #if not defined (VER_3)
    USBCommonIni(); 
  #endif
  if(signUSBMass)
    {
	 #if defined (VER_3)
	 USBCommonIni();
	 #endif
     while (bDeviceState != CONFIGURED);
	}
  else //if(!signUSBMass)
   {

  	 /* Flash unlock */
     FLASH_Unlock();
     /* Clear All pending flags */
     FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

//***********************************DEBUG********************************************/

     
//***********************************END OF DEBUG********************************************/
  /* Enable CRC clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    
  ledDioGPIOInit();
  led_dn(BOARD_LED_ON);
  led_mid(BOARD_LED_ON);
#if defined (VER_3)
  led_up(BOARD_LED_ON);
  ibuttonInit();
  rfmodemInit();
#endif
  Delay(1000);
  alarmInit();
  BKPInit();

  //timer6Init();

  //rs485Init();
#if not defined (VER_3)
  ais326dq_init();
#endif

  //ais326dq_data(&ais326dq_out);
  /*ADC*/
  adcInit();
  /*GPS*/
  gpsInit();

  /* reading settings */
  readConfig();
  /*MODEM*/
  gprsModemInit();
  gprsModemOn(innerState.activeSIMCard);
//***********************************DEBUG********************************************/
  //GSMSpeaker(1);
//***********************************END OF DEBUG********************************************/

#ifndef BRIDGE_USB_GSM
  setupGSM();
  ftpGSMPrepare();
  packetsIni();
#endif

  led_dn(BOARD_LED_OFF);
  led_mid(BOARD_LED_OFF);
#if defined (VER_3)
  led_up(BOARD_LED_OFF);
#endif

  rtc_init();
  rtc_gettime(&rtc);

#if 1			  /* WATCH DOG */
  /* IWDG timeout equal to 3.27 sec (the timeout may varies due to LSI frequency dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  /* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_64);	//32
  /* Set counter reload value to 0xFFF */
  IWDG_SetReload(0xFFF);
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
  setTimerIWDG(ONE_SEC);
#endif

  initSD();
#if defined (VER_3)
  //DACInit();
#endif

  /* Log  */
  saveSDInfo((u8 *)"TURN ON BLOCK ",strlen((u8 *)"TURN ON BLOCK "), SD_SENDED, SD_TYPE_MSG_LOG );
  //saveSDInfo((u8 *)readRTCTime(&rtc),strlen((const char *)readRTCTime(&rtc)), SD_SENDED, SD_TYPE_MSG_LOG );
#if defined (VER_3)  
  //DACSpeaker(1);
  //wp_play("0:/sound.wav");
  //DACSpeaker(0);
#endif 

  }	 //if(!signUSBMass)

  while (1)
  {
    if(!signUSBMass)
	  {
		 monitorWatchDog();

		 #ifndef BRIDGE_USB_GPS
	     if(!innerState.bootFTPStarted)
	         gpsHandling();
		 #endif

		 #ifndef BRIDGE_USB_GSM
		 if(!innerState.flagTmpDebug)
		    loopGSM();
			loopFTP();
            UpdatingFlash();
			if(!innerState.bootFTPStarted)
			   naviPacketHandle();
			rcvPacketHandle();
            rcvPacketHandleUSB();
		 #endif 

		 #if !defined (VER_3)
			 buttonScan();
		     accelScan();
		 #endif
		 
		 //rs485Analyse();
		 handleFOS();
		 executeDelayedCmd();
#if defined (VER_3)
		 #if 0 
		 if(innerState.flagDebug)
		 {
  			DACSpeaker(1);
	        /* Start DAC Channel1 conversion by software */
		    //a += 300;
    		//DAC_SetChannel1Data(DAC_Align_12b_R, 4000);
    		//DAC_SetChannel1Data(DAC_Align_12b_L, a);  //for saw
    		//DAC_SetChannel1Data(DAC_Align_8b_R, a);
			
			//DAC_SetChannel1Data(DAC_Align_12b_R, 4095);
			//DAC_SetChannel1Data(DAC_Align_12b_R, 0);
			//for (a = 0; a<4095; ++a)
			//for(;;)
			//  DAC_SetChannel1Data(DAC_Align_12b_R, 0);

			//DAC_SetChannel1Data(DAC_Align_12b_R, 0);
			//for ( ; ; )
			//{
			//   DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);
			//}
 

	        DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);  //debugga
		 }
		 else
		 {
		     DACSpeaker(0);
		 }
	  #endif
#endif
	 }
	else
	  handleUSBPresent();
  }	   //while(1)
}


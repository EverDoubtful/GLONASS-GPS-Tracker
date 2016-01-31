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
//#include "board.h"

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
#include "fifoPackets.h"
#include "ftp.h"
#include "packets.h"
#include "protocol_bin.h"
#include "bin_handle.h" //debugga
#include "crc.h"
#include "ff.h"
#include "hand_conf.h"
#include "ais326dq.h"
#include <string.h>
//#include <stdlib.h>
#include "menu.h"

#include "flash.h"
u32 reflect (u32 v,int b);

void TimingDelay_Decrement(void);
static __IO uint32_t TimingDelay;

/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nTime);


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BITMASK(X) (1L << (X))
#define MASK32 0xFFFFFFFFL
#define LOCAL static

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 bufNAVItoGSM[GPS_SENDING_BUFFER_SIZE];

struct ais326dq_data_t ais326dq_out;


//#define ADDRESS_VECTOR_FLASH  0 //0x08008000   
#define ADDRESS_VECTOR_FLASH  0x08008000   

//----------------------Temp sector------------------------
u8 debugPacketStop = 0;
//----------------------END OF TEMP SECTOR------------------------



extern uint16_t MAL_Init (uint8_t lun);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 signUSBMass;  // 1- mass storage, 0 - virtual com port
GPIO_InitTypeDef GPIO_InitStructure;

RTC_t rtc;
//double temp, temp2;
struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************/
int fputc(int ch, FILE *f)
 {
	  if (DEMCR & TRCENA) {
		while (ITM_Port32(0) == 0);
		ITM_Port8(0) = ch;
	  }
	  return(ch);
 }
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
//u8 bufExtBin[1024];
//u32 bufIndexData[3];
//extern u8 bufCmdBin[SIZE_EXT_BIN];
//const char * tr = "get ready";

int main(void)
{

//  u16 len = 0;			    
//  len = menuDisplay(bufExtBin);
//  if (len)
//	signUSBMass = 0;

  signUSBMass = 0;
//***********************************DEBUG********************************************/

      
//***********************************END OF DEBUG********************************************/

    /* Configure IO connected to USB PWR DET *********************/	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE );
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOD, &GPIO_InitStructure);

    if((GPIO_ReadInputData(GPIOD) & 0x08))   // USB is present after switch - work MASS Storage
      {
	     signUSBMass = 1;
      }

	signUSBMass = 0;   //debugga

#if 1  	  //that's for temporary off USB - for debugging
  
  NVIC_Configuration();
  Set_System();
  Set_USBClock();
  USB_Interrupts_Config();
  USB_Init();
  if(signUSBMass)
     while (bDeviceState != CONFIGURED);

#endif 
//
#if 1
  if(!signUSBMass)
   {
    if (SysTick_Config(SystemCoreClock / 1000))  //1ms
     { 
       /* Capture error */ 
       while (1);
     }

  	 /* Flash unlock */
     FLASH_Unlock();
     /* Clear All pending flags */
     FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

//***********************************DEBUG********************************************/

     
//***********************************END OF DEBUG********************************************/

  /* Enable CRC clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    
  ledDioGPIOInit();
  //LED_MIDDLE = 1;
  //GPIO_ResetBits(LED_MID_PORT, LED_MID_PORT_PIN);
  BKPInit();
  //debugga

  /*
  u32 mileage = 0;
  mileage         = BKPReadReg(BKP_DR_MSG_NUMBER1)  | ( (u32)(BKPReadReg(BKP_DR_MSG_NUMBER2) << 16)  );
  mileage++;
  BKPWriteReg(BKP_DR_MSG_NUMBER1,mileage);BKPWriteReg(BKP_DR_MSG_NUMBER2,mileage >> 16);
  mileage         = BKPReadReg(BKP_DR_MSG_NUMBER1)  | ( (u32)(BKPReadReg(BKP_DR_MSG_NUMBER2) << 16)  );
  mileage = 0;
  BKPWriteReg(BKP_DR_MSG_NUMBER1,mileage);BKPWriteReg(BKP_DR_MSG_NUMBER2,mileage >> 16);
  mileage         = BKPReadReg(BKP_DR_MSG_NUMBER1)  | ( (u32)(BKPReadReg(BKP_DR_MSG_NUMBER2) << 16)  );
  if(mileage)
	signUSBMass = 0;   //debugga
  */

  Delay(1000);

  //rs485Init();

  //Delay(1000);

  ais326dq_init();
  ais326dq_data(&ais326dq_out);

  adcInit();

  gpsInit();

  gprsModemInit();
  
  gprsModemOn();
#ifndef BRIDGE_USB_GSM
  setupGSM();
  ftpGSMPrepare();
  packetsIni();
#endif
  rtc_init();
  rtc_gettime(&rtc);
  initSD();

  /* reading settings */
  readConfig();
  /* Log  */
  saveSDInfo((u8 *)"TURN ON BLOCK ",strlen((u8 *)"TURN ON BLOCK "), SD_SENDED, SD_TYPE_MSG_LOG );
  saveSDInfo((u8 *)readRTCTime(&rtc),strlen((const char *)readRTCTime(&rtc)), SD_SENDED, SD_TYPE_MSG_LOG );

  }	 //if(!signUSBMass)
#endif

  //u8 tempCnt;
  while (1)
  {
    if(!signUSBMass)
	 {
	     gpsHandling();
		 #ifndef BRIDGE_USB_GSM
		    loopGSM();
			loopFTP();
            UpdatingFlash();
			naviPacketHandle();
			rcvPacketHandle();
            rcvPacketHandleUSB();
		 #endif 
		 buttonScan();
		 accelScan();
		 
//		 if(getButtonUserPressed())
//		  {
//		    led_mid(BOARD_LED_XOR);
//		    ais326dq_data(&ais326dq_out);
//		  }

		 //rs485Analyse();
		 handleFOS();
         adcScan();

  	 }  /* if(!signUSBMass)  */
  }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

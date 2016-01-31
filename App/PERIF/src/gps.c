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
#include "gps.h"
#include <nmea/nmea.h>
#include "bkp.h"
#include <string.h>
#include "hw_config.h"  //deb
#include <stdio.h>
nmeaINFO info;
nmeaPARSER parser;

//initial receive
		 u8  gpsPrimBuf[GPS_PRIM_RECEIVE_BUFFER_SIZE];
//volatile u16 gpsPrimCnt;
volatile u16 gpsPrimRxTimeout;
//handling by NMEA
		 //u8  gpsHndlBuf[GPS_HANDLE_NMEA_BUFFER_SIZE];
volatile u16 gpsHndlCnt;

nmeaPOS         gpsPointPrev, gpsPointCur; 
extern double   mileageExact;
       u32      mileage;
extern u32      mileageSaved;
u32             mileagePrev;
u8              flagGPSChanges;

/* Private functions ---------------------------------------------------------*/

void gpsInterruptsConfig(void);
void USART1_Configuration(void);
void gpsPGIOInit(void);
void GPS_Power_on(void);
void GPS_Power_off(void);
void USART1_RX_DMA_Init( void );

//-------------------------------------------------------------------------------------------
//-----------These functions work with hardware of GPS----------------------------------
//-------------------------------------------------------------------------------------------

void GPS_Power_on(void)
{
   GPIO_SetBits(GPS_PWR_PORT, GPS_PWR_PORT_PIN);
}
//-------------------------------------------------------------------------------------------
void GPS_Power_off(void)
{
   GPIO_ResetBits(GPS_PWR_PORT, GPS_PWR_PORT_PIN);
}
//-------------------------------------------------------------------------------------------
void gpsInterruptsConfig(void)
{
  	NVIC_InitTypeDef NVIC_InitStructure;
  	/* Configure the NVIC Preemption Priority Bits */  
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	  /* Enable the USART1 Interrupt */
	  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);

    /* Enable DMA1 channel5 IRQ Channel */
//    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);


}
//-------------------------------------------------------------------------------------------
void gpsPGIOInit(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE );
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE );

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPS_PWR,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPS_PPS,ENABLE );

	/* Configure IO connected to NAV_PWR *********************/	
	GPIO_InitStructure.GPIO_Pin =   GPS_PWR_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPS_PWR_PORT, &GPIO_InitStructure);

   	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
//  	GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_9;
//  	GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_AF_PP;
//  	GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_50MHz;
//  	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  	/* Configure USART1 Rx (PA.10) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
  	/* Configure GPS PPS  as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPS_PPS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPS_PPS_PORT, &GPIO_InitStructure);

}
//-------------------------------------------------------------------------------------------
void USART1_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate            = 115200;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);
	#ifdef BRIDGE_USB_GPS
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	#endif
	USART_Cmd(USART1, ENABLE);
}
//-------------------------------------------------------------------------------------------
void gpsInit(void)
{
	gpsPGIOInit();
	GPS_Power_on();
	#ifdef BRIDGE_USB_GPS
	   gpsInterruptsConfig();
	#endif
	USART1_Configuration();
	#ifndef BRIDGE_USB_GPS
		USART1_RX_DMA_Init();
	#endif
    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);
	mileagePrev = 0;
	mileageExact    = 0;
	gpsPrimRxTimeout = 200;
}

//-------------------------------------------------------------------------------------------
//------------------------Working with data------------------------------------------
//-------------------------------------------------------------------------------------------
//this function is cycling in usart interrupt handler

//void gps_rx_get_byte(u8 byte)
//{
//       if(gpsPrimCnt == GPS_PRIM_RECEIVE_BUFFER_SIZE) 
//         gpsPrimCnt = 0;
//       gpsPrimBuf[gpsPrimCnt++] = byte; /// 
//       gpsPrimRxTimeout = MS_100 * 1; /// 
//}
//-------------------------------------------------------------------------------------------
		
void gps_usart_decr_timer(void)
{
     if(gpsPrimRxTimeout)
      --gpsPrimRxTimeout; 
}

//-------------------------------------------------------------------------------------------
void gpsHandling(void)
{	
  static u16 cntBytesInFifoPrev = 0;
  u16 cpsr;
  if(gpsPrimRxTimeout == 0)
    {
	   gpsPrimRxTimeout = (MS_10 * 5);
	   if( ( DMA1_Channel5->CNDTR == cntBytesInFifoPrev) && (DMA1_Channel5->CNDTR != sizeof(gpsPrimBuf)) )
	      {
		    cpsr = __disable_irq();
	        //gpsPrimRxTimeout = MS_100 * 2;
			//gpsPrimCnt = sizeof(gpsPrimBuf) - DMA1_Channel5->CNDTR; 
			gpsHndlCnt = sizeof(gpsPrimBuf) - DMA1_Channel5->CNDTR; 
			//gpsPrimCnt = 0;
		    DMA1_Channel5->CCR  &= ~DMA_CCR5_EN;      //запретить работу канала
			DMA1_Channel5->CMAR  =  (uint32_t)gpsPrimBuf;   //адрес буфера в пам€ти
			DMA1_Channel5->CNDTR =  sizeof(gpsPrimBuf);     //size of buffer
			DMA1_Channel5->CCR  |=  DMA_CCR5_EN;      //разрешить работу канала

			if(!cpsr)     __enable_irq();
			//memcpy(gpsHndlBuf, gpsPrimBuf,gpsHndlCnt);
			//nmea_parse(&parser,(const char *) gpsHndlBuf, (int)gpsHndlCnt, &info);
			nmea_parse(&parser,(const char *) gpsPrimBuf, (int)gpsHndlCnt, &info);
			calcDistance();
		  }
	   cntBytesInFifoPrev = DMA1_Channel5->CNDTR;
	}
   /*
    if((gpsPrimRxTimeout == 0) && (gpsPrimCnt > 100))
      { 
	    u16 cpsr = __disable_irq();
        gpsPrimRxTimeout = MS_100 * 2; 
		gpsHndlCnt = gpsPrimCnt; 
		gpsPrimCnt = 0;
		if(!cpsr)     __enable_irq();
		memcpy(gpsHndlBuf, gpsPrimBuf,gpsHndlCnt);
		nmea_parse(&parser,(const char *) gpsHndlBuf, (int)gpsHndlCnt, &info);
		calcDistance();
      }
   */

}
//-------------------------------------------------------------------------------------------
 
void calcDistance(void)
{
  static u8 flagGPSAppear = 0;

	if(info.sig != NMEA_SIG_BAD)
	  {
	    if(flagGPSAppear)
		 {
		    gpsPointPrev.lat = gpsPointCur.lat;
		    gpsPointPrev.lon = gpsPointCur.lon;
		 }
		nmea_info2pos(&info, &gpsPointCur);
		if(flagGPSAppear)
		  { 
		    if((u32)(info.speed))  
			  {
			    mileageExact   += nmea_distance(&gpsPointPrev, &gpsPointCur); //+10 - debugga
			    mileage         = mileageSaved + (u32)(mileageExact);     //was += 
				if((mileage - mileagePrev) > 100 )  //if delta > 100 meters  than we should send it in packet
				   	{
					  mileagePrev      = mileage; 
					  flagGPSChanges = 0x01;
					}
				BKPWriteReg(BKP_DR_MILEAGE_N1,mileage);
		        BKPWriteReg(BKP_DR_MILEAGE_N2,mileage >> 16);
			  }
		  }
		flagGPSAppear = 1;
	  }
	else
	  flagGPSAppear = 0;
}
//-------------------------------------------------------------------------------------------
u8 getFlagGPSChanges(void)
{
   u8 temp = flagGPSChanges;
   flagGPSChanges = 0;
   return temp;
}

//-------------------------------------------------------------------------------------------
u8 getGLONASSStateValid(void)	// 1 - ok valid, 0 - no valid
{
//  u8 res = 0;
//  if(info.sig != NMEA_SIG_BAD)
//     res = 1;
//  return res;
  return (info.sig != NMEA_SIG_BAD) ? 1 : 0;
}
//-------------------------------------------------------------------------------------------
u16 getGpsPacket(u8 * buf)
{
  u16 len = 0;
  //memcpy(buf,gpsHndlBuf,gpsHndlCnt);
  memcpy(buf,gpsPrimBuf,gpsHndlCnt);
  len = gpsHndlCnt;
  //USBWriteStr("\r\n cmd:");
  //USBDebWrite(gpsHndlCnt);

  return len;
}
//-------------------------------------------------------------------------------------------
u16 zeroMileage(u8 * buf)
{
  mileage = 0;
  mileagePrev = 0;
  BKPWriteReg(BKP_DR_MILEAGE_N1,mileage);
  BKPWriteReg(BKP_DR_MILEAGE_N2,mileage >> 16);
  return 0;
}

//********************************************************************************
//Function: инициализаци€ DMA1 дл€ работы с USART1 (прием данных)            //
//********************************************************************************
void USART1_RX_DMA_Init( void )
{
 //¬ключить тактирование DMA1
 if ((RCC->AHBENR & RCC_AHBENR_DMA1EN) != RCC_AHBENR_DMA1EN)
      RCC->AHBENR |= RCC_AHBENR_DMA1EN;
 //«адать адрес источника и приемника и количество данных дл€ обмена
 DMA1_Channel5->CPAR  =  (u32)&USART1->DR;   //адрес регистра перефирии
 DMA1_Channel5->CMAR  =  (uint32_t)gpsPrimBuf;   //адрес буфера в пам€ти
 DMA1_Channel5->CNDTR =  sizeof(gpsPrimBuf);     //size of buffer

 //----------------- ћанипул€ции с регистром конфигурации  ----------------
 //—ледующие действи€ можно обьединить в одну команду (разбито дл€ нагл€дности)
 DMA1_Channel5->CCR   =  0;                       //предочистка регистра конфигурации
 DMA1_Channel5->CCR  |=  DMA_CCR5_CIRC;           //включить циклический режим
 DMA1_Channel5->CCR  &= ~DMA_CCR5_DIR;            //направление: запись в пам€ть
 //Ќастроить работу с переферийным устройством
 DMA1_Channel5->CCR  &= ~DMA_CCR5_PSIZE;          //размерность данных 8 бит
 DMA1_Channel5->CCR  &= ~DMA_CCR5_PINC;           //не использовать инкремент указател€
 //Ќастроить работу с пам€тью
 DMA1_Channel5->CCR  &= ~DMA_CCR5_MSIZE;          //размерность данных 8 бит
 DMA1_Channel5->CCR  |=  DMA_CCR5_MINC;           //использовать инкремент указател€
 USART1->CR3         |=  USART_CR3_DMAR;          //разрешить прием USART1 через DMA
 DMA1_Channel5->CCR  |=  DMA_CCR5_EN;      //разрешить работу канала
   /* Enable DMA1 Channel5 Transfer Complete interrupt */
 //DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);

}

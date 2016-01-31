/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Main Interrupt Service Routines.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "sdSDIODrv.h"
#include "hw_config.h"
#include "gps.h"
#include "gprs_modem.h"
#include "gsm.h"
#include "rs_485.h"
#include "adc.h"
#include "ftp.h"
#include "packets.h"
#include "ais326dq.h"
#include "protocol_bin.h"
#include "dio_led.h"
#include "alarm.h"
#include "hand_conf.h"
#include "timers.h"

void TimingDelay_Decrement(void);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/*******************************************************************************
* Function Name  : NMI_Handler
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
}

/*******************************************************************************
* Function Name  : HardFault_Handler
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : MemManage_Handler
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : BusFault_Handler
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : UsageFault_Handler
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : SVC_Handler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void)
{
}

/*******************************************************************************
* Function Name  : DebugMon_Handler
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void)
{
}

/*******************************************************************************
* Function Name  : PendSV_Handler
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void)
{
}

/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Handler(void)
{
    //static u16 a = 0;  //deb
    TimingDelay_Decrement();
	#ifndef VER_3
       handleTimerBtn();
	   handleTimerAccel();
	#endif
	gps_usart_decr_timer();
	handleTimerADC();
	//gpsPacketQueueHandle();
	handleTimerGSMPresence();
	handleTimerUSBBoot();
    handleTimerSendGSMFTP();
    handleTimerPackets();
    handleTimerPrBin();
	handleTimerIWDG();
	handleTimerAlarm();
	handleTimerLeds();
	handleTimerAnChanging();
	handleTimerGPRSModem();
}
//-----------------------------------------------GPS--------------------------------------------
#ifdef BRIDGE_USB_GPS
void USART1_IRQHandler(void)
{
  if((USART1->SR & USART_FLAG_RXNE) != RESET)
    {
	    /* Send the received data to the PC Host*/
		USBWriteChar(USART1->DR);
    }
  /* If overrun condition occurs, clear the ORE flag and recover communication */
  if((USART1->SR & USART_FLAG_ORE) != RESET)
    {
      (void)USART_ReceiveData(USART1);
    }

}
#endif
//--------------------------------------------GSM-----------------------------------------------
#if 0  
void USART2_IRQHandler(void)
{
     //u8 c;
    //if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	if((USART2->SR & USART_FLAG_RXNE) != RESET)
       {
	      //c = USART2->DR;
#ifdef BRIDGE_USB_GSM
          /* Send the received data to the PC Host*/
          //USART_To_USB_Send_Data(c);//bridge USB-COM2(GSM)
		  USBWriteChar(USART2->DR);
#else
		  gsmGetByte(USART2->DR);
#endif		  		    
	   }

    //if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
	
	//if((USART2->SR & USART_FLAG_TXE) != RESET)
	//       gsmSendRoutine();

    /* If overrun condition occurs, clear the ORE flag and recover communication */
    //if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
	if((USART2->SR & USART_FLAG_ORE) != RESET)
      {
        (void)USART_ReceiveData(USART2);
      }
	 
}
#endif
/*----------------------------------GSM with DMA-----------------------------------------------------*/
#ifdef BRIDGE_USB_GSM
void USART2_IRQHandler(void)
{
     //u8 c;
    //if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	if((USART2->SR & USART_FLAG_RXNE) != RESET)
       {
          /* Send the received data to the PC Host*/
		  USBWriteChar(USART2->DR);
	   }
    /* If overrun condition occurs, clear the ORE flag and recover communication */
	if((USART2->SR & USART_FLAG_ORE) != RESET)
      {
        (void)USART_ReceiveData(USART2);
      }
}
#endif
 //-----------------------------------------RS-485--------------------------------------------------

void USART3_IRQHandler(void)
{
	//u8 c;
	//if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	if((USART3->SR & USART_FLAG_RXNE) != RESET)
	{ 	
		rs485GetByte();    
	}
    /* If overrun condition occurs, clear the ORE flag and recover communication */
    //if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
	if((USART3->SR & USART_FLAG_ORE) != RESET)
      {
        (void)USART_ReceiveData(USART3);
      }

    //if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
	if((USART3->SR & USART_FLAG_TXE) != RESET)
	   {
	       rs485SendRoutine();
       }
    //if(USART_GetITStatus(USART3, USART_IT_TC) != RESET)
	if((USART3->SR & USART_IT_TC) != RESET)
	   {
	       //rs485SendRoutine();
		   rs485finish();
       }

}
/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/*******************************************************************************
* Function Name  : USB_HP_CAN1_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts requests
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
  CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}
/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{ 
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
  
}
/*******************************************************************************
* Function Name  : ADC1_2_IRQHandler
* Description    : This function handles ADC1 and ADC2 global interrupts requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC1_2_IRQHandler(void)
{
  /* Get injected channel13 converted value */
  adcProcessHandler();
 
}
/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void)
{
  // rtc_Handler();
}

//void TIM6_DAC_IRQHandler(void)
//{
    //tim6DACHandler();
//}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void TIM6_IRQHandler(void)
{
  tim6Handler();
}
/*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : This function handles DMA1 Channel 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
  /* Test on DMA1 Channel7 Transfer Complete interrupt */
  if(DMA_GetITStatus(DMA1_IT_TC7))
  {
    /* Get Current Data Counter value after complete transfer */
    //CurrDataCounterEnd = DMA_GetCurrDataCounter(DMA1_Channel7);
	handleDMA1Ch7();
    /* Clear DMA1 Channel7 Half Transfer, Transfer Complete and Global interrupt pending bits */
    DMA_ClearITPendingBit(DMA1_IT_GL7);
  }
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/*******************************************************************************
* Function Name  : PPP_IRQHandler
* Description    : This function handles PPP interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*void PPP_IRQHandler(void)
{
}*/

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

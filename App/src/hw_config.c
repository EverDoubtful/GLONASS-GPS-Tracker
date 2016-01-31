/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : hw_config.c
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
 
#include "hw_config.h"
#include "mass_mal.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "string.h"
#include "fifo.h"

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************/
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
	#if defined (VER_3)
      #define  MASK_USB_DET          (1 << 15)
	#else
	  #define  MASK_USB_DET          0x08
	#endif

/* Define the STM32F10x hardware depending on the used evaluation board */
#define USB_DISCONNECT                     GPIOD  
#if defined (VER_3)
 #define USB_DISCONNECT_PIN                GPIO_Pin_4
#else
 #define USB_DISCONNECT_PIN                GPIO_Pin_9
#endif
#define RCC_APB2Periph_GPIO_DISCONNECT    RCC_APB2Periph_GPIOD


	#define USB_DETECTOR                    GPIOD  
#if defined (VER_3)	
	#define USB_DETECTOR_PIN                GPIO_Pin_15
#else
	#define USB_DETECTOR_PIN                GPIO_Pin_3
#endif
	#define RCC_APB2Periph_GPIO_DETECTOR    RCC_APB2Periph_GPIOD
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ErrorStatus HSEStartUpStatus;
USART_InitTypeDef USART_InitStructure;

extern u8 signUSBMass;  // 1- mass storage, 0 - virtual com port

#define     USB_RX_DATA_SIZE      64
uint8_t  USART_Rx_Buffer [USB_RX_DATA_SIZE];   /* send data to USB*/

uint8_t  USB_Tx_State = 0;
/* These are 2 ring buffers*/
uint8_t     USB_RcvBuffer[USB_RING_RX_DATA_SIZE];  //USB_RX_DATA_SIZE was earlier
fifo_buffer USB_RxRB;

uint8_t     USB_TrBuffer[USB_RING_TX_DATA_SIZE]; 
fifo_buffer USB_TxRB;


u8  ENDP0_RXADDR;  //define from usb_conf.h
u8  ENDP0_TXADDR;  //define from usb_conf.h
u8  ENDP1_TXADDR;
u16 IMR_MSK;
u8  INTR_EOPFRAME;
DEVICE Device_Table;

#define EP_NUM_Mass                          (3)
#define EP_NUM_VCP                           (4)
DEVICE Device_Table_Mass =
  {
    EP_NUM_Mass,
    1
  };
DEVICE Device_Table_VCP =
  {
    EP_NUM_VCP,
    1
  };

//extern uint16_t USB_Rx_Cnt;
//extern uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
/* Extern variables ----------------------------------------------------------*/
extern LINE_CODING linecoding;

/* Private function prototypes -----------------------------------------------*/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void USBIniPin(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

//#if defined (VER_3)

    /* Configure IO connected to USB PWR DET *********************/	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DETECTOR, ENABLE);
    GPIO_InitStructure.GPIO_Pin = USB_DETECTOR_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  //ini
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(USB_DETECTOR, &GPIO_InitStructure);
  
//#endif

    /* Configure IO connected to USB PWR DET *********************/	
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT,ENABLE );
//    GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
//  	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
//  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  	GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);

}    
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u8  USBDetectPin(void)
{
    u8 res = 0;
    if((GPIO_ReadInputData(USB_DETECTOR) & MASK_USB_DET))   // USB is present after switch - work MASS Storage
	     res = 1;
	return res;
}                                              
#if defined (VER_3)
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
//void USB_PUPControl( u8 flag )
//{
//   if(flag == 1)
//      GPIO_SetBits(USB_PUP, USB_PUP_PIN);
//   else
//      GPIO_ResetBits(USB_PUP, USB_PUP_PIN);
//}
#endif
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void USBCommonIni(void)
{
	   /* these 4 functions for USB*/
	   Set_System();
	   Set_USBClock();
	   USB_Interrupts_Config();
	   USB_Init();
	#if defined (VER_3)
	   //USB_PUPControl(0);
	#endif
}                                         
/*******************************************************************************
* Function Name  : USB_To_USART_Send_Data.
* Description    : send the received data from USB to the UART 0.
* Input          : data_buffer: data address.
                   Nb_bytes: number of bytes to send.
* Return         : none.
*******************************************************************************/
void USB_To_USART_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes)
{
    ;
}

/*******************************************************************************
* Function Name  : Set_System
* Description    : Configures Main system clocks & power
* Input          : None.
* Return         : None.
*******************************************************************************/
void Set_System(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */ 
  if(signUSBMass)
    {
       ENDP0_RXADDR	= 0x18;
       ENDP0_TXADDR	= 0x58;
	   ENDP1_TXADDR = 0x98;
	   IMR_MSK = CNTR_CTRM  | CNTR_RESETM;
	   Device_Table = Device_Table_Mass;
	   INTR_EOPFRAME = 1;
    }
  else
    {
       ENDP0_RXADDR	= 0x40;
       ENDP0_TXADDR	= 0x80;
	   ENDP1_TXADDR = 0xC0;
       IMR_MSK = CNTR_CTRM  | CNTR_SOFM  | CNTR_RESETM;
	   Device_Table = Device_Table_VCP;
	   INTR_EOPFRAME = 0;
	   FIFO_Init(&USB_RxRB, USB_RcvBuffer, sizeof(USB_RcvBuffer));
       FIFO_Init(&USB_TxRB, USB_TrBuffer, sizeof(USB_TrBuffer));
    }
  /* Enable and Disconnect Line GPIO clock */
  USB_Disconnect_Config();
  
  
  /* MAL configuration */
  if(signUSBMass)
      MAL_Config();
}

/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz)
* Input          : None.
* Return         : None.
*******************************************************************************/
void Set_USBClock(void)
{
  /* Select USBCLK source */
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  
  /* Enable the USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
  /* Set the device state to suspend */
  bDeviceState = SUSPENDED;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  /* Set the device state to the correct state */
  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
  else
  {
    bDeviceState = ATTACHED;
  }

}

/*******************************************************************************
* Function Name  : USB_Interrupts_Config
* Description    : Configures the USB interrupts
* Input          : None.
* Return         : None.
*******************************************************************************/
void USB_Interrupts_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

		  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
		  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		  NVIC_Init(&NVIC_InitStructure);

  if(signUSBMass)
	  {
		
		  NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
		  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		  NVIC_Init(&NVIC_InitStructure);
		  
		  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
		  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		  NVIC_Init(&NVIC_InitStructure);
	  }
}

/*******************************************************************************
* Function Name  : USB_Cable_Config
* Description    : Software Connection/Disconnection of USB Cable.
* Input          : None.
* Return         : Status
*******************************************************************************/
void USB_Cable_Config (FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  }
  else
  {
    GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  }
}
/*                            WRITING                                   */
/*******************************************************************************/
void USBWriteChar(u8 ch)            /* one character */
{
    FIFO_Put(&USB_TxRB,ch);
}

/*******************************************************************************/

void USBWriteStr(const char * buf)   /* a string */
{
  u16 len = strlen(buf);
  //while(len--)
  //  FIFO_Put(&USB_TxRB,*buf++);
  ///*
  if(len < 10)
	  {
	    while(len--)
	       FIFO_Put(&USB_TxRB, *buf++);
      }
  else
	  {
	       FIFO_PutAr(&USB_TxRB, (u8 *)buf, len);
	  }
  //*/
}
/*******************************************************************************/

void USBWriteLen(u8 * buf, u16 len)  /*block of data*/
{
  //while(len--)
  //  FIFO_Put(&USB_TxRB,*buf++); 
  ///*
  if(len < 10)
	  {
	    while(len--)
	       FIFO_Put(&USB_TxRB, *buf++);
      }
  else
	  {
	       FIFO_PutAr(&USB_TxRB, buf, len);
	  }
   //*/
    
}
/*******************************************************************************/
void USBDebWrite(u32 dig)
{
   USBWriteChar(dig/1000 + 0x30);
   USBWriteChar((dig % 1000)/100 + 0x30);
   USBWriteChar((dig % 100)/10 + 0x30);
   USBWriteChar(dig % 10 + 0x30);
}
/*                            READING                                          */
/*******************************************************************************/

u16 USBReadWhole(u8 * buf)
{
  /*
  u16 nRead = 0;
  while(!FIFO_Empty(&USB_RxRB))
     {
      *buf++ = FIFO_Get(&USB_RxRB);
	  nRead++;
	 }
  return nRead;
  */
  ///*
 u16 nRead = 0;
 nRead = FIFO_GetAr(&USB_RxRB, buf);
 return nRead; 
  //*/
}

/*******************************************************************************/

u16 USBReadLen(u8 * buf, u8 len)
{
  u16 nRead = 0;
  while(len--)
   if(!FIFO_Empty(&USB_RxRB))
     {
      *buf++ = FIFO_Get(&USB_RxRB);
	  nRead++;
	 }
   else 
      break;
  return nRead;
}

/*******************************************************************************/
void USB_To_Outside_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes)
{
 
 #ifndef BRIDGE_USB_GSM
  while(Nb_bytes--)
     FIFO_Put(&USB_RxRB,*data_buffer++);
 #else
  u32 i;
  for (i = 0; i < Nb_bytes; i++)
   {
    while (!(USART2->SR & USART_FLAG_TXE));
    USART_SendData(USART2, *(data_buffer + i));
   }  
 #endif

}

/*******************************************************************************
* Function Name  : Handle_USBAsynchXfer.
* Description    : send data to USB.
* Input          : None.
* Return         : none.
*******************************************************************************/

void Handle_USBAsynchXfer (void)
{
     u8  USB_Tx_length = 0;
     while( (!FIFO_Empty(&USB_TxRB)) && (USB_Tx_length < VIRTUAL_COM_PORT_DATA_SIZE) )
      {
        USART_Rx_Buffer[USB_Tx_length++] = FIFO_Get(&USB_TxRB);
      }
     if(USB_Tx_length)
       {    
        UserToPMABufferCopy(USART_Rx_Buffer, ENDP1_TXADDR, USB_Tx_length);
        SetEPTxCount(ENDP1, USB_Tx_length);
        SetEPTxValid(ENDP1); 
       }
}

/*******************************************************************************
* Function Name  : UART_To_USB_Send_Data.
* Description    : send the received data from UART 0 to USB.
* Input          : None.
* Return         : none.
*******************************************************************************/
/*
void USART_To_USB_Send_Data(u8 byte)	
{
    FIFO_Put(&USB_TxRB, byte);
  
  //USART_Rx_Buffer[USART_Rx_ptr_in] = byte; 
  //USART_Rx_ptr_in++;
}
*/
/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(__IO uint32_t*)(0x1FFFF7E8);
  Device_Serial1 = *(__IO uint32_t*)(0x1FFFF7EC);
  Device_Serial2 = *(__IO uint32_t*)(0x1FFFF7F0);

  Device_Serial0 += Device_Serial2;

  if (Device_Serial0 != 0)
  {
    IntToUnicode (Device_Serial0, &MASS_StringSerial[2] , 8);
    IntToUnicode (Device_Serial1, &MASS_StringSerial[18], 4);
  }
}

/*******************************************************************************
* Function Name  : HexToChar.
* Description    : Convert Hex 32Bits value into char.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
  uint8_t idx = 0;
  
  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10; 
    }
    
    value = value << 4;
    
    pbuf[ 2* idx + 1] = 0;
  }
}

/*******************************************************************************
* Function Name  : MAL_Config
* Description    : MAL_layer configuration
* Input          : None.
* Return         : None.
*******************************************************************************/
void MAL_Config(void)
{
  MAL_Init(0);

#if defined(STM32F10X_HD) || defined(STM32F10X_XL) 
  /* Enable the FSMC Clock */
  //KAY
  //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
  //MAL_Init(1);
#endif /* STM32F10X_HD | STM32F10X_XL */
}

#if defined (USE_STM3210B_EVAL) || defined (USE_STM3210E_EVAL)
/*******************************************************************************
* Function Name  : USB_Disconnect_Config
* Description    : Disconnect pin configuration
* Input          : None.
* Return         : None.
*******************************************************************************/
void USB_Disconnect_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
#if 1
  /* Enable USB_DISCONNECT GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);
  /* USB_DISCONNECT_PIN used as USB pull-up */
  GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
#endif
}
#endif /* USE_STM3210B_EVAL or USE_STM3210E_EVAL */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

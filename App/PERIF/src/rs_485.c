/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "rs_485.h"
#include "common.h"
#include "gprs_modem.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define RS485_BUF_IO_SIZE        128
/* Private macro -------------------------------------------------------------*/
#define RS485_ON      GPIO_SetBits(RS485_DIR_PORT, RS485_DIR_PORT_PIN)
#define RS485_OFF     GPIO_ResetBits(RS485_DIR_PORT, RS485_DIR_PORT_PIN)

/* Private variables ---------------------------------------------------------*/
u8 buf_io_rs485[RS485_BUF_IO_SIZE];
volatile u16 rs485Cnt;
volatile u16 rs485BytesToSend;
u8 flagHasData;
//data for modem
u8 buf_io_modem[RS485_BUF_IO_SIZE];
volatile u16 rs485BytesForModem;
volatile u16 rs485BytesFromModem;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//-------------------------------------------------------------------------------------------
void USART3_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate            = 115200  ;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART3, ENABLE);     
}
//-------------------------------------------------------------------------------------------

void rs485GPIOInit(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_RS485_DIR,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_RS485_TX,ENABLE );

	/* Configure IO connected to DIR_485 *********************/	
	GPIO_InitStructure.GPIO_Pin =   RS485_DIR_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(RS485_DIR_PORT, &GPIO_InitStructure);

   	/* Configure USART3 Tx (PB.10) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin   =  RS485_TX_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_50MHz;
  	GPIO_Init(RS485_TX_PORT, &GPIO_InitStructure);
    
  	/* Configure USART3 Rx (PB.11) as input floating */
  	GPIO_InitStructure.GPIO_Pin = RS485_RX_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(RS485_RX_PORT, &GPIO_InitStructure);

}
//-------------------------------------------------------------------------------------------
void rs485Init(void)
{
   rs485GPIOInit();
   USART3_Configuration();
   RS485_OFF;
}
//-------------------------------------------------------------------------------------------
void rs485SendRoutine(void)
{

         //  Packets  from modem  - sending to pc-china board

         u8 c;
	    /* Write one byte to the transmit data register */
		 //if(!(RingBuffer_IsEmpty(&gsmTxRingBuffer)))
		 if(rs485Cnt < rs485BytesToSend)
		  {
		    
		    //c = RingBuffer_Remove(&gsmTxRingBuffer);
			c = buf_io_rs485[rs485Cnt++];
						   
	        USART_SendData(USART3,c );
	        /* Disable the USART1 Transmit interrupt */
			if(rs485Cnt ==  rs485BytesToSend - 1)  //last byte
			  {
			    rs485BytesToSend = 0;
			    rs485Cnt = 0;
	            USART_ITConfig(USART3, USART_IT_TC, ENABLE);
	            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
			  } 
		  }
}
//-------------------------------------------------------------------------------------------
void rs485finish(void)
{
    RS485_OFF;
    USART_ITConfig(USART3, USART_IT_TC, DISABLE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ClearITPendingBit(USART3, USART_IT_TC);   
}
//-------------------------------------------------------------------------------------------
void rs485GetByte(void)	   //receive from pc-chine board
{

   if(rs485Cnt == RS485_BUF_IO_SIZE) 
         rs485Cnt = 0;
   buf_io_rs485[rs485Cnt] = USART3->DR;
   if(buf_io_rs485[rs485Cnt] == '\n')	 //time to send to modem
     {
       //bufCopy(buf_io_rs485,buf_io_modem,rs485Cnt);
	   memcpy(buf_io_modem,buf_io_rs485,rs485Cnt);
	   rs485BytesForModem = rs485Cnt;
       USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
       rs485Cnt = 0; 
     }   
   else 
      rs485Cnt++;

}
//-------------------------------------------------------------------------------------------
void rs485Analyse(void)	  // send to modem and to rs485
{
   u8 c;
   //c = RecHndlRS485();
   //to rs-485
   if(rs485BytesFromModem)
     {
	   //flagHasData = 0;
	   //bufCopy(buf_io_modem,buf_io_rs485,rs485BytesFromModem);
	   memcpy(buf_io_rs485, buf_io_modem,rs485BytesFromModem);
	   rs485BytesToSend = rs485BytesFromModem;
	   rs485BytesFromModem = 0;
	   c = buf_io_rs485[0];
	   RS485_ON;
	   USART_SendData(USART3,c );
	   USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
     }

   // to modem
   if(rs485BytesForModem)
	 {
	    //gsmWrite(buf_io_modem, rs485BytesForModem, 0);
		cmdSendLen(buf_io_modem, rs485BytesForModem);
		rs485BytesForModem = 0;
	 }


}

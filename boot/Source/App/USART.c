#include "stm32f10x.h"
#include "USART.h"
#include "platform.h"
#include "string.h"

void erase_1page(void);
void write_1page(void);
void go_cmd(void);

void USART1_SendByte(u16 Data)
{ 
   while (!(USART1->SR & USART_FLAG_TXE));
   USART1->DR = (Data & (uint16_t)0x01FF);	 
}
void USART1Write(u8* data,u16 len)
{
	u16 i;
	for (i=0; i<len; i++){
		USART1_SendByte(data[i]);
	}
}	
void USART1SendBuf(u8* data)
{
	u16 i;
	u16 len = strlen((const char *)data);
	for (i=0; i<len; i++){
		USART1_SendByte(data[i]);
	}
}	

void USART1_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate            = 115200  ;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART1, ENABLE);     
}

volatile u16 rx_uart_count;
u8 rx_uart_buf[PAGE_SIZE];
volatile u8  rx_timeout;

void uart_decr_timer(void)
{
     if(rx_timeout)
      --rx_timeout; 
}
void rx_get_byte(void)
{
       if(rx_uart_count == PAGE_SIZE) 
         rx_uart_count = 0;
       rx_uart_buf[rx_uart_count] = USART1->DR; /// 
	   rx_uart_count++;
       rx_timeout = 2; /// 
}

void uart5_handle(void)
{
  u16 cpsr = __disable_irq();
  if((rx_uart_count > 0 && rx_timeout == 0))
     { 
		if(rx_uart_buf[0] == 'w')
		   write_1page();
		else if(rx_uart_buf[0] == 'g')
		   go_cmd();
		else if(rx_uart_buf[0] == 'r')
		   USART1_SendByte(rx_uart_buf[0]);

        rx_timeout = 20; /// 
        rx_uart_count = 0; ///  

      }
  if(!cpsr)
     __enable_irq();
}






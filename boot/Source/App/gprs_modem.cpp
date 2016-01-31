/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
//#include "USART.h"  // debug
#include "gprs_modem.h"
#include "fifo.h"
#include "platform.h"
#include "dio_led.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define GSM_TX_FIFO_BUF_SIZE    64    
#define GSM_RX_FIFO_BUF_SIZE    1024+10
   

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

u8  gsmRxRingBuffer[GSM_RX_FIFO_BUF_SIZE];
u8  gsmTxRingBuffer[GSM_TX_FIFO_BUF_SIZE];	   

fifo_buffer gsmRxRB;
fifo_buffer gsmTxRB;

u8 tempBuf[64];
extern u8 firmwareBuf[PAGE_SIZE];
extern u8 updateFlag;
extern u8 flagReadRawData;
       u8 flagPageDataReady;
extern u16 bytesFTPRealRead;
u16 byteCntPacket;
u16 bytesFTPToReadForPacket;

u16 byteCntPGSize;
extern u8  flagOpenFTP;
//rs485
//extern u8 buf_io_rs485[128];	  //todo 128 exact
//extern volatile u16 rs485BytesFromModem;
u8  volatile flagTx;
u16 volatile timeoutGSM;
u8  volatile flagNoResponce;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void gprsModemGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE );
 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_PWRKEY,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_POWER,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_STATUS,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_RING,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_SPKPWR,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_SIM1SEL,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_SIM2SEL,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_DCD,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_RTS,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_CTS,ENABLE );

    //GSM
		/* Configure USART2 Tx (PA.02) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  	/* Configure USART2 Rx (PA.03) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure IO connected to PWRKEY *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_PWRKEY_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_PWRKEY_PORT, &GPIO_InitStructure);
	/* Configure IO connected to PWR GPRS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_POWER_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_POWER_PORT, &GPIO_InitStructure);
	/* Configure IO connected to SPKPWR *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_SPKPWR_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_SPKPWR_PORT, &GPIO_InitStructure);
	/* Configure IO connected to SIM1SEL *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_SIM1SEL_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_SIM1SEL_PORT, &GPIO_InitStructure);
	/* Configure IO connected to SIM2SEL *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_SIM2SEL_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_SIM2SEL_PORT, &GPIO_InitStructure);
	/* Configure IO connected to RTS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_RTS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_RTS_PORT, &GPIO_InitStructure);

	/* Configure IO connected to STATUS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_STATUS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPRS_STATUS_PORT, &GPIO_InitStructure);
	/* Configure IO connected to RING *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_RING_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPRS_RING_PORT, &GPIO_InitStructure);
	/* Configure IO connected to DCD *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_DCD_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPRS_DCD_PORT, &GPIO_InitStructure);
	/* Configure IO connected to CTS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_CTS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPRS_CTS_PORT, &GPIO_InitStructure);

}

//-------------------------------------------------------------------------------------------
void gsmInterruptsConfig(void)
{
  	NVIC_InitTypeDef NVIC_InitStructure;
  	/* Configure the NVIC Preemption Priority Bits */  
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/* Enable the USART2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//-------------------------------------------------------------------------------------------

void USART2_Configuration(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate            = 115200  ;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);     
}
//-------------------------------------------------------------------------------------------
void gprsModemInit(void)
{
    gprsModemGPIOInit();
    USART2_Configuration();
	gsmInterruptsConfig();
	gsmRingBufInit();
}
//-------------------------------------------------------------------------------------------
void gprsModemOn(void)
{
    GPIO_SetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
	Delay(ONE_SEC);//1s
	//led_dn(BOARD_LED_OFF);
    GPIO_ResetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
	Delay(ONE_SEC*2);//1s
    GPIO_SetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);

}
//-------------------------------------------------------------------------------------------
void gsmRingBufInit(void)
{
	//RingBuffer_InitBuffer(&gsmRxRingBuffer);
    FIFO_Init(&gsmRxRB, gsmRxRingBuffer, sizeof(gsmRxRingBuffer));
    FIFO_Init(&gsmTxRB, gsmTxRingBuffer, sizeof(gsmTxRingBuffer));
     
	//RingBuffer_InitBuffer(&gsmTxRingBuffer);     
	flagTx = 0;     

}
//-------------------------------------------------------------------------------------------
void gprsModemOff(void)
{
    GPIO_ResetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
    //GPIO_SetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
    GPIO_ResetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
	//Delay(ONE_SEC*2);//1s

}
//-------------------------------------------------------------------------------------------

void checkForSend(void)
{
     u8 i;
    /* Write one byte to the transmit data register */
	 if(!FIFO_Empty(&gsmTxRB))
	 {
	   if(!flagTx)
	    {
		  led_dn(BOARD_LED_XOR); //debug
		  i = FIFO_Get(&gsmTxRB);
          //USART_SendData(UART4,i );
	      //USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
          USART_SendData(USART2,i );
	      USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

		  flagTx = 1;
		}
	 }
	 else flagTx = 0;
}

//-------------------------------------------------------------------------------------------
u8 cmdReceive(void)
{
   static u8 i = 0;
   u8 res = 0;
   static u8  pageNum = 0;
   u8 tb[2];
	while(!FIFO_Empty(&gsmRxRB))
    {
     tempBuf[i] = FIFO_Get(&gsmRxRB);
   
	 if(flagReadRawData)
	    {
  		  if(flagOpenFTP)
		   {
		    flagOpenFTP = 0;
		    pageNum = 0;
			byteCntPGSize = 0;
			
		   }

		   //we need to scan defined(exact)number of bytes
		  if(byteCntPacket < (bytesFTPRealRead))
		   {
		     byteCntPacket++;
		     firmwareBuf[byteCntPGSize] = tempBuf[i];
			 //USART1_SendByte(firmwareBuf[byteCntPGSize]);
			 byteCntPGSize++;

			 if ( 
			      (byteCntPGSize == FLASH_PAGE_SIZE)
				  // || 
			      //( (bytesFTPRealRead != FTP_PACKET_SIZE) && (byteCntPGSize == bytesFTPRealRead) )
			    )
				   {
				     //byteCntPGSize = 0;
					 flagPageDataReady = 1;
					 pageNum++;
			         //USART1Write((u8 *)"pg:", sizeof("pg:"));
	   				 int2char((char * )tb,pageNum,2,10);
					 //USART1_SendByte(tb[0]);
					 //USART1_SendByte(tb[1]);
					 //USART1_SendByte('\n');
				   }
		   }
		  else
		    {
			 bytesFTPToReadForPacket = bytesFTPToReadForPacket + bytesFTPRealRead;
			 if( bytesFTPToReadForPacket == FTP_PACKET_SIZE)
			     bytesFTPToReadForPacket = 0;
			 flagReadRawData = 0;
			 byteCntPacket = 0;
			}

		}
	 else if(!( (i==0) && ((tempBuf[i] == '\r' ) || (tempBuf[i] == '\n' ))))	  //missing forward bytes \r and \n
	   {
	     if( (i > 1) && (tempBuf[i-1] == '\r') && (tempBuf[i] == '\n') )
		      {
				 tempBuf[i-1] = '\0';
				 res = 1;
				 i = 0;
			     break;
			  }
		 else if((i == 0) && (tempBuf[i] == '>'))
		      {
			      res = 1;
				  i = 0;
				  break;
			  }
		 i++;
	   }


    }
  return res;
}

//-------------------------------------------------------------------------------------------
void gsmGetByte(void)
  {
			//FIFO_Put(&gsmRxRB,UART4->DR);
			 FIFO_Put(&gsmRxRB,USART2->DR);
  }
//-------------------------------------------------------------------------------------------
void gsmSendRoutine(void)
{
   //u8 c;
	    /* Write one byte to the transmit data register */
		 if(!FIFO_Empty(&gsmTxRB))
		  {
		    //c = RingBuffer_Remove(&gsmTxRingBuffer);
	        //USART_SendData(UART4,FIFO_Get(&gsmTxRB) );
	        USART_SendData(USART2,FIFO_Get(&gsmTxRB) );
		  }
		 else
	      {
	      /* Disable the USART1 Transmit interrupt */
	        //USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
	        USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
			flagTx = 0;
	      }

}

//-------------------------------------------------------------------------------------------
void gsmSendByte(u16 Data)
{ 
   while (!(USART2->SR & USART_FLAG_TXE));
   USART2->DR = (Data & (uint16_t)0x01FF);	 
}
//-------------------------------------------------------------------------------------------
void gsmWrite1Byte(u8 data)
{
	   //if(!(RingBuffer_IsFull(&gsmTxRingBuffer)))
		  //RingBuffer_Insert(&gsmTxRingBuffer, data);
	  FIFO_Put(&gsmTxRB, data);
}

//-------------------------------------------------------------------------------------------
void gsmWrite(u8* data,u8 len, u16 timeout)
{
	u16 i;
	for (i=0; i<len; i++)
	 {
	   FIFO_Put(&gsmTxRB, data[i]);
	 }
   setGSMTimeout(timeout);
}
//-------------------------------------------------------------------------------------------
void cmdSend(u8 * Buf, u8 len)
{
	u16 i;
	for (i=0; i<len; i++)
	   FIFO_Put(&gsmTxRB, Buf[i]);
}

//-------------------------------------------------------------------------------------------
void gsmSendPacket(u8* data,u16 len)
{
	u16 i;
	for (i=0; i<len; i++){
		gsmSendByte(data[i]);
	}
}	

//-------------------------------------------------------------------------------------------
void setGSMTimeout(u16 timeout)
{
    timeoutGSM = timeout;
	flagNoResponce = 0; 
}

//-------------------------------------------------------------------------------------------
void decrGSMTimeout(void)
{
	if(timeoutGSM)
	 {
       timeoutGSM--;
	   if(!timeoutGSM)
	      flagNoResponce = 1;
	 }
}
//-------------------------------------------------------------------------------------------
u8 getFlagNoResponce(void)
{
   return flagNoResponce;
}
// true  - 	we can put
//-------------------------------------------------------------------------------------------
u8 checkGSMTimeout(void)
{
   	return (!timeoutGSM)?1:0;
}
//-------------------------------------------------------------------------------------------
void gprsSIM1Sel( void )	 //MASTER 
{
      GPIO_SetBits(GPRS_SIM1SEL_PORT, GPRS_SIM1SEL_PORT_PIN);
      GPIO_ResetBits(GPRS_SIM2SEL_PORT, GPRS_SIM2SEL_PORT_PIN);
}
//-------------------------------------------------------------------------------------------
void gprsSIM2Sel( void )	  //SLAVE
{
      GPIO_SetBits(GPRS_SIM2SEL_PORT, GPRS_SIM2SEL_PORT_PIN);
      GPIO_ResetBits(GPRS_SIM1SEL_PORT, GPRS_SIM1SEL_PORT_PIN);
}

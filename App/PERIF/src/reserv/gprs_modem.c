/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "gprs_modem.h"
#include "gprs_modemInner.h"
#include "common.h"
#include "dio_led.h"
#include "fifo.h"
#include "string.h"
#include "ftp.h"
#include "bin_handle.h"
#include "fifoPackets.h"
#include "hw_config.h"
#include "protocol_bin.h"
#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define GSM_TX_FIFO_BUF_SIZE    2048 //512    
#define GSM_RX_FIFO_BUF_SIZE    64
const u8 * GSM_DATA_APPEAR[] = {"+IPD,"};
#define REPL_IPD_LEN strlen((char *)GSM_DATA_APPEAR[0])
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8  gsmRxRingBuffer[GSM_RX_FIFO_BUF_SIZE];
u8  gsmTxRingBuffer[GSM_TX_FIFO_BUF_SIZE];	   

fifo_buffer gsmRxRB;
fifo_buffer gsmTxRB;

u8 tempBuf[REC_AN_BUF_SIZE];

//rs485
extern u8 buf_io_rs485[128];	  //todo 128 exact
extern volatile u16 rs485BytesFromModem;
u8  volatile flagTx;
u16 volatile timeoutGSM;
u8  volatile flagNoResponce;
                  
extern u8 flagReadRawData;
//extern u8 bufExtBin[SIZE_EXT_BIN];

extern node_struct   nodesRcvPack[FIFO_PACKET_RECV_SIZE];
extern fifo_packet   fifoRecBufPack;
extern t_innerState innerState;

static u8  flagStartPacket = 0;
static u8  flagWaitDecideEcho = 0;
static u8  flagDataAppear = 0;
static u8  flagDataRead = 0;
static u8  flagWeAreCatchingPacket = 0;


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
void gsmRingBufInit(void)
{
    FIFO_Init(&gsmRxRB, gsmRxRingBuffer, sizeof(gsmRxRingBuffer));
    FIFO_Init(&gsmTxRB, gsmTxRingBuffer, sizeof(gsmTxRingBuffer));
	flagTx = 0;     

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
void gprsModemOn(u8 sel)
{
    //gprsSIM2Sel();
	if(sel == 0)
	   gprsSIMSelect(SIM_MASTER);
	else
	   gprsSIMSelect(SIM_SLAVE);
	Delay(ONE_SEC/2);
    GPIO_SetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
	Delay(ONE_SEC);//1s
	//led_dn(BOARD_LED_OFF);
    GPIO_ResetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
	Delay(ONE_SEC*2);//1s
    GPIO_SetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);

}
//-------------------------------------------------------------------------------------------
void gprsChangeSIM(void)
{
   innerState.activeSIMCard ^= 1;
   gprsModemOff();
   gprsModemOn(innerState.activeSIMCard);
}
//-------------------------------------------------------------------------------------------
void gprsModemOff(void)
{
    GPIO_ResetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
	//Delay(ONE_SEC);//1s
    //GPIO_ResetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
	//Delay(ONE_SEC*2);//1s
    GPIO_ResetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);

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
   		  i = FIFO_Get(&gsmTxRB);
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


   static u16 idx = 0;
   u8 res = 0;
   static u16  posBeginPacket = 0;
   static u16  posRN = 0;
   static u16 lenDataAppear;
   static u16 lenDataRead;
   static u8  numberOfDigitsRead;
   static u16 lenDataOrder;
   //u16 tempo;

   while(!FIFO_Empty(&gsmRxRB))
     {
	   /* Get 1 byte from buffer*/
       tempBuf[idx] = FIFO_Get(&gsmRxRB);
	   /* Read raw data by FTP*/
       if(flagReadRawData)
	    {
		  //USBWriteChar('c');
          checkReadRawData(tempBuf[idx]);
		}
	   else	 /* Replies from modem and queries from server*/
        {
		  //USBWriteChar('h');
          if( (idx) && (tempBuf[idx] == '\n') && (tempBuf[idx-1] == '\r') && (!flagWeAreCatchingPacket))	 /* found start or end of packet */  
    	    {
			 //USBWriteChar('h');
			 //if
			   //{
			    // USBWriteChar('i');
	    	     if(flagStartPacket)       /* found end of packet (answer from AT cmd)*/
	               {
	                 flagStartPacket = 0;
	                 memmove(&tempBuf[0],&tempBuf[posBeginPacket+2],idx-posBeginPacket-3);
	                 tempBuf[idx-posBeginPacket-3] = '\0';
					 res = 1;
					 idx = 0;
	                 posRN = 0;
					 USBWriteStr("a0-");
					 USBWriteStr((char *)tempBuf);		 //debugga
					 USBWriteStr("\r\n");
				     break;
	               }
	             flagWaitDecideEcho = 1;   /*we are hesitate is it start of usual packet or end of echo packet*/
	             posBeginPacket  = idx - 1;
	             if( (idx > posRN) && (posRN) )
	              {
				    //USBWriteChar('b');		 //debugga
	                if((idx - posRN) == 2)   /* found two \r\n side by side (podryad) - it means there is echo*/
	                  {
	                    //posRN = 0;
	                    flagStartPacket = 1;
	                    flagWaitDecideEcho = 0;
	                    posBeginPacket  = idx - 1; /* start with \r\n */
						//USBWriteChar('c');		 //debugga
	                  }
	                
	              }
	             posRN  = idx;
			   //}
            }
    	  else
            {                       /* confirmation it is start of packet*/
               if( ((posBeginPacket+3) == idx) && (flagWaitDecideEcho) )
                {
                  flagWaitDecideEcho = 0;
                  if( (tempBuf[idx-1] != '\r') && (tempBuf[idx] != '\n') )
                    {
					  //USBWriteChar('d');		 //debugga
                      flagStartPacket = 1;
                    }
                }
               if(flagStartPacket)  
                {
                  if(flagDataRead)
                   {
                     
                     lenDataRead++;
					 //USBWriteChar('p');		 //debugga
					 /* we have scanned all bytes from parsel and copy it in receive buffer*/
                     if(lenDataRead == lenDataAppear)
                       {
                         flagDataRead = 0;
                         lenDataAppear = 0;
                         flagStartPacket = 0; 
						 /*Handling of all kinds*/
		                 lenDataRead = unstuffData(&tempBuf[lenDataOrder+numberOfDigitsRead+1],lenDataRead);
						 /*0 in this call is naviSig =0 - means we don't save it in SD-card*/
						 //fifoPacketPut(&fifoRecBufPack,&tempBuf[lenDataOrder+numberOfDigitsRead+1], lenDataRead, 0);
						 //debugga
						 //USBWriteLen(&tempBuf[lenDataOrder+numberOfDigitsRead+1], lenDataRead);
						 //USBWriteChar('\r');
						 //USBWriteChar('\n');
						 //USBWriteChar('A');		 //debugga
                         lenDataRead = 0;
						 flagWeAreCatchingPacket = 0;
						 res = 0;  //add
						 idx = 0;
						 memset(tempBuf,0,sizeof(tempBuf));
						 break;	   //add
                       }
                   }
                  else if(flagDataAppear)
                   {
                      if(tempBuf[idx] == ':')	 /* found : in +IPD,XXX:data bytes...*/
                        {
						  //USBWriteChar(':');		 //debugga
						  //USBWriteChar(tempBuf[lenDataOrder]);		 //debugga
						  //USBWriteChar(tempBuf[lenDataOrder+1]);		 //debugga
                          lenDataAppear = atoi((const char *)&tempBuf[lenDataOrder]); /* found XXX number  of bytes*/
                          numberOfDigitsRead = getLenNum(lenDataAppear);
                          //flagStartPacket = 0;
						  //USBWriteChar(lenDataAppear/10 + 0x30);		 //debugga
						  //USBWriteChar(lenDataAppear%10 + 0x30);		 //debugga
                          flagDataRead = 1;
                          flagDataAppear = 0;

						  //USBWriteChar('g');		 //debugga
                        }																		
                   }
                  else if((!strncmp((char *)GSM_DATA_APPEAR[0],(char *)&tempBuf[posBeginPacket+2],REPL_IPD_LEN)))  /* "+IPD," catched*/
                    {
                      //led_dn(BOARD_LED_XOR);
                      
                      flagDataAppear  = 1;
                      lenDataOrder   = idx+1;
					  flagWeAreCatchingPacket = 1;
					  //if(idx == 3)
					  //   tempo = 4;
					  //USBWriteChar('f');		 //debugga
					  //USBWriteChar(idx/100 + 0x30);		 //debugga
					  //tempo = idx%100;
					  //USBWriteChar(idx/10 + 0x30);		 //debugga
					  //USBWriteChar(idx%10 + 0x30);		 //debugga
					  //USBWriteChar(tempo +);
 
                    }
                  /* finding '>' */
                  if( (tempBuf[idx-1] == '>') && (tempBuf[idx] == ' ') )	 
                   {
                     res = 1;
    				 idx = 0;
                     tempBuf[0] = '>';
                     flagStartPacket = 0;
					 //USBWriteChar('e');		 //debugga
    				 break;
                   }

                }	 //if(flagStartPacket)
            }		//else
             if(++idx == REC_AN_BUF_SIZE)  //protection just in case
                idx = 0;
    	    
        }
     }		  //while(!FIFO_Empty(&gsmRxRB))
   return res;
}

//-------------------------------------------------------------------------------------------
void cmdSendLen(u8 * Buf, u16 len)
{
	u16 i;
	for (i=0; i<len; i++)
	   FIFO_Put(&gsmTxRB, Buf[i]);
}
//-------------------------------------------------------------------------------------------
void cmdSend(u8 * Buf)
{
	u16 i;
	u16 len;
	len = strlen((const char *)Buf);
	for (i=0; i<len; i++)
	   FIFO_Put(&gsmTxRB, Buf[i]);
}

//-------------------------------------------------------------------------------------------
void gsmGetByte(u8 c)
  {
	FIFO_Put(&gsmRxRB,c);
  }
//-------------------------------------------------------------------------------------------
void gsmSendRoutine(void)
{
	    /* Write one byte to the transmit data register */
		 if(!FIFO_Empty(&gsmTxRB))
		  {
	        USART_SendData(USART2,FIFO_Get(&gsmTxRB) );
		  }
		 else
	      {
	      /* Disable the USART1 Transmit interrupt */
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
void gprsSIMSelect( sim_select_type sel )	  
{
    if(sel == SIM_MASTER)  //master
	  {
        GPIO_SetBits(GPRS_SIM1SEL_PORT, GPRS_SIM1SEL_PORT_PIN);
        GPIO_ResetBits(GPRS_SIM2SEL_PORT, GPRS_SIM2SEL_PORT_PIN);
		//innerState.activeSIMCard = SIM_MASTER;
	  } 
	else   //slave
	  {
	    GPIO_SetBits(GPRS_SIM2SEL_PORT, GPRS_SIM2SEL_PORT_PIN);
        GPIO_ResetBits(GPRS_SIM1SEL_PORT, GPRS_SIM1SEL_PORT_PIN);
	    //innerState.activeSIMCard = SIM_SLAVE;
	  }
}

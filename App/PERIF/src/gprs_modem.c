/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "gprs_modem.h"
#include "gprs_modemInner.h"
#include "common.h"
#include "dio_led.h"
#include "fifo.h"
#include "string.h"
#include "ftp.h"
#include "bkp.h"
#include "bin_handle.h"
#include "fifoPackets.h"
#include "hw_config.h"
#include "protocol_bin.h"
#include <stdlib.h>
/* Private typedef -----------------------------------------------------------*/
			    
/* Private define ------------------------------------------------------------*/
#define GSM_TX_FIFO_BUF_SIZE    2048 
#define GSM_RX_FIFO_BUF_SIZE    2048
const u8 * GSM_DATA_APPEAR[] = {
								"+IPD,",
								"+CMT: "
								};
#define REPL_IPD_LEN          strlen((char *)GSM_DATA_APPEAR[0])
#define REPL_CMT_SMS_LEN      strlen((char *)GSM_DATA_APPEAR[1])
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8  gsmRxRingBuffer[GSM_RX_FIFO_BUF_SIZE];
u8  gsmTxRingBuffer[GSM_TX_FIFO_BUF_SIZE];	   

fifo_buffer gsmRxRB;
fifo_buffer gsmTxRB;

u8 tempBuf[REC_AN_BUF_SIZE];

//rs485
extern u8 buf_io_rs485[];	  // 128
extern volatile u16 rs485BytesFromModem;
u8  volatile flagTx;
u16 volatile timeoutGSM;
u8  volatile flagNoResponce;
                  
extern u8 flagReadRawData;

extern node_struct   nodesRcvPack[];   //FIFO_PACKET_RECV_SIZE
extern t_innerState innerState;

static u8  flagStartPacket = 0;
static u8  flagWaitDecideEcho = 0;
static u8  flagDataAppear = 0;
static u8  flagDataRead = 0;
static u8  flagWeAreCatchingPacket = 0;

//sms
static u8 flagCatchSMSHead = 0;
static u8 flagCatchSMSBody = 0;
//turn on modem
volatile u32 timerModemValue;
volatile u8  flagModemOn1;
volatile u8  flagModemOn2;
volatile u8  flagModemOn3;
volatile u8  flagModemOn4;

volatile u32 timerProtNoSendValue;

//extern u8   bufTxGSM[];
//extern u16  cntTxGSM;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void gprsModemGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE );
 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_PWRKEY,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_POWER,ENABLE );

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_SIM1SEL,ENABLE );
#if defined (VER_3)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GSM_SPKON,ENABLE );
#else
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_SIM2SEL,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_STATUS,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_RING,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_SPKPWR,ENABLE );
#endif

	    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_DCD,ENABLE );

    //GSM
#if 1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_RTS,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_CTS,ENABLE );
	/* Configure USART2 Tx (PA.02) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  	/* Configure USART2 Rx (PA.03) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

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
	/* Configure IO connected to SIM1SEL *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_SIM1SEL_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_SIM1SEL_PORT, &GPIO_InitStructure);
#if defined (VER_3)
	/* Configure IO connected to GSM.SPKON *********************/	
	GPIO_InitStructure.GPIO_Pin = GSM_SPKON_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GSM_SPKON_PORT, &GPIO_InitStructure);
#else
	/* Configure IO connected to SIM2SEL *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_SIM2SEL_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_SIM2SEL_PORT, &GPIO_InitStructure);
	/* Configure IO connected to SPKPWR *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_SPKPWR_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_SPKPWR_PORT, &GPIO_InitStructure);
	/* Configure IO connected to STATUS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_STATUS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPRS_STATUS_PORT, &GPIO_InitStructure);
	/* Configure IO connected to RING *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_RING_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPRS_RING_PORT, &GPIO_InitStructure);
#endif

#if 1
	/* Configure IO connected to RTS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_RTS_PORT_PIN;
  	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_RTS_PORT, &GPIO_InitStructure);
	/* Configure IO connected to DCD *********************/	
	//GPIO_InitStructure.GPIO_Pin = GPRS_DCD_PORT_PIN;
  	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	//GPIO_Init(GPRS_DCD_PORT, &GPIO_InitStructure);
	/* Configure IO connected to CTS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_CTS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPRS_CTS_PORT, &GPIO_InitStructure);
#endif
}
//-------------------------------------------------------------------------------------------
void gprsAfterInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_RTS,ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_GPRS_CTS,ENABLE );
	/* Configure IO connected to RTS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_RTS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPRS_RTS_PORT, &GPIO_InitStructure);
	/* Configure IO connected to CTS *********************/	
	GPIO_InitStructure.GPIO_Pin = GPRS_CTS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPRS_CTS_PORT, &GPIO_InitStructure);
	/* Configure USART2 Tx (PA.02) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  	/* Configure USART2 Rx (PA.03) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);

 
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
    /* Enable DMA1 channel7 IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
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
	#ifdef BRIDGE_USB_GSM
	     USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	#endif
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
	#ifdef BRIDGE_USB_GSM
	   gsmInterruptsConfig();
	#endif
	gsmRingBufInit();
	USART2_TX_DMA_Init();
	#ifndef BRIDGE_USB_GSM
	   USART2_RX_DMA_Init();
	#endif
	innerState.flagDebug = 1;   //debugga
	//innerState.flagTmpDebug = 1;
}
//-------------------------------------------------------------------------------------------
void gprsChangeSIM(void)
{
   u32 temp;
   innerState.activeSIMCard ^= 1;
   //innerState.currentServer - for company
   temp = innerState.activeSIMCard | (innerState.currentServer << 1);
   BKPWriteReg(BKP_DR_INNER_STATE_N1,temp);
   BKPWriteReg(BKP_DR_INNER_STATE_N2,temp >> 16);

   gprsModemOff();
   gprsModemOn(innerState.activeSIMCard);
}

//-------------------------------------------------------------------------------------------
void gprsModemOn(u8 sel)
{
/*
    if(innerState.flagDebug)
		 USBWriteStr("ModemON\r\n");
    
	if(sel == 0)
	   gprsSIMSelect(SIM_MASTER);
	else
	   gprsSIMSelect(SIM_SLAVE);
	Delay(ONE_SEC/2);
    GPIO_SetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
	Delay(ONE_SEC);//1s
    GPIO_ResetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
	Delay(ONE_SEC*2);//1s
    GPIO_SetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
#if defined (VER_3)	
    setDutyCycle(8);
#endif
   innerState.dataSt = ModemOn;
*/

    if(innerState.flagDebug)
		 USBWriteStr("ModemON\r\n");
    
	if(sel == 0)
	   gprsSIMSelect(SIM_MASTER);
	else
	   gprsSIMSelect(SIM_SLAVE);
	setTimerModemOn(ONE_SEC/2);

	flagModemOn1  = 0;
	flagModemOn2  = 0;
	flagModemOn3  = 0;
	flagModemOn4  = 0;
}
//-------------------------------------------------------------------------------------------
void gprsModemOff(void)
{
    if(innerState.flagDebug)
		 USBWriteStr("ModemOFF\r\n");

	#ifndef VER_31
       GPIO_ResetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
	#else
       GPIO_SetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
	#endif

    GPIO_ResetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
    innerState.dataSt = ModemOff;

}
//-------------------------------------------------------------------------------------------
u8 cmdReceive(void)
{

   static u16 idx = 0;
   u8 res = 0;
   static u16  posBeginPacket = 0;
   static u16  posRN = 0;
   static u16  lenDataAppear;
   static u16  lenDataRead;
   static u8   numberOfDigitsRead;
   static u16  lenDataOrder;
   u8 C1 = 0, C2 = 0;
   static u16  prevCNDT = sizeof(gsmRxRingBuffer);
   u16 temp;
   DMA1_Channel6->CCR  &= ~DMA_CCR6_EN;      //запретить работу канала
   if(prevCNDT >= DMA1_Channel6->CNDTR)
       temp = (prevCNDT - DMA1_Channel6->CNDTR);
   else
       temp = (prevCNDT + sizeof(gsmRxRingBuffer) - DMA1_Channel6->CNDTR);
   FIFO_MovePtr(&gsmRxRB,temp);
   prevCNDT = DMA1_Channel6->CNDTR;
   DMA1_Channel6->CCR  |=  DMA_CCR6_EN;      //разрешить работу канала

   while(!FIFO_Empty(&gsmRxRB))
     {
	   /* Get 1 byte from buffer*/
       tempBuf[idx] = FIFO_Get(&gsmRxRB);
	   if(innerState.flagDebug)
	      USBWriteChar(tempBuf[idx]);
	   /* Read raw data by FTP*/
       if(flagReadRawData)
	    {
          checkReadRawData(tempBuf[idx]);
		}
	   else	 /* Replies from modem and queries from server*/
        {
		  /* found start or end of packet */
          if( (idx) && (tempBuf[idx] == '\n') && (tempBuf[idx-1] == '\r') && (!flagWeAreCatchingPacket) )	   
    	    {
	    	     if( (flagStartPacket) || (flagCatchSMSBody) )      /* found end of packet (answer from AT cmd)*/
	               {
				     C1 = 2;
					 C2 = 0;
					 if(flagCatchSMSBody)
					   {
					    C1 = 0;
						C2 = 2;
					   }
	                 flagStartPacket = 0;
	                 //memmove(&tempBuf[0],&tempBuf[posBeginPacket+2],idx-posBeginPacket-3);
	                 memmove(&tempBuf[0],&tempBuf[posBeginPacket+C1],idx-posBeginPacket-3 + C2);
	                 tempBuf[idx-posBeginPacket-3+C2] = '\0';


//					 if(innerState.flagDebug)
//					   {
//						 USBWriteStr("R:");
//						 USBWriteStr((char *)tempBuf);
//						 USBWriteStr("\r\n");
//					   }
					 res = 1;
					 idx = 0;
	                 posRN = 0;
					 
					 if(flagCatchSMSBody)
					  {
					   flagCatchSMSBody = 0;
					  }
					 if(flagCatchSMSHead)
					   {
					     flagCatchSMSHead = 0;
						 flagCatchSMSBody = 1;
					   }
				     break;
	               }
	             flagWaitDecideEcho = 1;   /*we are hesitate is it start of usual packet or end of echo packet*/
	             posBeginPacket  = idx - 1;
	             if( (idx > posRN) && (posRN) )
	              {
	                if((idx - posRN) == 2)   /* found two \r\n side by side (podryad) - it means there is echo*/
	                  {
	                    //posRN = 0;
	                    flagStartPacket = 1;
	                    flagWaitDecideEcho = 0;
	                    posBeginPacket  = idx - 1; /* start with \r\n */
	                  }
	                
	              }
	             posRN  = idx;
            }
    	  else
            {                       /* confirmation it is start of packet*/
               if( ((posBeginPacket+3) == idx) && (flagWaitDecideEcho) )
                {
                  flagWaitDecideEcho = 0;
                  if( (tempBuf[idx-1] != '\r') && (tempBuf[idx] != '\n') )
                    {
                      flagStartPacket = 1;
                    }
                }
               if(flagStartPacket)  
                {
                  if(flagDataRead)
                   {
                     lenDataRead++;
					 /* we have scanned all bytes from parsel and copy it in receive buffer*/
                     if(lenDataRead == lenDataAppear)
                       {
//						if(innerState.flagDebug)
//						 {
//						  USBWriteStr("Rcv:");
//					      USBDebWrite(lenDataRead);
//						  USBWriteStr("\r\n");
//						 }
                         flagDataRead = 0;
                         lenDataAppear = 0;
                         flagStartPacket = 0; 
						 /*Handling of all kinds*/
		                 lenDataRead = unstuffData(&tempBuf[lenDataOrder+numberOfDigitsRead+1],lenDataRead);
						 /*0 in this call is naviSig =0 - means we don't save it in SD-card*/
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
                          lenDataAppear = atoi((const char *)&tempBuf[lenDataOrder]); /* found XXX number  of bytes*/
                          numberOfDigitsRead = getLenNum(lenDataAppear);
                          flagDataRead = 1;
                          flagDataAppear = 0;

						  //USBWriteStr("toRead:");
					      //USBDebWrite(lenDataAppear);
						  //USBWriteStr("\r\n");
                        }																		
                   }
                  else if((!strncmp((char *)GSM_DATA_APPEAR[0],(char *)&tempBuf[posBeginPacket+2],REPL_IPD_LEN)))  /* "+IPD," catched*/
                    {
                      flagDataAppear  = 1;
                      lenDataOrder   = idx+1;
					  flagWeAreCatchingPacket = 1;
					  //USBWriteStr("IPD catched\r\n");
					  
                    }
				  /*Handling SMS*/
				  else if(!flagCatchSMSHead)
	                  if((!strncmp((char *)GSM_DATA_APPEAR[1],(char *)&tempBuf[posBeginPacket+2],REPL_CMT_SMS_LEN)))  /* "+CMT: " catched*/
	                    {
						  flagCatchSMSHead = 1;
	                    }
	                  /* finding '>' */
                  else if( (tempBuf[idx-1] == '>') && (tempBuf[idx] == ' ') )	 
                   {
                     res = 1;
    				 idx = 0;
                     tempBuf[0] = '>';
                     flagStartPacket = 0;
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
void gsmWrite1Byte(u8 data)
{
     FIFO_Put(&gsmTxRB, data);
	 //cntTxGSM %= MAX_SIZE_PACKET_GPRS_SIM900;
     //bufTxGSM[cntTxGSM++] = data;
}
//-------------------------------------------------------------------------------------------
void cmdSendLen(u8 * buf, u16 len) /* Sent block of data*/
{
    
	if(len < 10)
	  {
	    while(len--)
	       FIFO_Put(&gsmTxRB, *buf++);
      }
	else
	  {
	       FIFO_PutAr(&gsmTxRB, buf, len);
	  }

}
//-------------------------------------------------------------------------------------------
void cmdSend(u8 * buf)			/* Sent string*/
{
	u16 len = strlen((const char *)buf);
    if(innerState.flagDebug)
	  {
        USBWriteStr("S:");
	    USBWriteStr((char *)buf);
	  }
	if(len < 10)
	  {
	    while(len--)
	       FIFO_Put(&gsmTxRB, *buf++);
      }
	else
	  {
	       FIFO_PutAr(&gsmTxRB, buf, len);
	  }

}
//-------------------------------------------------------------------------------------------
void checkForSend(void)
{
	u16 cnt = 0;
	if(!FIFO_Empty(&gsmTxRB))
	  {
		StartDMAChannel7( FIFO_PtrStart(&gsmTxRB, &cnt), cnt );     //запустить первую передачу
		/* Protection if there was not one sending then we count timer(15min for example) and restart modem*/
		setTimerProtNoSend(ONE_SEC * 60 * 15);  //15min
	  }

}

//-------------------------------------------------------------------------------------------
void gsmGetByte(u8 c)
  {
	FIFO_Put(&gsmRxRB,c);
  }
//-------------------------------------------------------------------------------------------
void gprsSIMSelect( sim_select_type sel )	  
{
#if defined (VER_3)
    if(sel == SIM_MASTER)  //master
	  {
		GPIO_ResetBits(GPRS_SIM1SEL_PORT, GPRS_SIM1SEL_PORT_PIN);
	  } 
	else   //slave
	  {
		GPIO_SetBits(GPRS_SIM1SEL_PORT, GPRS_SIM1SEL_PORT_PIN);
	  }
#else
    if(sel == SIM_MASTER)  //master
	  {
        GPIO_SetBits(GPRS_SIM1SEL_PORT, GPRS_SIM1SEL_PORT_PIN);
        GPIO_ResetBits(GPRS_SIM2SEL_PORT, GPRS_SIM2SEL_PORT_PIN);
	  } 
	else   //slave
	  {
	    GPIO_SetBits(GPRS_SIM2SEL_PORT, GPRS_SIM2SEL_PORT_PIN);
        GPIO_ResetBits(GPRS_SIM1SEL_PORT, GPRS_SIM1SEL_PORT_PIN);
	  }
#endif
}
//-------------------------------------------------------------------------------------------
void GSMSpeaker(u8 sw)
{
#if defined (VER_3)
    if(sw == 1)  //ON
	  {
		GPIO_SetBits(GSM_SPKON_PORT, GSM_SPKON_PORT_PIN);
	  } 
	else   //OFF
	  {
		GPIO_ResetBits(GSM_SPKON_PORT, GSM_SPKON_PORT_PIN);
	  }
#endif  
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleTimerGPRSModem(void)
{
    if(timerModemValue)
	  {
	   timerModemValue--;
	   if(!timerModemValue )
	     {
		   if(!flagModemOn1)
		     {
		       flagModemOn1 = 1; 
			   #ifndef VER_31
			      GPIO_SetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
			   #else
			      GPIO_ResetBits(GPRS_POWER_PORT, GPRS_POWER_PORT_PIN);
			   #endif
			   setTimerModemOn(ONE_SEC);
		     }
		   else	if(!flagModemOn2)
		     {
		       flagModemOn2 = 1; 
			   GPIO_ResetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
			   setTimerModemOn(ONE_SEC*2);
		     }
		   else	if(!flagModemOn3)
		     {
		       flagModemOn3 = 1; 
			   GPIO_SetBits(GPRS_PWRKEY_PORT, GPRS_PWRKEY_PORT_PIN);
			   #if defined (VER_3)	
				    setDutyCycle(8);
			   #endif
			   innerState.dataSt = ModemOn;
			   //setTimerModemOn(ONE_SEC * 10);   //deb
		     }
		   else	if(!flagModemOn4)		 //deb
		     {
		       flagModemOn4 = 1;
			   USART2_Configuration();
			   gsmInterruptsConfig();
			   gprsAfterInit();
		     }
		 }
	  }

    if(timerProtNoSendValue)
	  {
	   timerProtNoSendValue--;
	   if(!timerProtNoSendValue )
	     {
		   innerState.flagProtNoSend = 1;
		 }
	  }

}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerModemOn(u32 val)
{
   	timerModemValue = val;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerProtNoSend(u32 val)
{
   	timerProtNoSendValue = val;
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
//unsigned char BuffTxd[32];        //буфер передатчика (с него родимого и будем брать данные)
//********************************************************************************
//Function: инициализация DMA1 для работы с USART2 (передача данных)            //
//********************************************************************************
void USART2_TX_DMA_Init( void )
{
 //Включить тактирование DMA1
 if ((RCC->AHBENR & RCC_AHBENR_DMA1EN) != RCC_AHBENR_DMA1EN)
      RCC->AHBENR |= RCC_AHBENR_DMA1EN;
 //Задать адрес источника и приемника и количество данных для обмена
 DMA1_Channel7->CPAR  =  (u32)&USART2->DR;   //адрес регистра перефирии
 //----------------- Манипуляции с регистром конфигурации  ----------------
 //Следующие действия можно обьединить в одну команду (разбито для наглядности)
 DMA1_Channel7->CCR   =  0;                       //предочистка регистра конфигурации
 DMA1_Channel7->CCR  &= ~DMA_CCR7_CIRC;           //выключить циклический режим
 DMA1_Channel7->CCR  |=  DMA_CCR7_DIR;            //направление: чтение из памяти
 //Настроить работу с переферийным устройством
 DMA1_Channel7->CCR  &= ~DMA_CCR7_PSIZE;          //размерность данных 8 бит
 DMA1_Channel7->CCR  &= ~DMA_CCR7_PINC;           //неиспользовать инкремент указателя
 //Настроить работу с памятью
 DMA1_Channel7->CCR  &= ~DMA_CCR7_MSIZE;          //размерность данных 8 бит
 DMA1_Channel7->CCR  |=  DMA_CCR7_MINC;           //использовать инкремент указателя
 USART2->CR3         |=  USART_CR3_DMAT;          //разрешить передачу USART1 через DMA
   /* Enable DMA1 Channel7 Transfer Complete interrupt */
 DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

}
//********************************************************************************
//Function: старт обмена в канале "память-DMA-USART1"                           //
//Argument: количество данных к обмену                                          //
//********************************************************************************
void StartDMAChannel7( u8 * buf, unsigned int LengthBufer )
{
  DMA1_Channel7->CCR  &= ~DMA_CCR7_EN;      //запретить работу канала
  DMA1_Channel7->CMAR  =  (uint32_t)buf;   //адрес буфера в памяти
  DMA1_Channel7->CNDTR =  LengthBufer;      //загрузить количество данных для обмена
  DMA1->IFCR          |=  DMA_IFCR_CTCIF7;  //сбросить флаг окончания обмена
  DMA1_Channel7->CCR  |=  DMA_CCR7_EN;      //разрешить работу канала
}
//********************************************************************************
//Function: проверка флага окончания обмена в канале "память-DMA-USART1"        //
//Result  : 0 - обмен не закончен; 1 - обмен закончен;                          //
//********************************************************************************
unsigned char GetStateDMAChannel7(void)
{
  if(DMA1->ISR & DMA_ISR_TCIF7) return 1;   //обмен окончен
  return 0;                                 //обмен продолжается
}
//********************************************************************************
//																				**
//********************************************************************************
void handleDMA1Ch7(void)
{
  u16 cnt = 0;
  if(!FIFO_Empty(&gsmTxRB))
     StartDMAChannel7(FIFO_PtrStart(&gsmTxRB, &cnt), cnt);     //запустить первую передачу
}
//********************************************************************************
//Function: инициализация DMA1 для работы с USART2 (прием данных)            //
//********************************************************************************
static void USART2_RX_DMA_Init( void )
{
 //Включить тактирование DMA1
 if ((RCC->AHBENR & RCC_AHBENR_DMA1EN) != RCC_AHBENR_DMA1EN)
      RCC->AHBENR |= RCC_AHBENR_DMA1EN;
 //Задать адрес источника и приемника и количество данных для обмена
 DMA1_Channel6->CPAR  =  (u32)&USART2->DR;   //адрес регистра перефирии
 DMA1_Channel6->CMAR  =  (uint32_t)gsmRxRingBuffer;   //адрес буфера в памяти
 DMA1_Channel6->CNDTR =  sizeof(gsmRxRingBuffer);     //size of buffer

 //----------------- Манипуляции с регистром конфигурации  ----------------
 //Следующие действия можно обьединить в одну команду (разбито для наглядности)
 DMA1_Channel6->CCR   =  0;                       //предочистка регистра конфигурации
 DMA1_Channel6->CCR  |=  DMA_CCR6_CIRC;           //включить циклический режим
 DMA1_Channel6->CCR  &=  ~DMA_CCR6_DIR;            //направление: запись в память
 //Настроить работу с переферийным устройством
 DMA1_Channel6->CCR  &= ~DMA_CCR6_PSIZE;          //размерность данных 8 бит
 DMA1_Channel6->CCR  &= ~DMA_CCR6_PINC;           //неиспользовать инкремент указателя
 //Настроить работу с памятью
 DMA1_Channel6->CCR  &= ~DMA_CCR6_MSIZE;          //размерность данных 8 бит
 DMA1_Channel6->CCR  |=  DMA_CCR6_MINC;           //использовать инкремент указателя
 USART2->CR3         |=  USART_CR3_DMAR;          //разрешить прием USART2 через DMA
 DMA1_Channel6->CCR  |=  DMA_CCR6_EN;             //разрешить работу канала

}

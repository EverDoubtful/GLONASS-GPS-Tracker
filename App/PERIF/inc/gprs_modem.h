
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPRS_MODEM_H
#define __GPRS_MODEM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

						      
/* Exported types ------------------------------------------------------------*/
typedef enum _sim_select
{
   SIM_MASTER = 0,
   SIM_SLAVE
}sim_select_type;
				      
/* Exported constants --------------------------------------------------------*/


#define REC_AN_BUF_SIZE         (MAX_SIZE_PACKET_GPRS_SIM900 + 40) //was 64	 debugga


/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

void gprsModemInit(void);
void checkForSend(void);
void gsmGetByte(u8 c);
void gsmSendRoutine(void);
void gprsModemOn(u8 sel);
void gprsModemOff(void);
void gsmWrite(u8* data,u8 len, u16 timeout);
void gsmWrite1Byte(u8 data);
void gprsSIMSelect( sim_select_type sel );
void gprsChangeSIM(void);
void GSMSpeaker(u8 sw);
void handleTimerGPRSModem(void);
//u8 RecHndlRS485(void);
void USART2_TX_DMA_Init( void );
unsigned char GetStateDMAChannel7(void);
void StartDMAChannel7( u8 * buf, unsigned int LengthBufer );
void handleDMA1Ch7(void);

//-----------------new 


void cmdSend(u8 * Buf);
void cmdSendLen(u8 * Buf, u16 len);
u8   cmdReceive(void);



//------------------


#ifdef __cplusplus
}
#endif

#endif /* __GPRS_MODEM_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

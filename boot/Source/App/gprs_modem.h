
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPRS_MODEM_H
#define __GPRS_MODEM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

						      
/* Exported types ------------------------------------------------------------*/
				      
/* Exported constants --------------------------------------------------------*/
#define GPRS_PWRKEY_PORT                          GPIOD  
#define GPRS_PWRKEY_PORT_PIN                      GPIO_Pin_5
#define RCC_APB2Periph_GPIO_GPRS_PWRKEY           RCC_APB2Periph_GPIOD

#define GPRS_POWER_PORT                           GPIOD  
#define GPRS_POWER_PORT_PIN                       GPIO_Pin_10
#define RCC_APB2Periph_GPIO_GPRS_POWER            RCC_APB2Periph_GPIOD

#define GPRS_STATUS_PORT                          GPIOD  
#define GPRS_STATUS_PORT_PIN                      GPIO_Pin_6
#define RCC_APB2Periph_GPIO_GPRS_STATUS           RCC_APB2Periph_GPIOD
							
#define GPRS_RING_PORT                            GPIOD  
#define GPRS_RING_PORT_PIN                        GPIO_Pin_7
#define RCC_APB2Periph_GPIO_GPRS_RING             RCC_APB2Periph_GPIOD

#define GPRS_SPKPWR_PORT                          GPIOD  
#define GPRS_SPKPWR_PORT_PIN                      GPIO_Pin_11
#define RCC_APB2Periph_GPIO_GPRS_SPKPWR           RCC_APB2Periph_GPIOD

#define GPRS_SIM1SEL_PORT                          GPIOB  
#define GPRS_SIM1SEL_PORT_PIN                      GPIO_Pin_2
#define RCC_APB2Periph_GPIO_GPRS_SIM1SEL           RCC_APB2Periph_GPIOB
#define GPRS_SIM2SEL_PORT                          GPIOB  
#define GPRS_SIM2SEL_PORT_PIN                      GPIO_Pin_3
#define RCC_APB2Periph_GPIO_GPRS_SIM2SEL           RCC_APB2Periph_GPIOB

#define GPRS_DCD_PORT                          GPIOA  
#define GPRS_DCD_PORT_PIN                      GPIO_Pin_4
#define RCC_APB2Periph_GPIO_GPRS_DCD           RCC_APB2Periph_GPIOA
#define GPRS_RTS_PORT                          GPIOA  
#define GPRS_RTS_PORT_PIN                      GPIO_Pin_1
#define RCC_APB2Periph_GPIO_GPRS_RTS           RCC_APB2Periph_GPIOA
#define GPRS_CTS_PORT                          GPIOA  
#define GPRS_CTS_PORT_PIN                      GPIO_Pin_0
#define RCC_APB2Periph_GPIO_GPRS_CTS           RCC_APB2Periph_GPIOA



/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

void gprsModemGPIOInit(void);
void USART2_Configuration(void);
void gprsModemInit(void);
void gsmInterruptsConfig(void);
void gsmRingBufInit(void);
void checkForSend(void);
//u8   checkReceive(void);
void gsmSendByte(u16 Data);
void gsmWrite(u8* data,u8 len, u16 timeout);
void gsmWrite1Byte(u8 data);
void gsmSendPacket(u8* data,u16 len);
void setGSMTimeout(u16 timeout);
void decrGSMTimeout(void);
u8   getFlagNoResponce(void);
u8   checkGSMTimeout(void);
void gsmGetByte(void);
void gsmSendRoutine(void);
void gprsModemOn(void);
void gprsModemOff(void);
void gprsSIM1Sel( void );
void gprsSIM2Sel( void );
u8   RecHndlRS485(void);

void cmdSend(u8 * Buf, u8 len);
u8   cmdReceive(void);
//void readRawData(void);

void Delay(__IO uint32_t nTime);

int int2char(char * pbuf, int data, const char field_size, const char base);

#ifdef __cplusplus
}
#endif

#endif /* __GPRS_MODEM_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

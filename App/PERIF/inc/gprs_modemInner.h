
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPRS_MODEM_INNER_H
#define __GPRS_MODEM_INNER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

						      
/* Exported types ------------------------------------------------------------*/
				      
/* Exported constants --------------------------------------------------------*/
#if defined (VER_3)
	#define GPRS_PWRKEY_PORT                          GPIOA  
	#define GPRS_PWRKEY_PORT_PIN                      GPIO_Pin_6
	#define RCC_APB2Periph_GPIO_GPRS_PWRKEY           RCC_APB2Periph_GPIOA
#else
	#define GPRS_PWRKEY_PORT                          GPIOD  
	#define GPRS_PWRKEY_PORT_PIN                      GPIO_Pin_5
	#define RCC_APB2Periph_GPIO_GPRS_PWRKEY           RCC_APB2Periph_GPIOD
#endif

#define GPRS_POWER_PORT                               GPIOD  
#if defined (VER_3)
    #define GPRS_POWER_PORT_PIN                       GPIO_Pin_13
#else
    #define GPRS_POWER_PORT_PIN                       GPIO_Pin_10
#endif
#define RCC_APB2Periph_GPIO_GPRS_POWER                RCC_APB2Periph_GPIOD

#if defined (VER_3)
	#define GSM_SPKON_PORT                            GPIOE  
	#define GSM_SPKON_PORT_PIN                        GPIO_Pin_2
	#define RCC_APB2Periph_GPIO_GSM_SPKON             RCC_APB2Periph_GPIOE
#else
	#define GPRS_STATUS_PORT                          GPIOD  
	#define GPRS_STATUS_PORT_PIN                      GPIO_Pin_6
	#define RCC_APB2Periph_GPIO_GPRS_STATUS           RCC_APB2Periph_GPIOD
								
	#define GPRS_RING_PORT                            GPIOD  
	#define GPRS_RING_PORT_PIN                        GPIO_Pin_7
	#define RCC_APB2Periph_GPIO_GPRS_RING             RCC_APB2Periph_GPIOD
	
	#define GPRS_SPKPWR_PORT                          GPIOD  
	#define GPRS_SPKPWR_PORT_PIN                      GPIO_Pin_11
	#define RCC_APB2Periph_GPIO_GPRS_SPKPWR           RCC_APB2Periph_GPIOD
#endif

#if defined (VER_3)
	#define GPRS_SIM1SEL_PORT                          GPIOE  
	#define GPRS_SIM1SEL_PORT_PIN                      GPIO_Pin_8
	#define RCC_APB2Periph_GPIO_GPRS_SIM1SEL           RCC_APB2Periph_GPIOE
#else
	#define GPRS_SIM1SEL_PORT                          GPIOB  
	#define GPRS_SIM1SEL_PORT_PIN                      GPIO_Pin_2
	#define RCC_APB2Periph_GPIO_GPRS_SIM1SEL           RCC_APB2Periph_GPIOB
	#define GPRS_SIM2SEL_PORT                          GPIOB  
	#define GPRS_SIM2SEL_PORT_PIN                      GPIO_Pin_3
	#define RCC_APB2Periph_GPIO_GPRS_SIM2SEL           RCC_APB2Periph_GPIOB
#endif


//#define GPRS_DCD_PORT                          GPIOA  
//#define GPRS_DCD_PORT_PIN                      GPIO_Pin_4
//#define RCC_APB2Periph_GPIO_GPRS_DCD           RCC_APB2Periph_GPIOA

#define GPRS_RTS_PORT                          GPIOA  
#if defined (VER_3)
	#define GPRS_RTS_PORT_PIN                      GPIO_Pin_0
#else
	#define GPRS_RTS_PORT_PIN                      GPIO_Pin_1
#endif
#define RCC_APB2Periph_GPIO_GPRS_RTS           RCC_APB2Periph_GPIOA

#define GPRS_CTS_PORT                          GPIOA  
#if defined (VER_3)
	#define GPRS_CTS_PORT_PIN                      GPIO_Pin_1
#else
	#define GPRS_CTS_PORT_PIN                      GPIO_Pin_0
#endif
#define RCC_APB2Periph_GPIO_GPRS_CTS           RCC_APB2Periph_GPIOA

void USART2_Configuration(void);
void gprsModemGPIOInit(void);
void gsmInterruptsConfig(void);
void gsmRingBufInit(void);

//void setGSMTimeout(u16 timeout);
//void decrGSMTimeout(void);
//u8   getFlagNoResponce(void);
//u8   checkGSMTimeout(void);
void setTimerModemOn(u32 val);
void setTimerProtNoSend(u32 val);

//void gprsSIM1Sel( void );
//void gprsSIM2Sel( void );
static void USART2_RX_DMA_Init( void );

//------------------


#ifdef __cplusplus
}
#endif

#endif /* __GPRS_MODEM_INNER_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

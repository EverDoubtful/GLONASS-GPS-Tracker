
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DIO_LED_H
#define __DIO_LED_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef enum  {
  BOARD_LED_OFF = 0,
  BOARD_LED_ON,
  BOARD_LED_XOR  
} led_action;
typedef enum  {
  DOUT_OFF = 0,
  DOUT_ON    
} dout_action;
/* Exported constants --------------------------------------------------------*/
//LEDS
#define LED_MID_PORT                          GPIOD  
#if defined (VER_3)
	#define LED_MID_PORT_PIN                      GPIO_Pin_6
#else
	#define LED_MID_PORT_PIN                      GPIO_Pin_14
#endif
#define RCC_APB2Periph_GPIO_LED_MID           RCC_APB2Periph_GPIOD

#define LED_DN_PORT                           GPIOD  
#if defined (VER_3)
	#define LED_DN_PORT_PIN                       GPIO_Pin_7
#else
	#define LED_DN_PORT_PIN                       GPIO_Pin_15
#endif
#define RCC_APB2Periph_GPIO_LED_DN            RCC_APB2Periph_GPIOD
#if defined (VER_3)
	#define LED_UP_PORT                           GPIOD  
	#define LED_UP_PORT_PIN                       GPIO_Pin_5
	#define RCC_APB2Periph_GPIO_LED_UP            RCC_APB2Periph_GPIOD
#endif

//TTL DOUT
#define TTL_DOUT1_PORT                        GPIOB  
#define TTL_DOUT1_PORT_PIN                    GPIO_Pin_0
#define RCC_APB2Periph_GPIO_TTL_DOUT1         RCC_APB2Periph_GPIOB
#define TTL_DOUT2_PORT                        GPIOB  
#define TTL_DOUT2_PORT_PIN                    GPIO_Pin_1
#define RCC_APB2Periph_GPIO_TTL_DOUT2         RCC_APB2Periph_GPIOB
//DIN
#define TTL_DIN1_PORT                         GPIOE  
#define TTL_DIN1_PORT_PIN                     GPIO_Pin_0
#define RCC_APB2Periph_GPIO_TTL_DIN1          RCC_APB2Periph_GPIOE
#define TTL_DIN2_PORT                         GPIOE  
#define TTL_DIN2_PORT_PIN                     GPIO_Pin_1
#define RCC_APB2Periph_GPIO_TTL_DIN2          RCC_APB2Periph_GPIOE
//BUTTONS
#define BUT_TAMPER_PORT                         GPIOC  
#define BUT_TAMPER_PORT_PIN                     GPIO_Pin_13
#define RCC_APB2Periph_GPIO_BUT_TAMPER          RCC_APB2Periph_GPIOC

/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void led_mid( led_action act );
void led_dn( led_action act);
void ttl_dout1( dout_action act );
void ttl_dout2( dout_action act );
void din1_read(void);
void din2_read(void);

void ledDioGPIOInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __DIO_LED_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

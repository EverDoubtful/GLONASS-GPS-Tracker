
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RS_485_H
#define __RS_485_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x_lib.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
//DIR
#define RS485_DIR_PORT                         GPIOD  
#define RS485_DIR_PORT_PIN                     GPIO_Pin_13
#define RCC_APB2Periph_GPIO_RS485_DIR          RCC_APB2Periph_GPIOD

#define RS485_TX_PORT                         GPIOB  
#define RS485_TX_PORT_PIN                     GPIO_Pin_10
#define RCC_APB2Periph_GPIO_RS485_TX          RCC_APB2Periph_GPIOB
#define RS485_RX_PORT                         GPIOB  
#define RS485_RX_PORT_PIN                     GPIO_Pin_11
#define RCC_APB2Periph_GPIO_RS485_RX          RCC_APB2Periph_GPIOB

/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

void USART3_Configuration(void);
void rs485GetByte(void);
void rs485SendRoutine(void);
void rs485Init(void);
void rs485Analyse(void);
void rs485finish(void);
#ifdef __cplusplus
}
#endif

#endif /* __RS_485_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

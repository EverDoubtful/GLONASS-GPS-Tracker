
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RFMODEM_H
#define __RFMODEM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#if defined (VER_3)

	#define RFMODEM_TXD_PORT                     GPIOE  
	#define RFMODEM_TXD_PORT_PIN                 GPIO_Pin_0
	#define RCC_APB2Periph_GPIO_RFMODEM_TXD      RCC_APB2Periph_GPIOE

	#define RFMODEM_RXD_PORT                     GPIOE  
	#define RFMODEM_RXD_PORT_PIN                 GPIO_Pin_1
	#define RCC_APB2Periph_GPIO_RFMODEM_RXD      RCC_APB2Periph_GPIOE

	#define RFMODEM_TXD_CLK_PORT                 GPIOC  
	#define RFMODEM_TXD_CLK_PORT_PIN             GPIO_Pin_6
	#define RCC_APB2Periph_GPIO_RFMODEM_TXD_CLK  RCC_APB2Periph_GPIOC

	#define RFMODEM_RXD_CLK_PORT                 GPIOC  
	#define RFMODEM_RXD_CLK_PORT_PIN             GPIO_Pin_7
	#define RCC_APB2Periph_GPIO_RFMODEM_RXD_CLK  RCC_APB2Periph_GPIOC

	#define RFMODEM_PTT_PORT                     GPIOE  
	#define RFMODEM_PTT_PORT_PIN                 GPIO_Pin_4
	#define RCC_APB2Periph_GPIO_RFMODEM_PTT      RCC_APB2Periph_GPIOE

	#define RFMODEM_DIR_PORT                     GPIOB  
	#define RFMODEM_DIR_PORT_PIN                 GPIO_Pin_8
	#define RCC_APB2Periph_GPIO_RFMODEM_DIR      RCC_APB2Periph_GPIOB
#endif

/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void rfmodemInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __RFMODEM_H */


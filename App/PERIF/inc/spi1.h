
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI1_H
#define __SPI1_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x_lib.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
//
#define SPI1_SCK_PORT                         GPIOA  
#define SPI1_SCK_PORT_PIN                     GPIO_Pin_5
#define RCC_APB2Periph_GPIO_SPI1_SCK          RCC_APB2Periph_GPIOA
#define SPI1_MISO_PORT                        GPIOA  
#define SPI1_MISO_PORT_PIN                    GPIO_Pin_6
#define RCC_APB2Periph_GPIO_SPI1_MISO         RCC_APB2Periph_GPIOA
#define SPI1_MOSI_PORT                        GPIOA  
#define SPI1_MOSI_PORT_PIN                    GPIO_Pin_7
#define RCC_APB2Periph_GPIO_SPI1_MOSI         RCC_APB2Periph_GPIOA


/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

void spi1_init_pins(void);

#ifdef __cplusplus
}
#endif

#endif /* __SPI1_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

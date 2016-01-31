
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C1_H
#define __I2C1_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm32f10x_lib.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
//
#define I2C1_SDA_PORT                         GPIOB  
#define I2C1_SDA_PORT_PIN                     GPIO_Pin_7
#define RCC_APB2Periph_GPIO_I2C1_SDA          RCC_APB2Periph_GPIOB
#define I2C1_SCL_PORT                         GPIOB  
#define I2C1_SCL_PORT_PIN                     GPIO_Pin_6
#define RCC_APB2Periph_GPIO_I2C1_SCL          RCC_APB2Periph_GPIOB


/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

void i2c1_init_pins(void);

#ifdef __cplusplus
}
#endif

#endif /* __I2C1_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

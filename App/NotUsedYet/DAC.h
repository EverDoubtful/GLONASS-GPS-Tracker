
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DAC_H
#define __DAC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#if defined (VER_3)
	#define DACOUT_PORT                     GPIOA  
	#define DACOUT_PORT_PIN                 GPIO_Pin_4
	#define RCC_APB2Periph_GPIO_DACOUT      RCC_APB2Periph_GPIOA

	#define DACSPK_PORT                     GPIOE  
	#define DACSPK_PORT_PIN                 GPIO_Pin_3
	#define RCC_APB2Periph_GPIO_DACSPK      RCC_APB2Periph_GPIOE

#endif

/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void DACInit(void);
void DACSpeaker(u8 sw);
void tim6DACHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __DAC_H */


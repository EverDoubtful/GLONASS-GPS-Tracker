
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IBUTTON_H
#define __IBUTTON_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#if defined (VER_3)
	#define IBUTTON_PORT                     GPIOB  
	#define IBUTTON_PORT_PIN                 GPIO_Pin_9
	#define RCC_APB2Periph_GPIO_IBUTTON      RCC_APB2Periph_GPIOB
#endif

/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

void ibuttonInit(void);

#ifdef __cplusplus
}
#endif

#endif /* __IBUTTON_H */


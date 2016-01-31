/* Includes ------------------------------------------------------------------*/
#if defined (VER_3)
#include "stm32f10x.h"
#include "ibutton.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void ibuttonInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_IBUTTON, ENABLE);
	/* Configure IO connected to DIN1 *********************/	
	GPIO_InitStructure.GPIO_Pin = IBUTTON_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(IBUTTON_PORT, &GPIO_InitStructure);
  
}
//-------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "spi1.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void spi1_init_pins(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Configure IO connected to SCK *********************/	
	GPIO_InitStructure.GPIO_Pin =   SPI1_SCK_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(SPI1_SCK_PORT, &GPIO_InitStructure);
	/* Configure IO connected to MOSI *********************/	
	GPIO_InitStructure.GPIO_Pin =   SPI1_MOSI_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStructure);
    
	/* Configure IO connected to MISO *********************/	
	GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(SPI1_MISO_PORT, &GPIO_InitStructure);

}

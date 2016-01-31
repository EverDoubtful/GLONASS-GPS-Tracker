/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "i2c1.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void i2c1_init_pins(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Configure IO connected to SCK *********************/	
	GPIO_InitStructure.GPIO_Pin =   I2C1_SCL_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStructure);
	/* Configure IO connected to MOSI *********************/	
	GPIO_InitStructure.GPIO_Pin =   I2C1_SDA_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(I2C1_SDA_PORT, &GPIO_InitStructure);
}

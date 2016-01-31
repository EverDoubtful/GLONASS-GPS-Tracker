/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "dio_led.h"
//#include "common.h"
void GPIO_XorBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//-------------------------------------------------------------------------------------------

void led_mid( led_action act)
{
   if(act == BOARD_LED_ON)
      GPIO_SetBits(LED_MID_PORT, LED_MID_PORT_PIN);
   else	if(act == BOARD_LED_OFF)
      GPIO_ResetBits(LED_MID_PORT, LED_MID_PORT_PIN);
   else 
      GPIO_XorBits(LED_MID_PORT, LED_MID_PORT_PIN);
}
//-------------------------------------------------------------------------------------------
void led_dn( led_action act)
{
   if(act == BOARD_LED_ON)
      GPIO_SetBits(LED_DN_PORT, LED_DN_PORT_PIN);
   else	if(act == BOARD_LED_OFF)
      GPIO_ResetBits(LED_DN_PORT, LED_DN_PORT_PIN);
   else 
      GPIO_XorBits(LED_DN_PORT, LED_DN_PORT_PIN);

}
//-------------------------------------------------------------------------------------------
void ttl_dout1( dout_action act )
{
   if(act == DOUT_ON)
      GPIO_SetBits(TTL_DOUT1_PORT, TTL_DOUT1_PORT_PIN);
   else
      GPIO_ResetBits(TTL_DOUT1_PORT, TTL_DOUT1_PORT_PIN);
  
}
//-------------------------------------------------------------------------------------------
void ttl_dout2( dout_action act )
{
   if(act == DOUT_ON)
      GPIO_SetBits(TTL_DOUT2_PORT, TTL_DOUT2_PORT_PIN);
   else
      GPIO_ResetBits(TTL_DOUT2_PORT, TTL_DOUT2_PORT_PIN);
  
}
//-------------------------------------------------------------------------------------------
void din1_read(void)
{
  GPIO_ReadInputDataBit(TTL_DIN1_PORT, TTL_DIN1_PORT_PIN);
}
//-------------------------------------------------------------------------------------------
void din2_read(void)
{
  GPIO_ReadInputDataBit(TTL_DIN2_PORT, TTL_DIN2_PORT_PIN);
}
//-------------------------------------------------------------------------------------------
void ledDioGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_LED_MID, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_LED_DN, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_TTL_DOUT1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_TTL_DOUT2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_TTL_DIN1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_TTL_DIN2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_BUT_TAMPER, ENABLE);

	/* Configure IO connected to LED_NAV *********************/	
	GPIO_InitStructure.GPIO_Pin =   LED_MID_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(LED_MID_PORT, &GPIO_InitStructure);
	/* Configure IO connected to LED_PWR *********************/	
	GPIO_InitStructure.GPIO_Pin =   LED_DN_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(LED_DN_PORT, &GPIO_InitStructure);
	//TTL DOUTS
	/* Configure IO connected to TTL_DOUT1 *********************/	
	GPIO_InitStructure.GPIO_Pin =   TTL_DOUT1_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(TTL_DOUT1_PORT, &GPIO_InitStructure);
	/* Configure IO connected to TTL_DOUT2 *********************/	
	GPIO_InitStructure.GPIO_Pin =   TTL_DOUT2_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(TTL_DOUT2_PORT, &GPIO_InitStructure);
    //DINS
	/* Configure IO connected to DIN1 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_DIN1_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(TTL_DIN1_PORT, &GPIO_InitStructure);
	/* Configure IO connected to DIN2 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_DIN2_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(TTL_DIN2_PORT, &GPIO_InitStructure);
	//BUTTONS
	/* Configure IO connected to TAMPER *********************/	
	GPIO_InitStructure.GPIO_Pin = BUT_TAMPER_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(BUT_TAMPER_PORT, &GPIO_InitStructure);

}
//-------------------------------------------------------------------------------------------

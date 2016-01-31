/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "timers.h"
#include "dio_led.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
//-------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void timer6Init(void)
{
  TIM_TimeBaseInitTypeDef base_timer;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  TIM_TimeBaseStructInit(&base_timer);

  //base_timer.TIM_Prescaler = 48000 - 1;    //1ms
  //base_timer.TIM_Period = 1000;			 //1s
  base_timer.TIM_Prescaler = 480 - 1;		//10mks
  base_timer.TIM_Period = 50000;			// 50000 * 10mks =  500ms
  TIM_TimeBaseInit(TIM6, &base_timer);

  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM6, ENABLE);

  NVIC_EnableIRQ(TIM6_IRQn);
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void tim6Handler(void)
{
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
   	//led_dn(BOARD_LED_XOR);
}

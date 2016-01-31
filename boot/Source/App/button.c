/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "button.h"
#include "platform.h"
//#include "dio_led.h"
//#include "common.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 buttonCode;
u32 btnTimeout;
u8  flagBtn;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
// ------------------------------------------------------------------------------
void setButtonUserPressed(u8 code)
{ 
  buttonCode |= code;
}
// ------------------------------------------------------------------------------

u8 getButtonUserPressed(void)
{ 
  u8 res = 0;
  if(buttonCode & USER_BUTTON)
   {
     buttonCode &= ~USER_BUTTON;
	 res = 1;
   }
  else if(buttonCode & WAKEUP_BUTTON)
   {
     buttonCode &= ~WAKEUP_BUTTON;
	 res = 2;
   }

  return res;
}
// ------------------------------------------------------------------------------

u8 buttonPressed(void)
{
  return buttonCode;
}
// ------------------------------------------------------------------------------
void handleTimerBtn(void)
{
   if(btnTimeout)
    {
      btnTimeout--;
	  if(!btnTimeout)
	   {
		 setButtonUserPressed(USER_BUTTON);
		 //led_mid(BOARD_LED_XOR);	   //for debug
	   }
	}
}

// ------------------------------------------------------------------------------
void goTimerBtn(u32 delay)
{
  btnTimeout = delay;
  //flagBtn = 0;
}
// ------------------------------------------------------------------------------
u8 getFlagBtn(void)
{
   return flagBtn;
}
// ------------------------------------------------------------------------------
void buttonScan(void)
{
  u16 locBit;

      locBit = ( GPIO_ReadInputData(GPIOC) & 0x2000 );
	  
	  if( locBit ) 	  //no jumper
	      {
		    if(flagBtn)
			   flagBtn = 0;
		  }
      else	 //jumper
	      {
		    if(!flagBtn)
			 {
	           flagBtn = 1;
			   goTimerBtn(ONE_SEC * 1);
			 }
		 }

}
// ------------------------------------------------------------------------------


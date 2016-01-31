

#include "stm32f10x.h"
#include "common.h"
#include "dio_led.h"

u32 timerValueIWDG;
u8  flagIWDGReload = 0;
static __IO uint32_t TimingDelay;


//ETM
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

//-------------------------------------------------------------------------------------------
/*
void GPIO_XorBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));
    
  if(GPIOx->ODR & GPIO_Pin)
     GPIOx->BRR = GPIO_Pin;
  else 
     GPIOx->BSRR = GPIO_Pin;
}
*/
//-------------------------------------------------------------------------------------------

u8 strhex2int(u8 * buf)
{
  u8 res = 0;
  u8 sym = *buf++;

  if(sym >= '0' && sym <= '9')
      sym = sym - '0'; 
  else if(sym >= 'a' && sym <= 'f')
      sym = sym - 'a' + 10; 
  else if((sym >= 'A') && (sym <= 'F'))
      sym = sym - 'A' + 10;
	   
  res = sym << 4;
  sym = *buf++;

  if(sym >= '0' && sym <= '9')
      sym = sym - '0'; 
  else if(sym >= 'a' && sym <= 'f')
      sym = sym - 'a' + 10; 
  else if((sym >= 'A') && (sym <= 'F'))
      sym = sym - 'A' + 10; 

  return (res+sym);
}
//-------------------------------------------------------------------------------------------
int hex2char(int _C)
{
  int x = _C & 0x0f;
  if(x >= 0 && x <= 9)
    return x+'0';
  else
    return x-10+'A';
}
//-------------------------------------------------------------------------------------------

int int2char(char * pbuf, int data, const char field_size, const char base)
{
  u8 i;
  pbuf += field_size;
  
  for (i = 0; i<field_size; i++)
  {
    if (base == 10)
    {
      *(--pbuf) = data%10+'0';
      data = data/10;
    }
    else if (base==16)
    {
      *(--pbuf) = hex2char(data);
      data = data>>4;
    }
    else
      return -2;
  }
  return 1;
}
//-------------------------------------------------------------------------------------------
/*
void bufCopy(const u8 * src,u8 * dst, u32 len)
{
 if(len)
  while(len--)
    *dst++ = *src++;

}
*/
//-------------------------------------------------------------------------------------------

u8 bufCmpUser(const u8 * src,const u8 * dst, u8 len)
{
 //u8 res = 1;
 if(len)
  while(len--)
  {
   if(*dst++ != *src++)
    return 0;
  }
 return 1;
}
//-------------------------------------------------------------------------------------------
u8 crcNavi(const u8 *buf,u8 len)
{
   u8 res=0;
   while(len--)
     res ^= *buf++;
   return res; 
}
//-------------------------------------------------------------------------------------------
u8 getLenNum(u32 num)
{
	u8 i = 10;
	if(num < 10)      i = 1;
	else if(num < 100) i = 2;
	else if(num < 1000) i = 3;
	else if(num < 10000) i = 4;
	else if(num < 100000) i = 5;
	else if(num < 1000000) i = 6;
	else if(num < 10000000) i = 7;
	else if(num < 100000000) i = 8;
	else if(num < 1000000000) i = 9;
	//else if(num < 10000000000) i = 10;
	return i;
}
//-------------------------------------------------------------------------------------------
void monitorWatchDog(void)
{
  if(flagIWDGReload)
    {
	  flagIWDGReload = 0;
	  /* Reload IWDG counter */
	  IWDG_ReloadCounter();
	  setTimerIWDG(ONE_SEC);
    }
}
//-------------------------------------------------------------------------------------------
void handleTimerIWDG(void)
{
    if(timerValueIWDG)
	  {
	   timerValueIWDG--;
	   if(!timerValueIWDG )
	     {
		   flagIWDGReload = 1;
		 }
	  }
}
// ------------------------------------------------------------------------------
void setTimerIWDG(u32 val)
{
   	timerValueIWDG = val;
}
// ------------------------------------------------------------------------------
/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}
// ------------------------------------------------------------------------------

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}
//// ------------------------------------------------------------------------------
//
//int  SendChar(int ch)
//{
//	  if (DEMCR & TRCENA) {
//		while (ITM_Port32(0) == 0);
//		ITM_Port8(0) = ch;
//	  }
//	  return(ch);
//}
// ------------------------------------------------------------------------------

//int  SendChar(int ch)
//{
//	  return ITM_SendChar(ch);
//}



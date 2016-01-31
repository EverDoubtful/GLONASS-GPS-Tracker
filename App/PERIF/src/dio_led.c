/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "dio_led.h"
#include "common.h"
#include "protocol_bin.h"
#include <nmea/nmea.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
//LEDS
#define LED_MID_PORT                          GPIOD  
#if defined (VER_3)
	#define LED_MID_PORT_PIN                      GPIO_Pin_6
#else
	#define LED_MID_PORT_PIN                      GPIO_Pin_14
#endif
#define RCC_APB2Periph_GPIO_LED_MID           RCC_APB2Periph_GPIOD

#define LED_DN_PORT                           GPIOD  
#if defined (VER_3)
	#define LED_DN_PORT_PIN                       GPIO_Pin_7
#else
	#define LED_DN_PORT_PIN                       GPIO_Pin_15
#endif
#define RCC_APB2Periph_GPIO_LED_DN            RCC_APB2Periph_GPIOD
#if defined (VER_3)
	#define LED_UP_PORT                           GPIOD  
	#define LED_UP_PORT_PIN                       GPIO_Pin_5
	#define RCC_APB2Periph_GPIO_LED_UP            RCC_APB2Periph_GPIOD

	#define JTAG_TMS_PORT                         GPIOA  
	#define JTAG_TMS_PORT_PIN                     GPIO_Pin_13
	#define RCC_APB2Periph_GPIO_JTAG_TMS          RCC_APB2Periph_GPIOA
	#define JTAG_TCK_PORT                         GPIOA  
	#define JTAG_TCK_PORT_PIN                     GPIO_Pin_14
	#define RCC_APB2Periph_GPIO_JTAG_TCK          RCC_APB2Periph_GPIOA
	#define JTAG_TDI_PORT                         GPIOA  
	#define JTAG_TDI_PORT_PIN                     GPIO_Pin_15
	#define RCC_APB2Periph_GPIO_JTAG_TDI          RCC_APB2Periph_GPIOA
	#define JTAG_TDO_PORT                         GPIOB
	#define JTAG_TDO_PORT_PIN                     GPIO_Pin_3
	#define RCC_APB2Periph_GPIO_JTAG_TDO          RCC_APB2Periph_GPIOB
	#define JTAG_NTRST_PORT                       GPIOB  
	#define JTAG_NTRST_PORT_PIN                   GPIO_Pin_4
	#define RCC_APB2Periph_GPIO_JTAG_NTRST        RCC_APB2Periph_GPIOB

#endif

//TTL DOUT
#if defined (VER_3)
	#define TTL_DOUT1_PORT                        GPIOE  
	#define TTL_DOUT1_PORT_PIN                    GPIO_Pin_9
	#define RCC_APB2Periph_GPIO_TTL_DOUT1         RCC_APB2Periph_GPIOE
	#define TTL_DOUT2_PORT                        GPIOE  
	#define TTL_DOUT2_PORT_PIN                    GPIO_Pin_11
	#define RCC_APB2Periph_GPIO_TTL_DOUT2         RCC_APB2Periph_GPIOE
#else
	#define TTL_DOUT1_PORT                        GPIOB  
	#define TTL_DOUT1_PORT_PIN                    GPIO_Pin_0
	#define RCC_APB2Periph_GPIO_TTL_DOUT1         RCC_APB2Periph_GPIOB
	#define TTL_DOUT2_PORT                        GPIOB  
	#define TTL_DOUT2_PORT_PIN                    GPIO_Pin_1
	#define RCC_APB2Periph_GPIO_TTL_DOUT2         RCC_APB2Periph_GPIOB
#endif

//DIN
#define TTL_DIN1_PORT                         GPIOE  
#if defined (VER_3)
	#define TTL_DIN1_PORT_PIN                     GPIO_Pin_13
#else
	#define TTL_DIN1_PORT_PIN                     GPIO_Pin_0
#endif
#define RCC_APB2Periph_GPIO_TTL_DIN1          RCC_APB2Periph_GPIOE

#define TTL_DIN2_PORT                         GPIOE  
#if defined (VER_3)
    #define TTL_DIN2_PORT_PIN                     GPIO_Pin_14
#else
    #define TTL_DIN2_PORT_PIN                     GPIO_Pin_1
#endif
#define RCC_APB2Periph_GPIO_TTL_DIN2          RCC_APB2Periph_GPIOE
#if defined (VER_3)
	#define TTL_DIN3_PORT                         GPIOB  
	#define TTL_DIN3_PORT_PIN                     GPIO_Pin_10
	#define RCC_APB2Periph_GPIO_TTL_DIN3          RCC_APB2Periph_GPIOB
	#define TTL_DIN4_PORT                         GPIOB  
	#define TTL_DIN4_PORT_PIN                     GPIO_Pin_11
	#define RCC_APB2Periph_GPIO_TTL_DIN4          RCC_APB2Periph_GPIOB
#else
	//BUTTONS
	#define BUT_TAMPER_PORT                         GPIOC  
	#define BUT_TAMPER_PORT_PIN                     GPIO_Pin_13
	#define RCC_APB2Periph_GPIO_BUT_TAMPER          RCC_APB2Periph_GPIOC
#endif


#define LED_UP_ON      (LED_UP_PORT->BSRR  = LED_UP_PORT_PIN)
#define LED_UP_OFF     (LED_UP_PORT->BRR   = LED_UP_PORT_PIN)
#define LED_MID_ON     (LED_MID_PORT->BSRR = LED_MID_PORT_PIN)
#define LED_MID_OFF    (LED_MID_PORT->BRR  = LED_MID_PORT_PIN)
#define LED_DN_ON      (LED_DN_PORT->BSRR  = LED_DN_PORT_PIN)
#define LED_DN_OFF     (LED_DN_PORT->BRR   = LED_DN_PORT_PIN)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern t_innerState innerState;
extern bin_info  binInfo;
extern nmeaINFO info;

u32 timerValueLeds;
u32 timerValueDIN;
u32 timerValuePwr;
u32 timerValueLedNavi;
u8  flagLedsUp;
u8  flagLedsMid;
u8  flagLedsMidGPS;
u8 	flagLedsMidSignNum;
u8  flagLedsDn;

u8  dutyCycle;	//skvashznost'
u8  byteDIN;
volatile  u8 inputDIN[3];      

/* Private function prototypes -----------------------------------------------*/
void setTimerLeds(u32 val);
void setTimerLedsDn(u32 val);
void setTimerLedsMid(u32 val);

void setTimerDIN(u32 val);
u8   readDIN(void);
/* Private functions ---------------------------------------------------------*/
//-------------------------------------------------------------------------------------------

void led_mid( led_action act)
{
   if(act == BOARD_LED_ON)
      LED_MID_PORT->BSRR = LED_MID_PORT_PIN;
   else	if(act == BOARD_LED_OFF)
      LED_MID_PORT->BRR = LED_MID_PORT_PIN;
   else 
      LED_MID_PORT->ODR ^= LED_MID_PORT_PIN;
}
//-------------------------------------------------------------------------------------------
void led_dn( led_action act)
{
   if(act == BOARD_LED_ON)
      LED_DN_PORT->BSRR = LED_DN_PORT_PIN;
   else	if(act == BOARD_LED_OFF)
      LED_DN_PORT->BRR = LED_DN_PORT_PIN;
   else 
      LED_DN_PORT->ODR ^= LED_DN_PORT_PIN;
}
#if defined (VER_3)
//-------------------------------------------------------------------------------------------
void led_up( led_action act)
{  
   if(act == BOARD_LED_ON)
      LED_UP_PORT->BSRR = LED_UP_PORT_PIN;
   else	if(act == BOARD_LED_OFF)
      LED_UP_PORT->BRR = LED_UP_PORT_PIN;
   else 
      LED_UP_PORT->ODR ^= LED_UP_PORT_PIN;
}
#endif
//-------------------------------------------------------------------------------------------
/*  maskNumTU: mask control which TU should control,bit0 - TU1, bit1- TU2;0-no control,1 - yes
    maskAction: mask control what to do with TU - On(1) or Off(0)        */
//-------------------------------------------------------------------------------------------
void ttl_dout( u8 maskNumTU, u8 maskAction)
{
   if(maskNumTU & 0x01)
     {
	   if(maskAction & 0x01)
	      GPIO_SetBits(TTL_DOUT1_PORT, TTL_DOUT1_PORT_PIN);
	   else
	      GPIO_ResetBits(TTL_DOUT1_PORT, TTL_DOUT1_PORT_PIN);
	  }
   
   if(maskNumTU & 0x02)
     {
	   if(maskAction & 0x02)
	      GPIO_SetBits(TTL_DOUT2_PORT, TTL_DOUT2_PORT_PIN);
	   else
	      GPIO_ResetBits(TTL_DOUT2_PORT, TTL_DOUT2_PORT_PIN);
     }
}
/*******************************************************************************/
/*     			   */
///* If we want change buffer by info we should remove pointer 5 bytes right(pil+cmd+aux+len)*/
/*******************************************************************************/
u8 controlTU(u8 * buf)
{
  ttl_dout(*buf++, *buf);
  buf--;
  *buf = readStateOutput();
  return 1; 
}
//-------------------------------------------------------------------------------------------
u8 readStateInput(void)
{
  u16 cpsr, temp;
  cpsr = __disable_irq();
  temp = byteDIN;
  if(!cpsr)
     	__enable_irq();
  return temp;
}

//-------------------------------------------------------------------------------------------
u8 readDIN(void)
{
  u8 res = 0;
  res = GPIO_ReadInputDataBit(TTL_DIN1_PORT, TTL_DIN1_PORT_PIN);
  res |= (GPIO_ReadInputDataBit(TTL_DIN2_PORT, TTL_DIN2_PORT_PIN) << 1);
#if defined (VER_3)
  res |= (GPIO_ReadInputDataBit(TTL_DIN3_PORT, TTL_DIN3_PORT_PIN) << 2);
  res |= (GPIO_ReadInputDataBit(TTL_DIN4_PORT, TTL_DIN4_PORT_PIN) << 3);
#endif
  return res;
}
//-------------------------------------------------------------------------------------------
u8 readStateOutput(void)
{
  u8 res = 0;
  	//if((data == 0xFF) && (!(GPIO_ReadInputData(GPIOA) & 0x10)) )
  res = GPIO_ReadOutputDataBit(TTL_DOUT1_PORT, TTL_DOUT1_PORT_PIN);
  res |= (GPIO_ReadOutputDataBit(TTL_DOUT2_PORT, TTL_DOUT2_PORT_PIN) << 1);

  return res;
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


#if defined (VER_3)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_LED_UP, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_TTL_DIN3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_TTL_DIN4, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_JTAG_TMS, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_JTAG_TCK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_JTAG_TDI, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_JTAG_TDO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_JTAG_NTRST, ENABLE);
#else
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_BUT_TAMPER, ENABLE);
#endif

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
#if defined (VER_3)
	/* Configure IO connected to LED UP *********************/	
	GPIO_InitStructure.GPIO_Pin =   LED_UP_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(LED_UP_PORT, &GPIO_InitStructure);
	/* JTAG */
    GPIO_InitStructure.GPIO_Pin   = JTAG_TMS_PORT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(JTAG_TMS_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = JTAG_TCK_PORT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(JTAG_TCK_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = JTAG_TDI_PORT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(JTAG_TDI_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = JTAG_TDO_PORT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(JTAG_TDO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = JTAG_NTRST_PORT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_Init(JTAG_NTRST_PORT, &GPIO_InitStructure);
#endif
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
#if defined (VER_3)
	/* Configure IO connected to DIN3 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_DIN3_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(TTL_DIN3_PORT, &GPIO_InitStructure);
	/* Configure IO connected to DIN4 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_DIN4_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(TTL_DIN4_PORT, &GPIO_InitStructure);
#else
//BUTTONS
	/* Configure IO connected to TAMPER *********************/	
	GPIO_InitStructure.GPIO_Pin = BUT_TAMPER_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(BUT_TAMPER_PORT, &GPIO_InitStructure);
#endif

	setTimerDIN(ONE_SEC * 3);
	setTimerLedsMid(ONE_SEC * 5);
	setTimerLedsDn(ONE_SEC * 3);
	flagLedsDn = 1;
	flagLedsMid = 1;
	flagLedsMidGPS  = 1;
	flagLedsMidSignNum  = 1;

}
#if defined (VER_3)
//-------------------------------------------------------------------------------------------
void setDutyCycle(u8 duty) //duty of MS_100
{
    dutyCycle = duty;
	flagLedsUp = 1;
	setTimerLeds(MS_1);
}
#endif
//-------------------------------------------------------------------------------------------
void handleTimerLeds(void)
{
    static u8 cntDIN = 0;
	u32 temp;
	u8 nGPS;
	u8 nGLN;
	static u8 nTemp = 0;
	u8 flagLightOff;

#if defined (VER_3)
   /* Working with leds */
	/* Led UP - gsm status*/
    if(timerValueLeds)
	  {
	   timerValueLeds--;
	   if(!timerValueLeds )
	     {
		   if(flagLedsUp)
		      {
		   		//led_up(BOARD_LED_ON);
				//LED_UP_PORT->BSRR = LED_UP_PORT_PIN;
				LED_UP_ON;
				flagLedsUp = 0;
				//setTimerLeds(MS_10 * 6 + MS_1 * 4);		//64ms
				timerValueLeds = (MS_10 * 6 + MS_1 * 4);
			  }
		   else
		      {
		   		//led_up(BOARD_LED_OFF);
				//LED_UP_PORT->BRR = LED_UP_PORT_PIN;
				LED_UP_OFF;
				flagLedsUp = 1;
				//setTimerLeds(MS_100 * dutyCycle);
				timerValueLeds = (MS_100 * dutyCycle);
			  }
		 }
	  }
#endif
	/* Led MID - Navi satellites number status*/
    if(timerValueLedNavi)
	  {
	   timerValueLedNavi--;
	   if(!timerValueLedNavi )
	     {
		  if(info.sig != NMEA_SIG_BAD)  /* we got NAVI */
		     {
			   //Light
			   if(flagLedsMid)
			      {
			   		flagLightOff = 0;
					flagLedsMid = 0;
					if(flagLedsMidGPS)  //GPS
					  {
					    nGPS = info.satinfo.inuseGPS;
						if(nGPS)
						  {
						    if(flagLedsMidSignNum)   //Sign	(short)
							  {
								//setTimerLedsMid(MS_10 * 6 + MS_1 * 4);		//64ms
								timerValueLedNavi = (MS_10 * 6 + MS_1 * 4);
								flagLedsMidSignNum = 0;
							  }
							else					 //Number
							  {
								if(++nTemp >= nGPS)
								  {
								    nTemp = 0;
									flagLedsMidSignNum = 1;
									flagLedsMidGPS = 0;	  //go to GLN
									//setTimerLedsMid(ONE_SEC);		//1 sec
									timerValueLedNavi = ONE_SEC;
								  }
								else
								  {
									//setTimerLedsMid(ONE_SEC);		//1 sec
									timerValueLedNavi = ONE_SEC;
								  }
							  }
						  }
						else  // no GPS sat-s
						  {
								    nTemp = 0;
									flagLedsMidSignNum = 1;
									flagLedsMidGPS = 0;	  //go to GLN
									//setTimerLedsMid(ONE_SEC);		//just to go further without delay
									timerValueLedNavi = ONE_SEC;
									flagLightOff = 1;
						  }
					  }
					else  	            //GLN
					  {
					    nGLN = info.satinfo.inuseGLN;
						if(nGLN)
						  {
						    if(flagLedsMidSignNum)   //Sign	(Long)
							  {
								//setTimerLedsMid(ONE_SEC * 3);		//3 sec
								timerValueLedNavi = (ONE_SEC * 3);
								flagLedsMidSignNum = 0;
							  }
							else					 //Number
							  {
								if(++nTemp >= nGLN)
								  {
								    nTemp = 0;
									flagLedsMidSignNum = 1;
									flagLedsMidGPS = 1;	  //go to GPS
									//setTimerLedsMid(ONE_SEC);		//1 sec
									timerValueLedNavi = ONE_SEC;
								  }
								else
								  {
									//setTimerLedsMid(ONE_SEC);		//1 sec
									timerValueLedNavi = ONE_SEC;
								  }
							  }
						   }
						else  // no GLN sat-s
						   {
								    nTemp = 0;
									flagLedsMidSignNum = 1;
									flagLedsMidGPS = 1;	  //go to GPS
									//setTimerLedsMid(ONE_SEC);		//just to go further without delay
									timerValueLedNavi = ONE_SEC;
									flagLightOff = 1;
						   }

					  }
					  if(!flagLightOff)
					    {
					     //led_mid(BOARD_LED_ON);
						 //LED_MID_PORT->BSRR = LED_MID_PORT_PIN;
						 LED_MID_ON;
						}
				  }	//if(flagLedsMid)
			   else	  //Off
			      {
			   		//led_mid(BOARD_LED_OFF);
					//LED_MID_PORT->BRR = LED_MID_PORT_PIN;
					LED_MID_OFF;
					flagLedsMid = 1;
					//setTimerLedsMid(ONE_SEC);		//1 sec
					timerValueLedNavi = ONE_SEC;
				  }
		    }	//if(info.sig != NMEA_SIG_BAD)  /* we got NAVI */
		   else
			 {
			   	//led_mid(BOARD_LED_OFF);
				//LED_MID_PORT->BRR = LED_MID_PORT_PIN;
				LED_MID_OFF;
				//setTimerLedsMid(ONE_SEC);		//1 sec
				timerValueLedNavi = ONE_SEC;
			 }
		 }
	  }
	/* Led DOWN - power status*/
    if(timerValuePwr)
	  {
	   timerValuePwr--;
	   if(!timerValuePwr )
	     {
		   if(flagLedsDn)
		      {
		   		//led_dn(BOARD_LED_ON);  //debugga
				//LED_DN_PORT->BSRR = LED_DN_PORT_PIN;
				LED_DN_ON;
				flagLedsDn = 0;
				//setTimerLedsDn(MS_10 * 6 + MS_1 * 4);		//64ms
				timerValuePwr = (MS_10 * 6 + MS_1 * 4);
			  }
		   else
		      {
		   		//led_dn(BOARD_LED_OFF);
				//LED_DN_PORT->BRR = LED_DN_PORT_PIN;
				LED_DN_OFF;
				flagLedsDn = 1;
				temp = ONE_SEC * 5;
				if(innerState.PowerMainBatt) //Batt
				  temp = ONE_SEC * 10;
				//setTimerLedsDn(temp);
				timerValuePwr = temp;
			  }
		 }
	  }
	/* Working with DIN*/
	if(timerValueDIN)
	  {
		timerValueDIN--;
		if(!timerValueDIN)
		  {
		    inputDIN[cntDIN++] = readDIN();
			cntDIN %= 3;
	        byteDIN  = (~(inputDIN[0] ^ inputDIN[1]) & inputDIN[0]);
	        byteDIN |= (~(inputDIN[0] ^ inputDIN[2]) & inputDIN[0]);
	        byteDIN |= (~(inputDIN[1] ^ inputDIN[2]) & inputDIN[1]);
			//setTimerDIN(MS_10 * 2); 
			timerValueDIN = (MS_10 * 2);
		  }
	  }
}
#if defined (VER_3)
// ------------------------------------------------------------------------------
void setTimerLeds(u32 val)
{
   	timerValueLeds = val;
}
#endif
// ------------------------------------------------------------------------------
void setTimerLedsDn(u32 val)
{
   	timerValuePwr = val;
}
// ------------------------------------------------------------------------------
void setTimerLedsMid(u32 val)
{
   	timerValueLedNavi = val;
}
// ------------------------------------------------------------------------------
void setTimerDIN(u32 val)
{
   	timerValueDIN = val;
}


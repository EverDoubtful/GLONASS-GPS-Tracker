/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "adc.h"
#include "common.h"
#include "dio_led.h"	  //debugga
#include "flash.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#if defined (VER_3)
	#define ADC_CHANNELS            (8+2)	//(+2 - Utemp, Urefint)
    float  koefVPower;
	u32    koefADCPwrDivider; //(9.66)  koef delitelya (we write it multiplied by 100 : 10.0 will be 1000)
#else
	#define ADC_CHANNELS            (4+2)	 //(+2 - Utemp, Urefint)
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//#define ADC_DEB_EL    256
//volatile u16 adcDebVal[ADC_DEB_EL];
//volatile u8 flagDebScan;
//u16 index = 0;
#define ADC_AVER_VAL          8
//volatile u16 adcChannels[ADC_CHANNELS];
volatile u16 adcAverChan[ADC_CHANNELS][ADC_AVER_VAL]; 	   // 0 - 7 - scanned 
volatile u8  ADCNumbersChannels[ADC_CHANNELS]; 
volatile u8  ADCIndexScan[ADC_CHANNELS]; 
volatile u8  adcChanIndex;	    //DATA

#define CR2_EXTTRIG_SWSTART_Set     ((uint32_t)0x00500000)

//u8 adcChanNumberIndex;	// ORDER NUMBER OF CHANNELS
//volatile u8 flagADCWait;
u32 timerValueADC;
u32 timerValueBatt;
u32 timerValueRareCh;

u8  flagTimeQueryBatt;
u8  flagTimeQueryRareCh;					  
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//-------------------------------------------------------------------------------------------
void adcGPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef   ADC_InitStructure;
    //AINS
	/* Configure IO connected to AIN1 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_AIN1_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(TTL_AIN1_PORT, &GPIO_InitStructure);
	/* Configure IO connected to AIN2 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_AIN2_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(TTL_AIN2_PORT, &GPIO_InitStructure);
#if defined (VER_3)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_POWER_VBAT_CHECK, ENABLE);
	/* Configure IO connected to POWER_VBAT_CHECK *********************/	
	GPIO_InitStructure.GPIO_Pin =   POWER_VBAT_CHECK_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(POWER_VBAT_CHECK_PORT, &GPIO_InitStructure);

	/* Configure IO connected to AIN3 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_AIN3_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(TTL_AIN3_PORT, &GPIO_InitStructure);
	/* Configure IO connected to AIN4 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_AIN4_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(TTL_AIN4_PORT, &GPIO_InitStructure);
	/* Configure IO connected to AIN5 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_AIN5_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(TTL_AIN5_PORT, &GPIO_InitStructure);
	/* Configure IO connected to AIN6 *********************/	
	GPIO_InitStructure.GPIO_Pin = TTL_AIN6_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(TTL_AIN6_PORT, &GPIO_InitStructure);
#endif
/* Configure IO connected to VIN_SENS *********************/	
	GPIO_InitStructure.GPIO_Pin = VIN_SENS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(VIN_SENS_PORT, &GPIO_InitStructure);
	/* Configure IO connected to VBAT_SENS *********************/	
	GPIO_InitStructure.GPIO_Pin = VBAT_SENS_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  	GPIO_Init(VBAT_SENS_PORT, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE );

	ADC_TempSensorVrefintCmd(ENABLE);

	/* ADCCLK = PCLK2/4 */
    RCC_ADCCLKConfig(RCC_PCLK2_Div8); 
	/* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //ADC_Mode_RegSimult;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;/*ADC_CHANNELS*/;
  ADC_Init(ADC1, &ADC_InitStructure);
    /* Enable ADC1 EOC interupt */
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

  /*ANALOG WATCHDOG*/
  /* Configure high and low analog watchdog thresholds */
  //ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x0B00, 0x0300);
  /* Configure channel14 as the single analog watchdog guarded channel */
  //ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_10);
  /* Enable analog watchdog on one regular channel */
  //ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);
  /* Enable AWD interupt */
  //ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibaration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
#if defined (VER_3)
  ADCNumbersChannels[0] = ADC_Channel_8; //vin
  ADCNumbersChannels[1] = ADC_Channel_9; //vbat
  ADCNumbersChannels[2] = ADC_Channel_10;
  ADCNumbersChannels[3] = ADC_Channel_11;
  ADCNumbersChannels[4] = ADC_Channel_12;
  ADCNumbersChannels[5] = ADC_Channel_13;
  ADCNumbersChannels[6] = ADC_Channel_14;
  ADCNumbersChannels[7] = ADC_Channel_15;
  ADCNumbersChannels[8] = ADC_Channel_16; //temp sensor
  ADCNumbersChannels[9] = ADC_Channel_17; //ref int
#else
  ADCNumbersChannels[0] = ADC_Channel_10; //vin
  ADCNumbersChannels[1] = ADC_Channel_11; //vbat
  ADCNumbersChannels[2] = ADC_Channel_14;
  ADCNumbersChannels[3] = ADC_Channel_15;
  ADCNumbersChannels[4] = ADC_Channel_16;
  ADCNumbersChannels[5] = ADC_Channel_17;

  //  ADCNumbersChannels[0] = ADC_Channel_14; //vin real
#endif
  adcChanIndex = 0;
  //adcChanNumberIndex = 0;
  //flagADCWait = 0;
}
//-------------------------------------------------------------------------------------------
#if defined (VER_3)
void pwr_vbat_check(batt_check_action action)
{
    if(action)
		GPIO_SetBits(POWER_VBAT_CHECK_PORT, POWER_VBAT_CHECK_PORT_PIN);
	else
		GPIO_ResetBits(POWER_VBAT_CHECK_PORT, POWER_VBAT_CHECK_PORT_PIN);
}
#endif
//-------------------------------------------------------------------------------------------
void adcInterruptsConfig(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  /* Configure and enable ADC interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}
//-------------------------------------------------------------------------------------------
void adcInit(void)
{
  adcGPIOInit();
  adcInterruptsConfig();
  setTimerADC(ONE_SEC * 5);
  flagTimeQueryBatt = 1;
  flagTimeQueryRareCh = 1;
  /* Read calibr koef */
#if defined (VER_3)
  read_page(ADDRESS_PAGE_CALIBR_KOEF, (u8 *)&koefADCPwrDivider , sizeof(koefADCPwrDivider));
  if(koefADCPwrDivider  == 0xFFFFFFFF) 
     koefADCPwrDivider = 966;
  koefVPower = ((float)koefADCPwrDivider / 100 )* koefADC;
#endif

}
//-------------------------------------------------------------------------------------------
void adcProcessHandler(void)
{
    static u8 flagBattOn = 0;
	//static u8 flagFirstScan = 1;
	static u8 cntRareCh = 0;
	u8 index;
//  if(ADC_GetITStatus(ADC1, ADC_IT_AWD) != RESET)
//    {
//      /* Clear ADC1 AWD pending interrupt bit */
//      ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
//	  led_dn(BOARD_LED_XOR);
//    }
//  else
//    {
		/* Get result*/
		//adcAverChan[adcChanIndex][0] = ADC1->DR; //ADC_GetConversionValue(ADC1);
		if(!flagBattOn)     /* Usual channel  */
		  {
			ADCIndexScan[adcChanIndex]++;
			ADCIndexScan[adcChanIndex] %= ADC_AVER_VAL;
			index = ADCIndexScan[adcChanIndex] ;
		  }
		else				/* Battery channel  */
		    index = 0;

		adcAverChan[adcChanIndex][index] = ADC1->DR;

		//if(!flagBattOn)  //not battary
		//  adcAverChan[adcChanIndex][1] = adcAverChan[adcChanIndex][1] + ((adcAverChan[adcChanIndex][0] - adcAverChan[adcChanIndex][1]) >> 4);
		//else
		//  adcAverChan[adcChanIndex][1] = adcAverChan[adcChanIndex][0];
				   		
	    if(adcChanIndex == 1)  //Batt channel
			   {
		         //pwr_vbat_check(BATT_CHECK_OFF);
				 //GPIO_ResetBits(POWER_VBAT_CHECK_PORT, POWER_VBAT_CHECK_PORT_PIN);
				 if(flagTimeQueryBatt)
				   {
				     flagTimeQueryBatt = 0;
					 #if defined (VER_3)
					 //POWER_VBAT_CHECK_PORT->BRR = POWER_VBAT_CHECK_PORT_PIN;   //debug
					 #endif
					 flagBattOn = 0;
				   }
			   }
		adcChanIndex++;
		if(adcChanIndex == ADC_CHANNELS)
		   {
			 adcChanIndex = 0;
			 //setTimerADC(MS_100);
			 timerValueADC = (MS_100 +  MS_10 * 5);
			 //flagFirstScan = 0;
		   }
		else
		   {
		     if(adcChanIndex == 1)  //Batt channel
			    {
		          //pwr_vbat_check(BATT_CHECK_ON);
				  //GPIO_SetBits(POWER_VBAT_CHECK_PORT, POWER_VBAT_CHECK_PORT_PIN);
				  if(flagTimeQueryBatt)
				    {
					  #if defined (VER_3)
					  pwr_vbat_check(BATT_CHECK_OFF);  //debug
					  //POWER_VBAT_CHECK_PORT->BSRR = POWER_VBAT_CHECK_PORT_PIN;
					  #endif
					  flagBattOn = 1;
					  timerValueBatt = (ONE_SEC * 60);
					  //led_dn(BOARD_LED_XOR);  //debugga
				    }
				  else
					adcChanIndex++;
				}
				/*
			  else if(adcChanIndex == 0)   //power
			    {
				  if(flagTimeQueryRareCh)
				    {
					  cntRareCh++;
					}
				  else
				    adcChanIndex = 2;
			    } */
			  else if(adcChanIndex == 8)   //Temp
			    {
				  if(flagTimeQueryRareCh)
				    {
					  cntRareCh++;
					}
				  else
				    adcChanIndex = 0;
			    }
			  else if(adcChanIndex == 9)   //Vref_int
			    {
				  if(flagTimeQueryRareCh)
				    {
					  cntRareCh++;
					}
				  else
				    adcChanIndex = 0;
			    }
		      if(cntRareCh == 2)
			     {
				   cntRareCh = 0;
				   flagTimeQueryRareCh = 0;
				   timerValueRareCh = ONE_SEC;
				 }
		
			 //setTimerADC(MS_1*5);
			 timerValueADC = MS_10;
		   }
//    }
}
//-------------------------------------------------------------------------------------------
void handleTimerADC(void)
{
    if(timerValueADC)
	  {
	   timerValueADC--;
	   if(!timerValueADC )
	     {
			//ADC_RegularChannelConfig(ADC1, ADCNumbersChannels[adcChanIndex], 1, ADC_SampleTime_239Cycles5);
			if(ADCNumbersChannels[adcChanIndex] > ADC_Channel_9)
			   ADC1->SMPR1 = 0x00FFFFFF;
			else
			   ADC1->SMPR2 = 0x3F000000;
			ADC1->SQR3 = ADCNumbersChannels[adcChanIndex];
			//ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			/* Enable the selected ADC conversion on external event and start the selected ADC conversion */
    		ADC1->CR2 |= CR2_EXTTRIG_SWSTART_Set;
		 }
	  }
    if(timerValueBatt)
	  {
	     timerValueBatt--;
		 if(!timerValueBatt)
		    {
			  flagTimeQueryBatt = 1; 
			}
	  }
    if(timerValueRareCh)
	  {
	     timerValueRareCh--;
		 if(!timerValueRareCh)
		    {
			  flagTimeQueryRareCh = 1; 
			}
	  }

}
//-------------------------------------------------------------------------------------------
u16 ain_read(u8 channel)
{
  u16 cpsr, temp = 0;
  u8 i = 0, k = 0;
  cpsr = __disable_irq();
  //temp = adcAverChan[channel % ADC_CHANNELS][0];

  k = (channel % ADC_CHANNELS);
  if(k != 1)  /* Usual channel  */
    {
	  while(i < ADC_AVER_VAL)
	   {
	     temp += adcAverChan[k][i];
		 i++;
	   }
	  temp = (temp >> 3);
	  //adcAverChan[k][0] = temp;
    }
  else   /*Battery channel*/
    temp = adcAverChan[1][0];

  if(!cpsr)
     	__enable_irq();
  return temp;

}
// ------------------------------------------------------------------------------
void setTimerADC(u32 val)
{
   	timerValueADC = val;
}

//-------------------------------------------------------------------------------------------
void ainDebRead(u8 *buf, u16 len)
{
  /*
  u16 i = index;
  flagDebScan = 1;
  while(len--)
    {
     *(u16 *)buf = adcDebVal[i++];
	 i %= ADC_DEB_EL;
	 buf += 2;
	}
  flagDebScan = 0;
  */
}
/*******************************************************************************/
/* first prm - N channel(extending for the future - other channels),
 next 2 bytes - Voltage multiplied by 100				*/
/*******************************************************************************/
u16 setCalibrKoef(u8 * buf)
{
   u16 len = 0;
   #if defined (VER_3)
   buf++;  //missing N channel
   koefADCPwrDivider = ((*(u16 *)buf) / (100.0 * koefADC * ain_read(0) )) * 100 ;
   koefVPower = ((float )koefADCPwrDivider / 100) * koefADC;
   /* Write calibr koef */
   write_firm_page(ADDRESS_PAGE_CALIBR_KOEF, (u8 *)&koefADCPwrDivider);
   #endif

   return len;
}
//void adcCalcKoef(void)
//{
     //koefVPower = koefADCPwrDivider * koefStdADC;
//}

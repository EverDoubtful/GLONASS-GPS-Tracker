/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "adc.h"
#include "common.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#if defined (VER_3)
	#define ADC_CHANNELS            (8+2)	//(+2 - Utemp, Urefint)
#else
	#define ADC_CHANNELS            (4+2)	 //(+2 - Utemp, Urefint)
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//#define ADC_DEB_EL    256
//volatile u16 adcDebVal[ADC_DEB_EL];
//volatile u8 flagDebScan;
//u16 index = 0;

volatile u16 adcChannels[ADC_CHANNELS];
		 u8  ADCNumbersChannels[ADC_CHANNELS]; 
volatile u8 adcChanIndex;	    //DATA
//u8 adcChanNumberIndex;	// ORDER NUMBER OF CHANNELS
//volatile u8 flagADCWait;
u32 timerValueADC;
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
  ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x0B00, 0x0300);
  /* Configure channel14 as the single analog watchdog guarded channel */
  ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_10);
  /* Enable analog watchdog on one regular channel */
  ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);
  /* Enable AWD interupt */
  ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);

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
}
//-------------------------------------------------------------------------------------------
void adcProcessHandler(void)
{
  //if(!flagDebScan)
  //{
  /* Clear ADC1 AWD pending interrupt bit */
  ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);

  adcChannels[adcChanIndex] = ADC_GetConversionValue(ADC1);
#if defined (VER_3)
	 if(adcChanIndex == 1)  //Batt channel
         pwr_vbat_check(BATT_CHECK_OFF);
#endif

  //adcDebVal[index++] = ADC_GetConversionValue(ADC1);
  //index %= ADC_DEB_EL;
  adcChanIndex++;

  if(adcChanIndex == ADC_CHANNELS)
   {
	 adcChanIndex = 0;
	 setTimerADC(MS_100 * 5);
	 //setTimerADC(MS_10 * 5);
   }
  else
   {
#if defined (VER_3)
	 if(adcChanIndex == 1)  //Batt channel
         pwr_vbat_check(BATT_CHECK_ON);
#endif
	 setTimerADC(MS_10);
   }
   //}
}
//-------------------------------------------------------------------------------------------
u16 ain_read(u8 channel)
{
  u16 cpsr, temp;
  cpsr = __disable_irq();
  temp = adcChannels[channel % ADC_CHANNELS];
  if(!cpsr)
     	__enable_irq();
  return temp;

}

//-------------------------------------------------------------------------------------------
void handleTimerADC(void)
{
    if(timerValueADC)
	  {
	   timerValueADC--;
	   if(!timerValueADC )
	     {
			ADC_RegularChannelConfig(ADC1, ADCNumbersChannels[adcChanIndex], 1, ADC_SampleTime_239Cycles5);
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		 }
	  }
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

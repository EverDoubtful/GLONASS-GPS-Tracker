/* Includes ------------------------------------------------------------------*/
#if defined (VER_3)

#include "stm32f10x.h"
#include "dac.h"

#define DAC_DHR12RD_Address      0x40007420

uc16 Sine12bit[32] = {2047, 2447, 2831, 3185, 3498, 3750, 3939, 4056, 4095, 4056,
                      3939, 3750, 3495, 3185, 2831, 2447, 2047, 1647, 1263, 909, 
                      599, 344, 155, 38, 0, 38, 155, 344, 599, 909, 1263, 1647};

u32 DualSine12bit[32];
u8 idx = 0;
#define ARRAY_LENGHT(Value)         (sizeof(Value) / sizeof(Value[0]))
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void DACInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    DAC_InitTypeDef  DAC_InitStructure;
    //TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	//DMA_InitTypeDef            DMA_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DACOUT, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DACSPK, ENABLE);
    /* DAC Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    /* DMA clock enable */
    //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* TIM2 Periph clock enable */
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	/* DACOUT port to output*/
	GPIO_InitStructure.GPIO_Pin =   DACOUT_PORT_PIN;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; //

  	//GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  	GPIO_Init(DACOUT_PORT, &GPIO_InitStructure);
	/* DACSPK port to output*/
	GPIO_InitStructure.GPIO_Pin =   DACSPK_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  	GPIO_Init(DACSPK_PORT, &GPIO_InitStructure);

    /* DAC channel1 Configuration */
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software;   //saw
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_Triangle;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude =  DAC_TriangleAmplitude_4095; // DAC_TriangleAmplitude_511;
  
  //DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software;		 //noise
  //DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_Noise;
  //DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits11_0;

  //DAC_InitStructure.DAC_Trigger = DAC_Trigger_T2_TRGO;   //saw by timer
  //DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_Triangle;
  //DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_TriangleAmplitude_2047;

    //DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = 0;
    //DAC_InitStructure.DAC_Trigger = DAC_Trigger_T7_TRGO;
    //DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;

    //DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;  //work
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);

    /* Enable DAC Channel1 */
    DAC_Cmd(DAC_Channel_1, ENABLE);

    /* Set DAC Channel1 DHR12L register */
    //DAC_SetChannel1Data(DAC_Align_12b_L, 0x7ff0);   //for pila
    //DAC_SetChannel1Data(DAC_Align_12b_R, 0x200);
    
	//DAC_SetChannel1Data(DAC_Align_12b_L, 0x7FF0);

    /* TIM2 enable counter */
    // TIM_Cmd(TIM2, ENABLE);
    /* Enable DMA for DAC Channel2 */
    //DAC_DMACmd(DAC_Channel_1, ENABLE);

    /* TIM8 enable counter */
    //TIM_Cmd(TIM7, ENABLE);


}

//-------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void DACSpeaker(u8 sw)
{
    if(sw == 1)  //ON
	  {
		GPIO_SetBits(DACSPK_PORT, DACSPK_PORT_PIN);
	  } 
	else   //OFF
	  {
		GPIO_ResetBits(DACSPK_PORT, DACSPK_PORT_PIN);
	  }
}
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "ff.h"
#include "waveplayer.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
//-------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

//#define WP_ENABLE  TIM6->CR1 |=  TIM_CR1_CEN; //????????? ???????????????
//#define WP_DISABLE TIM6->CR1 &= ~TIM_CR1_CEN; //?????????? ???????????????

unsigned char DAC_Buff1[SIZE_DAC_BUFF];        //????? ?????? ????
unsigned char DAC_Buff2[SIZE_DAC_BUFF];        //????? ?????? ????
static __IO uint16_t TIM6ARRValue = 1088;

//********************************************************************************************
//function ????????? ???? ? ????????? ????????? ??????? ?????? format                       //
//argument ????????? ?? ???????? ?????????, ??? ?????(? ????????? ???? ? ????)              //
//result   WP_OK - ???? ?????? ?????????, WP_ERROR_FILE - ?????? ????????                   //
//********************************************************************************************
WPRESULT wp_open( FIL *file,  const char *FileName, wp_format *format )
{
  unsigned char res = 0;                          //??? ???????? ??????????
  unsigned char buff[64];                     //????? ??? ????????? ?????
  u32      cnt;                          //?????????? ??????????? ????
  unsigned long i;                            //?????. ??????????
  FRESULT       resultFAT;

  resultFAT = f_open ( file, FileName, FA_READ );   //??????? ???? ??? ??????
  if(resultFAT)
     return WP_ERROR_FILE;               //?????? ????????
  resultFAT = f_read ( file, buff, 64, &cnt );  //????????? ?????? 64 ????
  if(resultFAT || cnt!=64)
     return WP_ERROR_FILE;           //?????? ??????

  //????? 0..3 - ?????? ???? ?????? "RIFF"
  if(buff[0]!='R' || buff[1]!='I' || buff[2]!='F' || buff[3]!='F')     res = 2;

  //????? 8..11 - ?????? ???? ?????? "WAVE"
  if(buff[8]!='W' || buff[9]!='A' || buff[10]!='V' || buff[11]!='E')   res = 2;

  //????? 12..15 - ?????? ???? ?????? "fmt "
  if(buff[12]!='f' || buff[13]!='m' || buff[14]!='t' || buff[15]!=' ') res = 2;

  //????? 20,21  - 1 ? 0
  if(buff[20]!=1 || buff[21]!=0) res = 2;

  //????????? ?????? ????? "fmt "
  i = buff[16] | (buff[17]<<8) | (buff[18]<<16) | (buff[19]<<24);

  //????? ????? "fmt " ?????? ????????? ???? "data"
  if(buff[20+i]!='d' || buff[21+i]!='a' || buff[22+i]!='t' || buff[23+i]!='a') res = 2;

  if(res == 2)
  {
    f_close(file);                          //??????? ????
    return WP_ERROR_FILE;                   //?????? ??????? ?????
  }

  /*                    ????????? ?????? ??????                        */
  format->Channels       = buff[22];
  format->SamplesPerSec  = buff[24] | (buff[25]<<8) | (buff[26]<<16) | (buff[27]<<24);
  format->AvgBytesPerSec = buff[28] | (buff[29]<<8) | (buff[30]<<16) | (buff[31]<<24);
  format->BitsPerSample  = buff[34] | (buff[35]<<8);
  format->Size           = buff[24+i]|(buff[25+i]<<8)|(buff[26+i]<<16)|(buff[27+i]<<24);

  //???????? ????????? ?? ?????? ??????
  f_lseek(file,i+28);

  return WP_OK;
}
//********************************************************************************************
					
void wp_init (wp_format *format)
{
  DAC_InitTypeDef            DAC_InitStructure;
  DMA_InitTypeDef            DMA_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

  /* TIM6 Configuration */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  TIM_DeInit(TIM6);

  /* DMA1 channel3 configuration */
  DMA_DeInit(DMA2_Channel3);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&DAC->DHR8R1; 
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) & DAC_Buff1;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 512;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA2_Channel3, &DMA_InitStructure);

  /* Enable DMA1 Channel3 */
  DMA_Cmd(DMA2_Channel3, ENABLE);

  /* DAC deinitialize */
  DAC_DeInit();
  DAC_StructInit(&DAC_InitStructure);

  /* Fill DAC InitStructure */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;

/* DAC Channel1: 8bit right---------------------------------------------------*/
  /* DAC Channel1 Init */
  DAC_Init(DAC_Channel_1, &DAC_InitStructure);
  /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
     automatically connected to the DAC converter. */
  DAC_Cmd(DAC_Channel_1, ENABLE);
  /* Enable DMA for DAC Channel1 */
  DAC_DMACmd(DAC_Channel_1, ENABLE);
  
}
//********************************************************************************************
//********************************************************************************************
//function ????????????? ????????? ????                                                     //
//argument ??? ?????(? ????????? ???? ? ????)                                               //
//result   ?????? ???????? WPRESULT ? ???????????? ?????                                    //
//********************************************************************************************
WPRESULT wp_play ( const char *FileName )
{
  FIL           file;                       
  wp_format     format;                     
  u32           cnt;                        
  WPRESULT      res;
  FRESULT       resultFAT;

  res = wp_open(&file, FileName, &format);  
  if(res) 
    return WP_ERROR_FILE;             

  wp_init(&format);                         

  /* TIM6 TRGO selection */
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
  TIM6ARRValue = FREQ_CLK / format.SamplesPerSec;
  TIM_SetAutoreload(TIM6, TIM6ARRValue );
  
  /* Start TIM6 */
  TIM_Cmd(TIM6, ENABLE);


  while(1)
  {
    resultFAT = f_read ( &file, DAC_Buff2, SIZE_DAC_BUFF, &cnt );
    if( resultFAT )
	   { 
	      res = WP_ERROR_READ_FILE;
		  break;
	   }
    if( cnt < SIZE_DAC_BUFF )
	    break;            
    while (DMA_GetFlagStatus(DMA2_FLAG_TC3) == RESET);
    //while(!(DMA2->ISR & DMA_ISR_TCIF3));      
    DMA2->IFCR = DMA2_FLAG_TC3;
    DMA2_Channel3->CCR = 0x0;
    DMA2_Channel3->CNDTR = 0x200;
    DMA2_Channel3->CPAR =  (u32)&DAC->DHR8R1;  //0x40007410;
    DMA2_Channel3->CMAR = (uint32_t) & DAC_Buff2;
    DMA2_Channel3->CCR = 0x2091;

    resultFAT = f_read ( &file, DAC_Buff1, SIZE_DAC_BUFF, &cnt );
    if(resultFAT)
	   {
	     res = WP_ERROR_READ_FILE;
		 break;
	   }
    if(cnt < SIZE_DAC_BUFF)
	    break;            
    
    while (DMA_GetFlagStatus(DMA2_FLAG_TC3) == RESET);
    //while(!(DMA2->ISR & DMA_ISR_TCIF3));      
    DMA2->IFCR = DMA2_FLAG_TC3;
    DMA2_Channel3->CCR = 0x0;
    DMA2_Channel3->CNDTR = 0x200;
    DMA2_Channel3->CPAR = (u32)&DAC->DHR8R1;
    DMA2_Channel3->CMAR = (uint32_t) & DAC_Buff1;
    DMA2_Channel3->CCR = 0x2091;
  }

  DMA2_Channel3->CCR = 0x0;
  /* Disable TIM6 */
  TIM_Cmd(TIM6, DISABLE);

  f_close(&file);                             //??????? ???? ????? ???????
  return res;                                 //?????????? ?????????

}
//********************************************************************************************


/* Includes ------------------------------------------------------------------*/
#if defined (VER_3)
#include "stm32f10x.h"
#include "rfmodem.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
void rfmodemInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_RFMODEM_TXD, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_RFMODEM_RXD, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_RFMODEM_TXD_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_RFMODEM_RXD_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_RFMODEM_PTT, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_RFMODEM_DIR, ENABLE);

	/* Configure IO connected to RXD RFMODEM *********************/	
	GPIO_InitStructure.GPIO_Pin = RFMODEM_RXD_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(RFMODEM_RXD_PORT, &GPIO_InitStructure);
	/* Configure IO connected to RXD_CLK RFMODEM *********************/	
	GPIO_InitStructure.GPIO_Pin = RFMODEM_RXD_CLK_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(RFMODEM_RXD_CLK_PORT, &GPIO_InitStructure);
	/* Configure IO connected to TXD_CLK RFMODEM *********************/	
	GPIO_InitStructure.GPIO_Pin = RFMODEM_RXD_CLK_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(RFMODEM_RXD_CLK_PORT, &GPIO_InitStructure);

	/* Configure IO connected to TXD RFMODEM *********************/	
	GPIO_InitStructure.GPIO_Pin =   RFMODEM_TXD_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(RFMODEM_TXD_PORT, &GPIO_InitStructure);
	/* Configure IO connected to DIR RFMODEM *********************/	
	GPIO_InitStructure.GPIO_Pin =   RFMODEM_DIR_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(RFMODEM_DIR_PORT, &GPIO_InitStructure);
	/* Configure IO connected to PTT RFMODEM *********************/	
	GPIO_InitStructure.GPIO_Pin =   RFMODEM_PTT_PORT_PIN;
  	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_Init(RFMODEM_PTT_PORT, &GPIO_InitStructure);
  
}

//-------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
#endif

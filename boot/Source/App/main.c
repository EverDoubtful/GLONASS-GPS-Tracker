#include "stm32f10x.h"
#include "ftp.h"
#include "fifo.h"
#include "gprs_modem.h"
#include "platform.h"
#include "button.h"
#include "stdio.h"
#include "dio_led.h"

					 
void TimingDelay_Decrement(void);
static __IO uint32_t TimingDelay;

/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nTime);

//u8  bufCom[16];	   

fifo_buffer bufFifo;
u8 gsmStart;

/* Private define ------------------------------------------------------------*/
#define START_ADDRESS_FW_DEST    ((u32)0x08008000)
#define START_ADDRESS_FW_SRC     ((u32)0x08040000)  //second half 	of whole size of flash(256KB for F103VE)

extern u16 byteCntPGSize;
       u8 flagLastPage;

#define ADDRESS_LAST_PAGE  0x807F800  // f103VE
//#define ADDRESS_LAST_PAGE  0x80FF800  // f103VG
/*******************************************************************************/
/* 	MAP of last page:
    0x10000 - bit32: sign for main app - there was updating, set to 0 by bootloader, read by main app
	0xNNFF  - bits16-31 - number of pages to write, set by main app, read by bootloader
	0xFFSS  - bits0-15  - sign that should update software , set by main app , read by bootloader
	  																		    */
/*******************************************************************************/


u32 EraseCounter = 0x00, Address = 0x00;
u32 Data;
vu32 NbrOfPage = 0x00;
volatile FLASH_Status FLASHStatus;

void UpdatingFlash(void);
void write_firm_pageFTP(u8 sign);
u8   write_firm_page(u32 adr, u8 *buf);
void read_page(u32 adr, u8 *buf);
u8 updateFlag;

//volatile TestStatus MemoryProgramStatus;
ErrorStatus HSEStartUpStatus;

static void s_jump_to(uint32_t appaddr);

void GPIO_XorBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
////////////////////////temp sesctor  //////////////////////




/////////////////////  end of temp sector////////////////////

u8  firmwareBuf[PAGE_SIZE];
u8  cntPageSaved;
u32 * bl;
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void NVIC_Configuration(void)
{ 
  	/* Configure the NVIC Preemption Priority Bits */  
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	#ifdef  VECT_TAB_RAM  
	  /* Set the Vector Table base location at 0x20000000 */ 
	  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#else  /* VECT_TAB_FLASH  */
	  /* Set the Vector Table base location at 0x08000000 */ 
	  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
	#endif
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void RCC_Configuration(void)
{
	//SystemInit();
	  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {
    /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    /* Flash 2 wait state */
    FLASH_SetLatency(FLASH_Latency_2);
 
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK/2 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

    /* PLLCLK = 8MHz * 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }	
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void Init_All_Periph(void)
{
    u32 * pAddress;
	u32   data;

	RCC_Configuration();

	//GPIO_Configuration();
	//jumper - 0, no jumper - 1
	/* Flash unlock */
    //FLASH_Unlock();
    /* Clear All pending flags */
    //FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

	//FLASHStatus = FLASH_ErasePage(ADDRESS_LAST_PAGE);
    //if(FLASHStatus == FLASH_COMPLETE)
	//   led_dn(BOARD_LED_ON);
	    
	pAddress = (u32 *)ADDRESS_LAST_PAGE;
	data  = *pAddress;
	//if((data == 0xFF) && (!(GPIO_ReadInputData(GPIOA) & 0x10)) )
	if( ( data & 0xff ) == 0xFF ) 
	   s_jump_to(0x08008000);
    cntPageSaved = ((data & 0x0000FF00) >> 8);

	// no jumper - in bootloader
    if (SysTick_Config(SystemCoreClock / 1000))  //1ms
	    { 
	      /* Capture error */ 
	      while (1);
	    }
		
	NVIC_Configuration();
    //while(SysTick_Config(SystemFrequency / 100000));  //10mks	  
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

int main(void)
{  
//     ****   DEBUG


//    ****** END OF DEBUG

	Init_All_Periph();

    ledDioGPIOInit();

    Delay(1000);
	led_dn(BOARD_LED_ON);
	led_mid(BOARD_LED_ON);

	//gprsModemInit();
	  //gprsModemOff();
    //gprsSIM2Sel();
	//gprsModemOn();  // debug
	//ftpGSMPrepare();

	/* Flash unlock */
    FLASH_Unlock();
    /* Clear All pending flags */
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

 	while(1)
  	{

	   //loopFTP();
	   //buttonScan();
	   UpdatingFlash();
	   //Delay(ONE_SEC/2);
	   //led_mid(BOARD_LED_XOR);
	   //if(getButtonUserPressed())
	     // {
		 //  gsmStart = 1;
		 // }
  	}
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void go_cmd(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  u32 Address = ADDRESS_LAST_PAGE;
  u32 Data;
  FLASHStatus = FLASH_ErasePage(ADDRESS_LAST_PAGE);

  Data = 0xFFFEFFFF;  //bit32 equals 0
  if( FLASHStatus == FLASH_COMPLETE)
    {
	  while((Address < (ADDRESS_LAST_PAGE+FLASH_PAGE_SIZE)) && (FLASHStatus == FLASH_COMPLETE))
	  {
	    FLASHStatus = FLASH_ProgramWord(Address,Data );
	    Address = Address + 4;
		Data = 0xFFFFFFFF;
	  }
    }
  
     
 /*
 When the address is valid and the checksum is correct, the bootloader firmware performs 
the following:
1 It initializes the registers of the peripherals used by the bootloader to their default reset 
values.
2 It initializes the user application's main stack pointer.
3 It jumps to the memory location programmed in the received ‘address + 4’ (which 
corresponds to the address of the application's reset handler).
For example, if the received address is 0x0800 0000, the bootloader will jump to the 
memory location programmed at address 0x0800 0004.
   */
  //__set_MSP(0x20000400);
  ///*
  GPIO_DeInit(GPIOD);
  GPIO_DeInit(GPIOA);

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);
  RCC_DeInit();
  __disable_irq();
  // restart system
  NVIC_SystemReset();

  //*/
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void UpdatingFlash(void)
{
  static u16 curPage = 0;
  //if(updateFlag)
  // {
	  // 1- for the first page, 2 - for the others 
   //   write_firm_pageFTP(updateFlag);
   //   updateFlag = 0;
  // }
  while(cntPageSaved--)
    {
      read_page(START_ADDRESS_FW_SRC + curPage*FLASH_PAGE_SIZE, firmwareBuf);
      write_firm_page(START_ADDRESS_FW_DEST + curPage*FLASH_PAGE_SIZE,firmwareBuf);
      curPage++;
      led_mid(BOARD_LED_XOR);
      
    }
    /*here should make added check for crc before go to main app - there is danger that some power unstable could be*/
  go_cmd();
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

u8 write_firm_page(u32 adr, u8 *buf)
{
  u16 byteCnt = 0;
  u32 adres = adr;
  u32 data;
  volatile FLASH_Status FLASHStatus;

 /*  FLASH Word program*/
  FLASHStatus = FLASH_ErasePage(adr);
  if( FLASHStatus == FLASH_COMPLETE)
    {
	  while((adres < (adr+FLASH_PAGE_SIZE)) && (FLASHStatus == FLASH_COMPLETE))
	  {
		data = *((u32 *)&buf[byteCnt]);
	    FLASHStatus = FLASH_ProgramWord(adres,data );
	    adres = adres + 4;
		byteCnt += 4;
	  }
    }
  return FLASHStatus;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void read_page(u32 adr, u8 *buf)
{
  u16 cntCycles = 0;
  bl  = (u32*)buf;
  while(cntCycles < FLASH_PAGE_SIZE)
    {
      *bl++ = *(vu32*)adr; 
      adr += 4;         
      cntCycles += 4; 
    }
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void write_firm_pageFTP(u8 sign)
{
  static u16 pageCnt = 0;
  u32 startAdres = 0;
  u16 byteCnt = 0;
  if(sign == 1)
    pageCnt = 0;
  //USART1_SendByte('b');

 /*  FLASH Word program*/
  Address	= START_ADDRESS_FW_DEST + pageCnt * FLASH_PAGE_SIZE;
  startAdres = Address;
  //handle if quantity of bytes does not equal to pageSize(not full page)
  
  if(byteCntPGSize != (FLASH_PAGE_SIZE))
    {
	  //flagLastPage = 1;
      while(byteCntPGSize < (FLASH_PAGE_SIZE))
  	      firmwareBuf[byteCntPGSize++] = 0xFF;
    }
  byteCntPGSize = 0;
  FLASHStatus = FLASH_ErasePage(Address);
  GPIO_XorBits(GPIOD, GPIO_Pin_8);
  if( FLASHStatus == FLASH_COMPLETE)
    {
	  while((Address < (startAdres+FLASH_PAGE_SIZE)) && (FLASHStatus == FLASH_COMPLETE))
	  {
        //Data = firmwareBuf[byteCnt] | firmwareBuf[byteCnt+1]<<8 | firmwareBuf[byteCnt+2]<<16 | firmwareBuf[byteCnt+3]<<24;
		Data = *((u32 *)&firmwareBuf[byteCnt]);
	    FLASHStatus = FLASH_ProgramWord(Address,Data );
	    Address = Address + 4;
		byteCnt += 4;
	  }
    }
  GPIO_XorBits(GPIOD, GPIO_Pin_9);
  pageCnt++;
  //USART1_SendByte('e');
  //USART1_SendByte('\n');

}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void erase_1page(void)
{

 FLASHStatus = FLASH_ErasePage(0x08008000 );
 //if( FLASHStatus == FLASH_COMPLETE)
    //USART1_SendByte('E');
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

static void s_jump_to(uint32_t appaddr)
{
    ///*
    volatile uint32_t jumpaddr;
    void (*app_fn)(void) = 0;

    // prepare jump address
    jumpaddr = *(__IO uint32_t*) (appaddr + 4);
    // prepare jumping function
    app_fn = (void (*)(void)) jumpaddr;
    // initialize user application's stack pointer
    __set_MSP(*(__IO uint32_t*) appaddr);
    // jump.
    app_fn();
   //*/

}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
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
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
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

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void GPIO_XorBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));
    
  if(GPIOx->ODR & GPIO_Pin)
     GPIOx->BRR = GPIO_Pin;
  else 
     GPIOx->BSRR = GPIO_Pin;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/


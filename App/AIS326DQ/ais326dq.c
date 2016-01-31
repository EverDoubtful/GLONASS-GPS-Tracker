/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : ais326dq.c
* Author             : AlexKolpakov
* Version            : V1.0
* Date               : 7-December-2011
* Description        : 
*******************************************************************************/

#include "ais326dq.h"
//#include "accel.h"
#include "dio_led.h"
#include "bin_handle.h"
#include "common.h"
#define Dummy_Byte 0xA5

extern struct ais326dq_data_t ais326dq_out;
u32 timerValueAccel;
u8  flagAccelTimerReady;

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval : None
  */
void ais326dq_init(void)
{
   uint8_t who;
   SPI_init();
   SELECT_ACCEL;
   SPI_ReadREG(WHO_AM_I,&who);
   DESELECT_ACCEL;

   //CTRL_REG1 = 0x47;
   SELECT_ACCEL;
   SPI_WriteREG(CTRL_REG1,0x47);
   DESELECT_ACCEL;
   SELECT_ACCEL;
   SPI_ReadREG(CTRL_REG1,&who);
   DESELECT_ACCEL;
   setTimerAccel(MS_100 * 5);

}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

u8 accelHandle(u8 * buf)
{
   u8 subCmd = *buf++;
   u8 nSize = 1;  //plus 1 byte of subcmd
   switch(subCmd)
    {
	  case SCMD_ACCEL_READ_XYZ: 
	     					 *buf++ = ais326dq_out.ax;
						     *buf++ = ais326dq_out.ay;
						     *buf++ = ais326dq_out.az;
							 nSize += 3;  //3 - accel code 
							 break;
	  default : break;
    }

   return nSize; 
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void accelScan(void)
{
   if(flagAccelTimerReady)
     {
	    flagAccelTimerReady = 0;
		setTimerAccel(MS_100 * 5);
        ais326dq_data(&ais326dq_out);
     }
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void ais326dq_data(struct ais326dq_data_t *data)
{
  //uint8_t val_x = 0,val_y = 0,val_z = 0 ;
//  SPI_ReadREG(OUTX_L,&val_x);
//  SPI_ReadREG(OUTX_L,&val_y);
//  data->ax = val_x | (val_y << 8);
   //uint8_t who;
  SELECT_ACCEL;
  //SPI_WriteREG(CTRL_REG1,0x47);
  //SPI_ReadREG(CTRL_REG1,&val_x);
  //SPI_ReadREG(WHO_AM_I,&who);
  //if(who == 0x3A)
  //  led_dn(BOARD_LED_ON);
  ///SPI_ReadREG(OUTX,&val_x);
  SPI_ReadREG(OUTX,&data->ax);

  //SPI_ReadREG(OUTY,&val_y);
  //data->ax = val_x | (val_y << 8);

  DESELECT_ACCEL;

   SELECT_ACCEL;
   SPI_ReadREG(OUTY,&data->ay);
   DESELECT_ACCEL;
   SELECT_ACCEL;
   SPI_ReadREG(OUTZ,&data->az);
   DESELECT_ACCEL;

}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void SPI_init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable SPI1 and GPIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA |
                         RCC_APB2Periph_GPIO_CS, ENABLE);

  /* Configure SPI1 pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure I/O for Flash Chip select */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIO_CS, &GPIO_InitStructure);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  /* SPI1 configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  /* Enable SPI1  */
  SPI_Cmd(SPI1, ENABLE);

}
/**
  * @brief  Reads a byte from the SPI Flash.
  *   This function must be used only if the Start_Read_Sequence
  *   function has been previously called.
  * @param  None
  * @retval : Byte Read from the SPI Flash.
  */
u8 SPI_ReadByte(void)
{
  return (SPI_SendByte(Dummy_Byte));
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte
  *   received from the SPI bus.
  * @param byte : byte to send.
  * @retval : The value of the received byte.
  */
u8 SPI_SendByte(u8 byte)
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, byte);

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void SPI_WriteREG(u8 address, u8 data)
{
   SPI_SendByte(address);
   SPI_SendByte(data);
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void SPI_ReadREG(u8 address, u8 *data)
{
   SPI_SendByte(address | ACCEL_RW_READ_SIGN);
   *data = SPI_ReadByte();
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void handleTimerAccel(void)
{
    if(timerValueAccel)
	  {
	   timerValueAccel--;
	   if(!timerValueAccel )
	      flagAccelTimerReady = 1;
	  }
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void setTimerAccel(u32 val)
{
   	timerValueAccel = val;
}




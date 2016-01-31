/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : bkp.c
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Endpoint routines
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "bkp.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
  #define BKP_DR_NUMBER              42
/* Interval between sending IN packets in frame number (1 frame = 1ms) */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u16 BKPDataReg[BKP_DR_NUMBER] =
  {
    BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4, BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8,
    BKP_DR9, BKP_DR10, BKP_DR11, BKP_DR12, BKP_DR13, BKP_DR14, BKP_DR15, BKP_DR16,
    BKP_DR17, BKP_DR18, BKP_DR19, BKP_DR20, BKP_DR21, BKP_DR22, BKP_DR23, BKP_DR24,
    BKP_DR25, BKP_DR26, BKP_DR27, BKP_DR28, BKP_DR29, BKP_DR30, BKP_DR31, BKP_DR32,
    BKP_DR33, BKP_DR34, BKP_DR35, BKP_DR36, BKP_DR37, BKP_DR38, BKP_DR39, BKP_DR40,
    BKP_DR41, BKP_DR42
  };  

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//-------------------------------------------------------------------------------------------

u16 BKPReadReg(u8 index)
{
  index %= BKP_DR_NUMBER;
  return BKP_ReadBackupRegister(BKPDataReg[index]);
}
//-------------------------------------------------------------------------------------------

void BKPWriteReg(u8 index, u16 data)
{
  index %= BKP_DR_NUMBER;
  BKP_WriteBackupRegister(BKPDataReg[index], data);
}
//-------------------------------------------------------------------------------------------
void BKPInit(void)
{
  /* Enable PWR and BKP clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Enable write access to Backup domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Clear Tamper pin Event(TE) pending flag */
  BKP_ClearFlag();

}



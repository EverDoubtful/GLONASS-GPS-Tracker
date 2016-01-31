/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_H
#define __COMMON_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#ifdef __cplusplus
 extern "C" {
#endif 

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define VERSION_MAIN    2 
#ifdef VER_3
  #define VERSION_SUB   2
#else
  #define VERSION_SUB   1
#endif
#define VERSION_BUILD	11

//#define BUF5_SIZE_RX    128

#define  MIN(a,b)   (a) < (b) ? (a) : (b)

#define CR_CODE         13		   // \r
#define NL_CODE         10		   // \n
#define CTRL_Z          0x1A

#define COMMA_CODE      0x2C
#define ASTERISK_CODE   0x2A
#define USD_CODE        0x24
#define GPRMC_STRING   	"GPRMC"
#define GNGSA_STRING    "GNGSA"
#define MLG_STRING      "MLG"
#define PGIO_STRING     "PGIO"

/* to update there have to be the same VERSION_MAIN; current SUB_VERSION has to be less or equal than new; current BUILD has to be less then new*/
//#define blockID       594782

#define ONE_SEC       (1000)
#define MS_100		  (100)
#define MS_10         (10)
#define MS_1          (1)

#define FIFO_PACKET_NAVI_SIZE     64
#define FIFO_PACKET_RECV_SIZE     8

//#define TIME_INTRV_GPS_QUERY      (ONE_SEC * 5)
#define  TIME_FIRST_INI            (ONE_SEC * 15)
/*  BIT-BAND */
#define Var_ResetBit_BB(VarAddr, BitNumber)  (*(vu32 *) (SRAM_BB_BASE | ((VarAddr - SRAM_BASE) << 5) | ((BitNumber) << 2)) = 0)
#define Var_SetBit_BB(VarAddr, BitNumber)    (*(vu32 *) (SRAM_BB_BASE | ((VarAddr - SRAM_BASE) << 5) | ((BitNumber) << 2)) = 1)
#define Var_GetBit_BB(VarAddr, BitNumber)    (*(vu32 *) (SRAM_BB_BASE | ((VarAddr - SRAM_BASE) << 5) | ((BitNumber) << 2)))

#define varSetBit(var,bit)   (Var_SetBit_BB((u32)&var,bit))
#define varResetBit(var,bit) (Var_ResetBit_BB((u32)&var,bit))
#define varGetBit(var,bit)   (Var_GetBit_BB((u32)&var,bit))

#define PRF_ResetBit_BB(VarAddr, BitNumber)  (*(vu32 *) (PERIPH_BB_BASE | ((VarAddr - PERIPH_BASE) << 5) | ((BitNumber) << 2)) = 0)
#define PRF_SetBit_BB(VarAddr, BitNumber)    (*(vu32 *) (PERIPH_BB_BASE | ((VarAddr - PERIPH_BASE) << 5) | ((BitNumber) << 2)) = 1)
#define PRF_GetBit_BB(VarAddr, BitNumber)    (*(vu32 *) (PERIPH_BB_BASE | ((VarAddr - PERIPH_BASE) << 5) | ((BitNumber) << 2)))

#define PRFSetBit(var,bit)   (PRF_SetBit_BB((u32)&var,bit))
#define PRFResetBit(var,bit) (PRF_ResetBit_BB((u32)&var,bit))
#define PRFGetBit(var,bit)   (PRF_GetBit_BB((u32)&var,bit))

//#define LED_MIDDLE_ADR     GPIOD_BASE + 0x0C  //0c - ODR register
//#define LED_MIDDLE        PRF_GetBit_BB(LED_MIDDLE_ADR, 14)
//#define _PORTD(i) (*((__O unsigned long *) (PERIPH_BB_BASE + ((GPIOD_BASE+0x0C-PERIPH_BASE) * 0x20) + (i*4))))

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
//void system_restart(void);
#define  SYSTEM_RESTART         system_restart()

//void GPIO_XorBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
int  hex2char(int _C);
int  int2char(char * pbuf, int data, const char field_size, const char base);
u8   strhex2int(u8 * buf);
//void bufCopy(const u8 * src,u8 * dst, u32 len);
u8   crcNavi(const u8 *buf,u8 len);
void TimingDelay_Decrement(void);
u8   bufCmpUser(const u8 * src,const u8 * dst, u8 len);
void handleTimerBtn(void);
void goTimerBtn(u32 delay);
u8   getFlagBtn(void);
u8   getLenNum(u32 num);
void Delay(__IO uint32_t nTime);
void TimingDelay_Decrement(void);


void handleTimerIWDG(void);
void setTimerIWDG(u32 val);
void monitorWatchDog(void);

//int  SendChar(int ch);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_H */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/

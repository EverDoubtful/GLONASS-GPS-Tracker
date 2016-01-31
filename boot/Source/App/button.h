
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BUTTON_H
#define __BUTTON_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define USER_BUTTON      0x01
#define WAKEUP_BUTTON    0x02
#define TAMPER_BUTTON    0x04

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
u8   buttonPressed(void);
void setButtonUserPressed(u8 code);
u8   getButtonUserPressed(void);
void buttonScan(void);
void handleTimerBtn(void);
#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

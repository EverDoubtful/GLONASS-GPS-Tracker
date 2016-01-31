
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ALARM_H
#define __ALARM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define ALARM_OFF         0
#define ALARM_CLOSING     1
#define ALARM_BREAKING    2

#define ALARM_IN1         0x01
#define ALARM_IN2         0x02
#define ALARM_IN3         0x04
#define ALARM_IN4         0x08

#define ALARM_IN1_ON      (1 << 0)
#define ALARM_IN2_ON      (1 << 1)
#define ALARM_IN3_ON      (1 << 2)
#define ALARM_IN4_ON      (1 << 3)

#define ALARM_AIN1_GND      (1 << 8)
#define ALARM_AIN1_VIN      (1 << 9)
#define ALARM_AIN1_CLOSED   (1 << 10)
#define ALARM_AIN1_BROKEN   (1 << 11)
#define ALARM_AIN2_GND      (1 << 12)
#define ALARM_AIN2_VIN      (1 << 13)
#define ALARM_AIN2_CLOSED   (1 << 14)
#define ALARM_AIN2_BROKEN   (1 << 15)

#define ALARM_AIN3_GND      (1 << 16)
#define ALARM_AIN3_VIN      (1 << 17)
#define ALARM_AIN3_CLOSED   (1 << 18)
#define ALARM_AIN3_BROKEN   (1 << 19)
#define ALARM_AIN4_GND      (1 << 20)
#define ALARM_AIN4_VIN      (1 << 21)
#define ALARM_AIN4_CLOSED   (1 << 22)
#define ALARM_AIN4_BROKEN   (1 << 23)

#define ALARM_AIN5_GND      (1 << 24)
#define ALARM_AIN5_VIN      (1 << 25)
#define ALARM_AIN5_CLOSED   (1 << 26)
#define ALARM_AIN5_BROKEN   (1 << 27)
#define ALARM_AIN6_GND      (1 << 28)
#define ALARM_AIN6_VIN      (1 << 29)
#define ALARM_AIN6_CLOSED   (1 << 30)
#define ALARM_AIN6_BROKEN   0x80000000	 //changed form - just to press warnings
/*Levels*/
#define ALARM_LVL_GND         100
#define ALARM_LVL_CLOSED_DN   (530-50)
#define ALARM_LVL_CLOSED_UP   (530+50)
#define ALARM_LVL_BROKEN_DN   (800-50)
#define ALARM_LVL_BROKEN_UP   (800+50)
#define ALARM_LVL_VIN         1500

/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
void handleTimerAlarm(void);
void alarmInit(void);
#ifdef __cplusplus
}
#endif

#endif /* __ALARM_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

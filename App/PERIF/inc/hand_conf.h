/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : .h
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Header for spi_flash.c file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HAND_CONF_H
#define __HAND_CONF_H
#include "common.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define        PROTOCOL_BINARY      1
#define        PROTOCOL_ASCII       2

//----------------------SETTINGS  sector------------------------

#define SIZE_MENU_VALUE           32

//----------------------END OF SETTINGS sector------------------------
/* ID CONSTANTS for # of parameter*/

#define CNF_UPD_CONFIG            0

#define CNF_BLOCK_ID              1
#define CNF_INTERVAL_SEND_PWR     2
#define CNF_INTERVAL_SEND_BATT    3
#define CNF_FLAG_TRACK_CONTROL    4
#define CNF_VEL_LIMIT             5
#define CNF_ANGLE_LIMIT           6
#define CNF_DIST_LIMIT            7
#define CNF_TURN_OFF_VOLT         8
#define CNF_MENU_PARAM            10   //not edit

#define CNF_GUARD                 11   //not edit
#define CNF_ALARM1                12   
#define CNF_ALARM2                13   
#define CNF_ALARM3                14   
#define CNF_ALARM4                15   
#define CNF_ALARM_AIN1            16
#define CNF_ALARM_AIN2            17
#define CNF_ALARM_AIN3            18
#define CNF_ALARM_AIN4            19
#define CNF_ALARM_AIN5            20
#define CNF_ALARM_AIN6            21

#define CNF_MENU_GPRS             22		 //not edit
#define CNF_MENU_SIM1             23		 //not edit
#define CNF_MENU_SIM2             24		 //not edit



#define CNF_APN1                  30
#define CNF_USERNAME1             31
#define CNF_PASSWORD1             32

#define CNF_APN2                  33
#define CNF_USERNAME2             34
#define CNF_PASSWORD2             35

#define CNF_SERVER1               36
#define CNF_PORT1                 37
#define CNF_SERVER2               38
#define CNF_PORT2                 39

#define CNF_CONTROL_NUM1          40
#define CNF_CONTROL_NUM2          41

#define CNF_BALANCE_SIM1          42
#define CNF_BALANCE_SIM2          43

#define CNF_MICROPH_SENS          44
#define CNF_DYN_VOL               45

#define CNF_MY_PHONE_SIM1         46
#define CNF_MY_PHONE_SIM2         47


#define CNF_MENU_FTP              50		 //not edit
#define CNF_FTP_ADRES             51		 
#define CNF_FTP_USERNAME          52		 
#define CNF_FTP_PASSWORD          53		 

#define CNF_PASSWORD              60		 

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
//typedef struct _version
//{
//  u8 mainVer;
//  u8 subVer;
//  u16 build;
//}t_version;

 typedef struct _config
{
  /* main settings*/
  u32 idBlock;                 // 0 - 999999
  u16 intervalSendingPower;    // 5 - 600
  u16 intervalSendingBattery;  // 30 - 600
  /* settings for track control*/
  u8  flagTrackControl;        // flag should turn on all this control(0 - no, 1 - yes)
  u16 velocityLimit;           // 10 - 1000
  u16 angleLimit;              // 5 - 360
  u16 distanceLimit;           // 100 - 50000 meters
  /*settings for microphone and dynamic */
  u8  microPhoneSens;
  u8  dynamicLevel;
  u16 turnOffVoltage;		   // 5-30V (store in values *10 , example 10V - means 100; 10.6V - 106 and so on)
//  u8  flagAlarm1;	/* bit0: set alarm ON(1) or OFF(0) , bit1: React on CLOSING(0) or BREAKING(1)*/
  u8  flagAlarm1;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  u8  flagAlarm2;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  u8  flagAlarm3;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  u8  flagAlarm4;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/

  u8  flagAlarmAIN1;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  u8  flagAlarmAIN2;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  u8  flagAlarmAIN3;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  u8  flagAlarmAIN4;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  u8  flagAlarmAIN5;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  u8  flagAlarmAIN6;	/* 0 - OFF, 1 - on CLOSING, 2 - on BREAKING*/
  /* GPRS settings */
  /*SIM*/
  u8  apn[2][SIZE_MENU_VALUE];
  u8  username[2][SIZE_MENU_VALUE/2];
  u8  password[2][SIZE_MENU_VALUE/2];
  /*main and reserv servers*/
  u8  server[2][SIZE_MENU_VALUE/2];  // standard mask xxx.xxx.xxx.xxx(15) + 1 just in case
  u16 port[2];

  u8  contrNumber[2][SIZE_MENU_VALUE/2];
  
  /*FTP PARAMETERS*/ 
  u8  ftpServer[SIZE_MENU_VALUE/2];
  u8  ftpUsername[SIZE_MENU_VALUE/2];
  u8  ftpPassword[SIZE_MENU_VALUE/2];
  /*Password to enter config*/
  u8  pswConfig[SIZE_MENU_VALUE/2];

  u8  getBalUSSD[2][8];
  u8  getPhNumUSSD[2][12];

  /* Some flags of system*/

  /* crc */
  u32 crc;
   
}configSet;
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void readConfig(void);
u8   writeConfig(u16 nSize, u8 * data);
u8   checkPsw(u8 * buf);
void defaultConfig(void);
void readGPSSettings(u8 chooseProtocol);
void handleADCChange(void);
void setTimerAnChanging(u32 val);
void handleTimerAnChanging(void);
void handleFOS(void);
void handleUSBPresent(void);
u8   getProtocol(void);

#ifdef __cplusplus
}
#endif
#endif /* __HAND_CONF_H */

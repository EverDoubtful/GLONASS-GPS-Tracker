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
#ifndef __PROTOCOL_BIN_H
#define __PROTOCOL_BIN_H
#include "common.h"

#ifdef __cplusplus
 extern "C" {
#endif

/*
bit	Значение		
0	Пройдена установленная дистанция		
//1	Изменился вектор движения		
//2	Запись в журнал по времени		
3	Изменение состояния дискретных датчиков		
4	Изменение состояния аналоговых датчиков		
//5	Изменение состояния датчиков RS485		
6	Потеря спутников GLONASS/GPS		
7	Потеря GPRS соединения		
8	Смена названия сотовой сети		
9	Остановка ТС		
10	Начало движения ТС		
11	Превышение разрешенной скорости		
12	Отключено внешнее питание		
13	Низкий заряд батареи		
14	Запуск системы		
15	Превышение ускорения/замедления по оси X		
16	Превышение ускорения/замедления по оси Y		
17	Превышение ускорения/замедления по оси Z		
18	Превышение температуры термодатчиков		
19	Смена курса на заданный угол		
20	Активная сим-карта (0 - Master; 1-Slave)		
21	Резерв ( всегда 0 )		
//22	Переполнения внутренней памяти		
23	Резерв ( всегда 0 )		
24	Резерв ( всегда 0 )		
25	Резерв ( всегда 0 )		
26	Резерв ( всегда 0 )		
27	Резерв ( всегда 0 )		
28	Резерв ( всегда 0 )		
29	Резерв ( всегда 0 )		
30	Резерв ( всегда 0 )		
31	Резерв ( всегда 0 )		
*/
typedef struct _flagsOfSystem
{
  u8 reserv6:1;          /* bit 24*/
  u8 reserv7:1;
  u8 reserv8:1;
  u8 reserv9:1;
  u8 reserv10:1;
  u8 reserv11:1;
  u8 reserv12:1;
  u8 reserv13:1;       /* bit 31*/

  u8 courseChanged:1;      /* bit 16*/
  u8 activeSIMCard:1;
  u8 sysHealth:1;      /* 0 -normal start, 1- watch-dog timer triggered*/
  u8 digOutputChanged:1;
  u8 SDPresence:1;
  u8 reserv3:1;
  u8 reserv4:1;
  u8 reserv5:1;            /* bit 23*/

  u8 vehicleSpeedExceed:1;  /* bit 8*/
  u8 extPowerOff:1;
  u8 lowBattery:1;
  u8 systemStart:1;
  u8 axisXChange:1;
  u8 axisYChange:1;
  u8 axisZChange:1;
  u8 tempGaugeExceed:1;    /* bit 15*/


  u8 distPassed:1;          /* bit 0*/
  u8 digInputChanged:1;
  u8 anInputChanged:1;
  u8 naviLost:1;
  u8 GPRSLost:1;
  u8 cellNameChanged:1;
  u8 vehicleStop:1;
  u8 vehicleStart:1;       /* bit 7*/

}flagsOfSystem;

typedef struct _bitsOfPresence
{
  u8 curDataState:1;           /* bit 24*/
  u8 gsmLocation:1;
  u8 reserv9:1;
  u8 reserv10:1;
  u8 reserv11:1;
  u8 reserv12:1;
  u8 reserv13:1;
  u8 reserv14:1;        /* bit 31*/
 
  u8 mileageChange:1;   /* bit 16*/
  u8 sat:1;
  u8 operatorName:1;   //changed
  u8 alarm:1;
  u8 stADC5:1;
  u8 stADC6:1;
  u8 stADC7:1;
  u8 stADC4:1;          /* bit 23*/

  u8 stateThermo3:1;     /* bit 8*/
  u8 stateThermo4:1;
  u8 stateThermo5:1;
  u8 stateThermo6:1;
  u8 stateThermo7:1;
  u8 stateThermo8:1;
  u8 stateFuel1:1;
  u8 stateFuel2:1;       /* bit 15*/


  u8 digInput:1;          /* bit 0*/
  u8 digOutput:1;
  u8 stADC0:1;
  u8 stADC1:1;
  u8 stADC2:1;
  u8 stADC3:1;
  u8 stateThermo1:1;
  u8 stateThermo2:1;       /* bit 7*/

}bitsOfPresence;

__packed typedef struct _sat_info
{
  u32 longitude;
  u32 latitude;	    //bytes: 0 - grad, 1 - min, 2,3 - sec
  u32 timeLabel;
  u16 altitude;
  u16 direction;
  u16 speedKM;
  u8  naviNumGLN;   
  u8  naviNumGPS;   
  u8  signs;     //bit0 - sign of long(0-plus,1-negative) bit1 - sign of lat
  //should use one more byte  
}sat_info;

__packed typedef struct _gsm_loc_info
{
  u16 mcc;	 /* mobile country code*/
  u8  mnc;	 /* mobile network code*/
  u16 lac;	 /* location area code*/
  u16 cid;	 /* cell ID code*/
}gsm_loc_info;

typedef struct _bin_info
{
  /*  Constant part */
  //u32            uID;
  u32            orderMsgNumber;
  flagsOfSystem  fos;
  bitsOfPresence bop;
  
  /* Variable part */
  u8  stateInput;
  u8  stateOutputs; 
#if defined (VER_3)
  u16 stateADC[8];   //1,2 - ADC converting , 3,4 - supply power & battery voltage - should present 
#else
  u16 stateADC[4];   //1,2 - ADC converting , 3,4 - supply power & battery voltage - should present 
#endif
  u16 stateThermo[8];
  u16 stateFuel[2];
  u32 mileage;        // will transmit when there was 1 meter changing
  u8  opName[16];
  sat_info  sat;
  u32  alarm;		 // alarms on DI happened
  gsm_loc_info gsm_loc; 
}bin_info;

//#define CV_OP_NAME 1

typedef enum
{
  ModemOn = 0,  
  ModemOff,  
  Ini,  
  DataMode,  
  VoiceMode,  
  ReIni,  
  FTPUpgrade,  
  USBUpgrade,  
  GPRSUpgrade,  
  Connected

}tDataState;

typedef struct _innerState
{

  u8 needRestart       : 1;          /* bit 0*/
  u8 bootloaderStarted : 1;
  u8 bootFTPStarted    : 1;
  u8 activeSIMCard     : 1;    //0 - master, 1  -slave
  u8 currentServer     : 1;    //0 - main, 1- slave
  u8 reconnectServer   : 1;	   // 0 - current mode, 1 - should reconnect(when changing adres or port from any interface)
  u8 allowedChange     : 1;	   // 1 - if password right
  u8 fwUpdated         : 1;    /*bit 7*/// 1 - if there was firmware updated, 0 - no        
  u8 PowerMainBatt     : 1;    /*bit 8*/// 0 - main power, 1 - Battery
  u8 flagDebug         : 1;	   // 0 - no debug, 1 - yes
  u8 goRestart         : 1;          
  u8 flagSDDisappeared : 1;
  u8 flagSDPhysAppeared: 1;
  u8 flCellNameChanged : 1; 
  u8 flGotGSMLocation  : 1; 
  u8 flagProtNoSend    : 1;
  u8 flagTmpDebug      : 1;	   // 0 - no debug, 1 - yes
  
  tDataState dataSt;
  tDataState savedDataSt;

  u32 alarm;

}t_innerState;

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
u16 prepareBinData(u8 * bufExt, u8 flag);
void executeDelayedCmd(void);
void handleTimerPrBin(void);
u16 quittance(void);

#ifdef __cplusplus
}
#endif
#endif /* __PROTOCOL_BIN_H */

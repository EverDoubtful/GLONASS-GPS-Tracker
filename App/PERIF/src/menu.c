/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "menu.h"
#include "hw_config.h"
#include "common.h"
#include "string.h"
#include "hand_conf.h"
#include "bin_handle.h"
#include "protocol_bin.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

#define NULL 0  
char menuTempBuf[SIZE_MENU_VALUE];

extern configSet config;

/* Declarations */
 
 const struct menu_item menuPARAMS;             /* pages name*/
     const struct menu_item menuIDBlock;
     const struct menu_item menuInterPwr;
     const struct menu_item menuInterBatt;
     const struct menu_item flagTrackControl;
     const struct menu_item menuVelLimit;
     const struct menu_item menuAngleLimit;
     const struct menu_item menuDistLimit;
     const struct menu_item menuTurnOffVolt;
     const struct menu_item menuMicrophone;
     const struct menu_item menuSpeaker;
 const struct menu_item menuGPRS;               /* pages name*/
     const struct menu_item menuSIM1;
         const struct menu_item menuAPN1;
         const struct menu_item menuUName1;
         const struct menu_item menuPass1;
         const struct menu_item menuCUSSD1;
         const struct menu_item menuCPHNUM1;
     const struct menu_item menuSIM2;
         const struct menu_item menuAPN2;
         const struct menu_item menuUName2;
         const struct menu_item menuPass2;
         const struct menu_item menuCUSSD2;
         const struct menu_item menuCPHNUM2;
	 const struct menu_item menuServer1;
     const struct menu_item menuPort1;
     const struct menu_item menuServer2;
     const struct menu_item menuPort2;
     const struct menu_item menuCNum1;
     const struct menu_item menuCNum2;
 const struct menu_item menuFTP;               /* pages name*/
     const struct menu_item menuFTPAdres;
     const struct menu_item menuFTPUsername;
     const struct menu_item menuFTPPassword;
 const struct menu_item menuGuard;             /* pages name*/
     const struct menu_item menuAlarm1;
     const struct menu_item menuAlarm2;
     const struct menu_item menuAlarm3;
     const struct menu_item menuAlarm4;
     const struct menu_item menuAlarmAIN1;
     const struct menu_item menuAlarmAIN2;
     const struct menu_item menuAlarmAIN3;
     const struct menu_item menuAlarmAIN4;
     const struct menu_item menuAlarmAIN5;
     const struct menu_item menuAlarmAIN6;

/* Definitions */ 
  const struct menu_item menuPARAMS            = { &menuIDBlock,NULL,NULL,&menuGPRS,"Params", false, CNF_MENU_PARAM, CHAR_TYPE_DATA, 0, 0 };
       const struct menu_item menuIDBlock      = { NULL,&menuPARAMS,NULL,&menuInterPwr,"ID block", true, CNF_BLOCK_ID,U32_TYPE_DATA, 1,999999};
       const struct menu_item menuInterPwr     = { NULL,&menuPARAMS,&menuIDBlock,&menuInterBatt,"Server Sending Interval, s", true, CNF_INTERVAL_SEND_PWR,U16_TYPE_DATA, 5,600};
       const struct menu_item menuInterBatt    = { NULL,&menuPARAMS,&menuInterPwr,&flagTrackControl,"Battery  Sending Interval, s", true, CNF_INTERVAL_SEND_BATT,U16_TYPE_DATA , 30, 600};
       const struct menu_item flagTrackControl = { NULL,&menuPARAMS,&menuInterBatt,&menuVelLimit,"Track Control", true, CNF_FLAG_TRACK_CONTROL,BOOL_TYPE_DATA , 0, 1};
       const struct menu_item menuVelLimit     = { NULL,&menuPARAMS,&flagTrackControl,&menuAngleLimit,"Velocity Trigger(km/h)", true, CNF_VEL_LIMIT,U16_TYPE_DATA , 5, 600};
       const struct menu_item menuAngleLimit   = { NULL,&menuPARAMS,&menuVelLimit,&menuDistLimit,"Angle Trigger(grad)", true, CNF_ANGLE_LIMIT,U16_TYPE_DATA , 5, 360};
       const struct menu_item menuDistLimit    = { NULL,&menuPARAMS,&menuAngleLimit,&menuTurnOffVolt,"Distance Trigger(m)", true, CNF_DIST_LIMIT,U16_TYPE_DATA , 10, 50000};
       const struct menu_item menuTurnOffVolt  = { NULL,&menuPARAMS,&menuDistLimit,&menuMicrophone,"TurnOff Voltage(V)", true, CNF_TURN_OFF_VOLT,U16_TYPE_DATA , 50, 300};
       const struct menu_item menuMicrophone   = { NULL,&menuPARAMS,&menuTurnOffVolt,&menuSpeaker,"Microphone sensitivity ", true, CNF_MICROPH_SENS,U8_TYPE_DATA , 0, 15};
       const struct menu_item menuSpeaker      = { NULL,&menuPARAMS,&menuMicrophone,NULL,"Dynamic volume", true, CNF_DYN_VOL, U8_TYPE_DATA , 0, 100};

  const struct menu_item menuGPRS              = { &menuSIM1,NULL,&menuPARAMS,&menuFTP,"GPRS", false, CNF_MENU_GPRS,CHAR_TYPE_DATA , 0, 0};
       const struct menu_item menuSIM1         = { &menuAPN1,&menuGPRS,NULL,&menuSIM2,"Master", false, CNF_MENU_SIM1,CHAR_TYPE_DATA , 0, 0};
           const struct menu_item menuAPN1     = { NULL,&menuSIM1,NULL,&menuUName1,"APN1", true, CNF_APN1,CHAR_TYPE_DATA, 5, SIZE_MENU_VALUE};
           const struct menu_item menuUName1   = { NULL,&menuSIM1,&menuAPN1,&menuPass1,"UserName1", true, CNF_USERNAME1,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
           const struct menu_item menuPass1    = { NULL,&menuSIM1,&menuUName1,&menuCUSSD1,"Password1", true, CNF_PASSWORD1,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
           const struct menu_item menuCUSSD1   = { NULL,&menuSIM1,&menuPass1,&menuCPHNUM1,"Balance CUSD-query", true, CNF_BALANCE_SIM1,CHAR_TYPE_DATA , 3 , 8};
           const struct menu_item menuCPHNUM1  = { NULL,&menuSIM1,&menuCUSSD1,NULL,"My Phone CUSD-query", true, CNF_MY_PHONE_SIM1,CHAR_TYPE_DATA , 3 , 12};
       const struct menu_item menuSIM2         = { &menuAPN2,&menuGPRS,&menuSIM1,&menuServer1,"Slave", false, CNF_MENU_SIM2,CHAR_TYPE_DATA ,0 ,0 };
           const struct menu_item menuAPN2     = { NULL,&menuSIM2,NULL,&menuUName2,"APN2", true, CNF_APN2,CHAR_TYPE_DATA, 5, SIZE_MENU_VALUE};
           const struct menu_item menuUName2   = { NULL,&menuSIM2,&menuAPN2,&menuPass2,"UserName2", true, CNF_USERNAME2,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
           const struct menu_item menuPass2    = { NULL,&menuSIM2,&menuUName2,&menuCUSSD2,"Password2", true, CNF_PASSWORD2,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
           const struct menu_item menuCUSSD2   = { NULL,&menuSIM2,&menuPass2,&menuCPHNUM2,"Balance CUSD-query", true, CNF_BALANCE_SIM2,CHAR_TYPE_DATA , 3 , 8};
           const struct menu_item menuCPHNUM2  = { NULL,&menuSIM2,&menuCUSSD2,NULL,"My Phone CUSD-query", true, CNF_MY_PHONE_SIM2,CHAR_TYPE_DATA , 3 , 12};
       const struct menu_item menuServer1      = { NULL,&menuGPRS,&menuSIM2,&menuPort1,"Main Server Address", true, CNF_SERVER1,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
       const struct menu_item menuPort1        = { NULL,&menuGPRS,&menuServer1,&menuServer2,"Port", true, CNF_PORT1,U16_TYPE_DATA , 1 , 65536};
       const struct menu_item menuServer2      = { NULL,&menuGPRS,&menuPort1,&menuPort2,"Reserv Server Address", true, CNF_SERVER2,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
       const struct menu_item menuPort2        = { NULL,&menuGPRS,&menuServer2,&menuCNum1,"Port", true, CNF_PORT2,U16_TYPE_DATA , 1 , 65536};
       const struct menu_item menuCNum1        = { NULL,&menuGPRS,&menuPort2,&menuCNum2,"Control Number1", true, CNF_CONTROL_NUM1,CHAR_TYPE_DATA , 0 , 13};	 //13 - contry code(3dig) + 10 dig cell code
       const struct menu_item menuCNum2        = { NULL,&menuGPRS,&menuCNum1,NULL,"Control Number2", true, CNF_CONTROL_NUM2,CHAR_TYPE_DATA , 0 , 13};
 const struct menu_item menuFTP                = { &menuFTPAdres,NULL,&menuGPRS,&menuGuard,"FTP", false, CNF_MENU_FTP,CHAR_TYPE_DATA , 0, 0};
     const struct menu_item menuFTPAdres       = { NULL,&menuFTP,NULL,&menuFTPUsername,"IP-address", true, CNF_FTP_ADRES,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
     const struct menu_item menuFTPUsername    = { NULL,&menuFTP,&menuFTPAdres,&menuFTPPassword,"Username", true, CNF_FTP_USERNAME,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
     const struct menu_item menuFTPPassword    = { NULL,&menuFTP,&menuFTPUsername,NULL,"Password", true, CNF_FTP_PASSWORD,CHAR_TYPE_DATA , 3 , SIZE_MENU_VALUE};
 const struct menu_item menuGuard              = { &menuAlarm1,NULL,&menuFTP,NULL,"Guard", false, CNF_GUARD,CHAR_TYPE_DATA , 0, 0};
       const struct menu_item menuAlarm1       = { NULL,&menuGuard,NULL,&menuAlarm2,"IN1 Alarm", true, CNF_ALARM1, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarm2       = { NULL,&menuGuard,&menuAlarm1,&menuAlarm3,"IN2 Alarm", true, CNF_ALARM2, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarm3       = { NULL,&menuGuard,&menuAlarm2,&menuAlarm4,"IN3 Alarm", true, CNF_ALARM3, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarm4       = { NULL,&menuGuard,&menuAlarm3,&menuAlarmAIN1,"IN4 Alarm", true, CNF_ALARM4, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarmAIN1    = { NULL,&menuGuard,&menuAlarm4,&menuAlarmAIN2,"AIN1 Alarm", true, CNF_ALARM_AIN1, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarmAIN2    = { NULL,&menuGuard,&menuAlarmAIN1,&menuAlarmAIN3,"AIN2 Alarm", true, CNF_ALARM_AIN2, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarmAIN3    = { NULL,&menuGuard,&menuAlarmAIN2,&menuAlarmAIN4,"AIN3 Alarm", true, CNF_ALARM_AIN3, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarmAIN4    = { NULL,&menuGuard,&menuAlarmAIN3,&menuAlarmAIN5,"AIN4 Alarm", true, CNF_ALARM_AIN4, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarmAIN5    = { NULL,&menuGuard,&menuAlarmAIN4,&menuAlarmAIN6,"AIN5 Alarm", true, CNF_ALARM_AIN5, GUARD_TYPE_DATA , 0, 2};
       const struct menu_item menuAlarmAIN6    = { NULL,&menuGuard,&menuAlarmAIN5,NULL,"AIN6 Alarm", true, CNF_ALARM_AIN6, GUARD_TYPE_DATA , 0, 2};

 struct menu_item *miCurPtr;                  /*current point of transfer*/
//child,parent,up,down
   
  /* Declarations of Current Values*/
  //const struct menu_itemCV mOpName;             

   /* Definitions of Current Values*/ 
  //const struct menu_itemCV mOpName            = { NULL,NULL,NULL,NULL,"Operator Name:", CV_OP_NAME, CHAR_TYPE_DATA };

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
u16 menuPointDisplay(struct menu_item *miPtr, u8 * buf);

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u16  menuDisplay(u8 * buf, u8 * signLongPack, u16 limit)
{
  u16 len = 0;
  static struct menu_item *miSavedPtr = NULL;
  static u8 nrPack = 0;
  *signLongPack = 0;
  if(miSavedPtr == NULL)
  	  miCurPtr = (struct menu_item *)&menuPARAMS;
  else
  	  miCurPtr = miSavedPtr;
    
  while(miCurPtr)
   {
      if(len < limit)
	    {
	      len += menuPointDisplay(miCurPtr, &buf[len]);
	      if(miCurPtr->child)
	        {
	          miCurPtr    = ( struct menu_item *)miCurPtr->child; 
	        }
	      else if(miCurPtr->down)
	       {
	          miCurPtr    = ( struct menu_item *)miCurPtr->down;
	       }
	      else
	       {
	          miCurPtr    = ( struct menu_item *)miCurPtr->parent;
	          if(miCurPtr)
	             miCurPtr    = ( struct menu_item *)miCurPtr->down;
	       }
	    }
	  else
	    {
	  		miSavedPtr = miCurPtr;
			nrPack++;
			*signLongPack = nrPack;
			buf += len;
  			return len;
	    }
   }
  if(nrPack)  //if there was long packet then we mark this pack as last 
    {
     *signLongPack = 0xFF;
	 nrPack = 0;
	}
  miSavedPtr = NULL;
  buf += len;
  return len; 

}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

u16 menuPointDisplay(struct menu_item *miPtr, u8 * buf)
{
  u16 len = 0;
  u8  temp;
  /* id */
  *buf++ = miPtr->id;
  /*child*/
  *buf++ = (miPtr->child) ? miPtr->child->id : 0; 
  /*parent*/
  *buf++ = (miPtr->parent) ? miPtr->parent->id : 0; 
  /*up*/
  *buf++ = (miPtr->up) ? miPtr->up->id : 0; 
  /*down*/
  *buf++ = (miPtr->down) ? miPtr->down->id : 0; 
  /*edit*/
  *buf++ = miPtr->edit? 1 : 0 ;
  /* type Data */
  *buf++ = miPtr->typeData;
  /* min */
  *buf++ = miPtr->min;
  /* max */
  *(u32*)buf = miPtr->max;
  buf += 4;
  len = 12; /* sizeof(child->id + parent->id + up->id + down->id + edit + id + typedata + min  + max)*/
  /*name*/
  temp = strlen(( char * )miPtr->name);
  strncpy( (char *)buf ,( char * )miPtr->name, temp );
  buf += temp;
  *buf++ = 0;
  len += temp+1; //+1 - because of finished 0
  temp = 0;
  /*value*/
  //memset(menuTempBuf, 0, SIZE_MENU_VALUE);

  switch(miPtr->id)
   {
     //case  0:  
     //strcpy((char *)buf, "NULL"); 																				                    break;

     case  CNF_BLOCK_ID          : *(u32*)buf = config.idBlock; temp = 4;                                             						break;
     case  CNF_INTERVAL_SEND_PWR : *(u16*)buf =config.intervalSendingPower; temp = 2;                        						break;   
     case  CNF_INTERVAL_SEND_BATT: *(u16*)buf =config.intervalSendingBattery;temp = 2; 												break;
     case  CNF_FLAG_TRACK_CONTROL: *(u8*) buf =config.flagTrackControl; temp = 1;            										break;
     case  CNF_VEL_LIMIT         : *(u16*)buf =config.velocityLimit;  temp = 2;                 									break;
     case  CNF_ANGLE_LIMIT       : *(u16*)buf =config.angleLimit;     temp = 2;                     								break;
     case  CNF_DIST_LIMIT        : *(u16*)buf =config.distanceLimit;  temp = 2;                  									break;
     case  CNF_TURN_OFF_VOLT     : *(u16*)buf =config.turnOffVoltage; temp = 2;                  									break;
     case  CNF_MICROPH_SENS      : *(u8*) buf =config.microPhoneSens; temp = 1;            										break;
     case  CNF_DYN_VOL           : *(u8*) buf =config.dynamicLevel;   temp = 1;            										break;

     case  CNF_ALARM1            : *(u8*)buf = config.flagAlarm1; temp = 1;            										break;
     case  CNF_ALARM2            : *(u8*)buf = config.flagAlarm2; temp = 1;            										break;
     case  CNF_ALARM3            : *(u8*)buf = config.flagAlarm3; temp = 1;            										break;
     case  CNF_ALARM4            : *(u8*)buf = config.flagAlarm4; temp = 1;            										break;
     case  CNF_ALARM_AIN1        : *(u8*)buf = config.flagAlarmAIN1; temp = 1;            									break;
     case  CNF_ALARM_AIN2        : *(u8*)buf = config.flagAlarmAIN2; temp = 1;            									break;
     case  CNF_ALARM_AIN3        : *(u8*)buf = config.flagAlarmAIN3; temp = 1;            									break;
     case  CNF_ALARM_AIN4        : *(u8*)buf = config.flagAlarmAIN4; temp = 1;            									break;
     case  CNF_ALARM_AIN5        : *(u8*)buf = config.flagAlarmAIN5; temp = 1;            									break;
     case  CNF_ALARM_AIN6        : *(u8*)buf = config.flagAlarmAIN6; temp = 1;            									break;

     case  CNF_APN1              : temp = strlen((char *)config.apn[0]);
	 								strncpy((char *)buf , (char *)config.apn[0] ,temp);                         						break;
     case  CNF_USERNAME1         : temp = strlen((char *)config.username[0]);
	 								strncpy((char *)buf , (char *)config.username[0] ,temp);               break;
     case  CNF_PASSWORD1         : temp = strlen((char *)config.password[0]); 
	 								strncpy((char *)buf , (char *)config.password[0] ,temp);               break;
     case  CNF_BALANCE_SIM1      : temp = strlen((char *)config.getBalUSSD[0]); 
	 								strncpy((char *)buf , (char *)config.getBalUSSD[0] ,temp);             break;
     case  CNF_MY_PHONE_SIM1     : temp = strlen((char *)config.getPhNumUSSD[0]); 
	 								strncpy((char *)buf , (char *)config.getPhNumUSSD[0] ,temp);             break;

     case  CNF_APN2              : temp = strlen((char *)config.apn[1]); 
	 								strncpy((char *)buf , (char *)config.apn[1] ,temp);                         break;
     case  CNF_USERNAME2         : temp = strlen((char *)config.username[1]);
	  								strncpy((char *)buf , (char *)config.username[1] ,temp);               break;
     case  CNF_PASSWORD2         : temp = strlen((char *)config.password[1]);
									strncpy((char *)buf , (char *)config.password[1] ,temp);               break;
     case  CNF_BALANCE_SIM2      : temp = strlen((char *)config.getBalUSSD[1]); 
	 								strncpy((char *)buf , (char *)config.getBalUSSD[1] ,temp);             break;
     case  CNF_MY_PHONE_SIM2     : temp = strlen((char *)config.getPhNumUSSD[1]); 
	 								strncpy((char *)buf , (char *)config.getPhNumUSSD[1] ,temp);             break;

     case  CNF_SERVER1           : temp = strlen((char *)config.server[0]);
	 								strncpy((char *)buf , (char *)config.server[0] ,temp);                   break;
     case  CNF_PORT1             : *(u16*)buf =config.port[0]; temp = 2;                           									 break;
	  								
     case  CNF_SERVER2           : temp = strlen((char *)config.server[1]);
	  								strncpy((char *)buf , (char *)config.server[1] ,temp);                   break;
     case  CNF_PORT2             : *(u16*)buf =config.port[1]; temp = 2;                          									 break;

     case  CNF_CONTROL_NUM1      : temp = strlen((char *)config.contrNumber[0]);
	 								strncpy((char *)buf , (char *)config.contrNumber[0] ,temp);         break;
     case  CNF_CONTROL_NUM2      : temp = strlen((char *)config.contrNumber[1]);
	  								strncpy((char *)buf , (char *)config.contrNumber[1] ,temp);         break;

     case  CNF_FTP_ADRES         : temp = strlen((char *)config.ftpServer);
	 							    strncpy((char *)buf , (char *)config.ftpServer ,temp);                         						break;
     case  CNF_FTP_USERNAME      : temp = strlen((char *)config.ftpUsername);
	 								strncpy((char *)buf , (char *)config.ftpUsername ,temp);               break;
     case  CNF_FTP_PASSWORD      : temp = strlen((char *)config.ftpPassword); 
	 								strncpy((char *)buf , (char *)config.ftpPassword ,temp);               break;

     default: break;
   }
  buf += temp;
  *buf++ = 0;
  len += temp + 1;  //+1 - because of finished 0 
  return len;
}


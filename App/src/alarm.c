/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "alarm.h"
#include "protocol_bin.h"
#include "dio_led.h"
#include "adc.h"
#include "hand_conf.h"
//#include <stdio.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u32 timerValueAlarm;
extern t_innerState innerState;
extern configSet config;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static void setTimerAlarm(u32 val);
//-------------------------------------------------------------------------------------------
void alarmInit(void)
{
  setTimerAlarm(ONE_SEC * (5 + 3));  //5 - adc init, 3 - reserv
}
// ------------------------------------------------------------------------------
void handleTimerAlarm(void)
{
    u16 temp;
    if(timerValueAlarm)
	  {
	   timerValueAlarm--;
	   if(!timerValueAlarm )
	     {

		   temp = readStateInput();
		   /* Analyse IN1*/
		   if(config.flagAlarm1 == ALARM_CLOSING)
		     {
			   if(temp & ALARM_IN1)  //CLOSED 
		          innerState.alarm |= ALARM_IN1_ON;
		     }
		   else if(config.flagAlarm1 == ALARM_BREAKING)
		     {
			   if(!(temp & ALARM_IN1))	 //BROKEN
		          innerState.alarm |= ALARM_IN1_ON;
		     }
		   /* Analyse IN2*/
		   if(config.flagAlarm2 == ALARM_CLOSING)
		     {
			   if(temp & ALARM_IN2)  //CLOSED 
		          innerState.alarm |= ALARM_IN2_ON;
		     }
		   else if(config.flagAlarm2 == ALARM_BREAKING)
		     {
			   if(!(temp & ALARM_IN2))	 //BROKEN
		          innerState.alarm |= ALARM_IN2_ON;
		     }
			 /* Analyse IN3*/ 
		   if(config.flagAlarm3 == ALARM_CLOSING)
		     {
			   if(temp & ALARM_IN3)  //CLOSED 
		          innerState.alarm |= ALARM_IN3_ON;
		     }
		   else if(config.flagAlarm3 == ALARM_BREAKING)
		     {
			   if(!(temp & ALARM_IN3))	 //BROKEN
		          innerState.alarm |= ALARM_IN3_ON;
		     }
			 /* Analyse IN4*/
		   if(config.flagAlarm4 == ALARM_CLOSING)
		     {
			   if(temp & ALARM_IN4)  //CLOSED 
		          innerState.alarm |= ALARM_IN4_ON;
		     }
		   else if(config.flagAlarm4 == ALARM_BREAKING)
		     {
			   if(!(temp & ALARM_IN4))	 //BROKEN
		          innerState.alarm |= ALARM_IN4_ON;
		     }

		   temp = ain_read(2);
		   /* Analyse AIN1*/
		   if(temp < ALARM_LVL_GND)
		       innerState.alarm |= ALARM_AIN1_GND;
		   else if(temp > ALARM_LVL_VIN)
		       innerState.alarm |= ALARM_AIN1_VIN;
		   else if(config.flagAlarmAIN1 == ALARM_CLOSING)
		     {
			   if( (temp < ALARM_LVL_CLOSED_UP) && (temp > ALARM_LVL_CLOSED_DN) )             
		          innerState.alarm |= ALARM_AIN1_CLOSED;
		     }
		   else if(config.flagAlarmAIN1 == ALARM_BREAKING)
		     {
			   if( (temp < ALARM_LVL_BROKEN_UP) && (temp > ALARM_LVL_BROKEN_DN) )             
		          innerState.alarm |= ALARM_AIN1_BROKEN;
		     }
		   temp = ain_read(3);
		   /* Analyse AIN2*/
		   if(temp < ALARM_LVL_GND)
		       innerState.alarm |= ALARM_AIN2_GND;
		   else if(temp > ALARM_LVL_VIN)
		       innerState.alarm |= ALARM_AIN2_VIN;
		   else if(config.flagAlarmAIN2 == ALARM_CLOSING)
		     {
			   if( (temp < ALARM_LVL_CLOSED_UP) && (temp > ALARM_LVL_CLOSED_DN) )             
		          innerState.alarm |= ALARM_AIN2_CLOSED;
		     }
		   else if(config.flagAlarmAIN2 == ALARM_BREAKING)
		     {
			   if( (temp < ALARM_LVL_BROKEN_UP) && (temp > ALARM_LVL_BROKEN_DN) )             
		          innerState.alarm |= ALARM_AIN2_BROKEN;
		     }
		   temp = ain_read(4);
		   /* Analyse AIN3*/
		   if(temp < ALARM_LVL_GND)
		       innerState.alarm |= ALARM_AIN3_GND;
		   else if(temp > ALARM_LVL_VIN)
		       innerState.alarm |= ALARM_AIN3_VIN;
		   else if(config.flagAlarmAIN3 == ALARM_CLOSING)
		     {
			   if( (temp < ALARM_LVL_CLOSED_UP) && (temp > ALARM_LVL_CLOSED_DN) )             
		          innerState.alarm |= ALARM_AIN3_CLOSED;
		     }
		   else if(config.flagAlarmAIN3 == ALARM_BREAKING)
		     {
			   if( (temp < ALARM_LVL_BROKEN_UP) && (temp > ALARM_LVL_BROKEN_DN) )             
		          innerState.alarm |= ALARM_AIN3_BROKEN;
		     }
		   temp = ain_read(5);
		   /* Analyse AIN4*/
		   if(temp < ALARM_LVL_GND)
		       innerState.alarm |= ALARM_AIN4_GND;
		   else if(temp > ALARM_LVL_VIN)
		       innerState.alarm |= ALARM_AIN4_VIN;
		   else if(config.flagAlarmAIN4 == ALARM_CLOSING)
		     {
			   if( (temp < ALARM_LVL_CLOSED_UP) && (temp > ALARM_LVL_CLOSED_DN) )             
		          innerState.alarm |= ALARM_AIN4_CLOSED;
		     }
		   else if(config.flagAlarmAIN4 == ALARM_BREAKING)
		     {
			   if( (temp < ALARM_LVL_BROKEN_UP) && (temp > ALARM_LVL_BROKEN_DN) )             
		          innerState.alarm |= ALARM_AIN4_BROKEN;
		     }
		   temp = ain_read(6);
		   /* Analyse AIN5*/
		   if(temp < ALARM_LVL_GND)
		       innerState.alarm |= ALARM_AIN5_GND;
		   else if(temp > ALARM_LVL_VIN)
		       innerState.alarm |= ALARM_AIN5_VIN;
		   else if(config.flagAlarmAIN5 == ALARM_CLOSING)
		     {
			   if( (temp < ALARM_LVL_CLOSED_UP) && (temp > ALARM_LVL_CLOSED_DN) )             
		          innerState.alarm |= ALARM_AIN5_CLOSED;
		     }
		   else if(config.flagAlarmAIN5 == ALARM_BREAKING)
		     {
			   if( (temp < ALARM_LVL_BROKEN_UP) && (temp > ALARM_LVL_BROKEN_DN) )             
		          innerState.alarm |= ALARM_AIN5_BROKEN;
		     }
		   temp = ain_read(7);
		   /* Analyse AIN6*/
		   if(temp < ALARM_LVL_GND)
		       innerState.alarm |= ALARM_AIN6_GND;
		   else if(temp > ALARM_LVL_VIN)
		       innerState.alarm |= ALARM_AIN6_VIN;
		   else if(config.flagAlarmAIN6 == ALARM_CLOSING)
		     {
			   if( (temp < ALARM_LVL_CLOSED_UP) && (temp > ALARM_LVL_CLOSED_DN) )             
		          innerState.alarm |= ALARM_AIN6_CLOSED;
		     }
		   else if(config.flagAlarmAIN6 == ALARM_BREAKING)
		     {
			   if( (temp < ALARM_LVL_BROKEN_UP) && (temp > ALARM_LVL_BROKEN_DN) )             
		          innerState.alarm |= ALARM_AIN6_BROKEN;
		     }

		   setTimerAlarm(MS_100);
		 }
	  }   
}
// ------------------------------------------------------------------------------
static void setTimerAlarm(u32 val)
{
   	timerValueAlarm = val;
}


#include "nmea/info.h"


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIME_RTC2_H
#define __TIME_RTC2_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

						      
/* Exported types ------------------------------------------------------------*/
typedef struct {
	uint16_t year;	/* 1..4095 */
	uint8_t  month;	/* 1..12 */
	uint8_t  mday;	/* 1.. 31 */
	uint8_t  wday;	/* 0..6, Sunday = 0*/
	uint8_t  hour;	/* 0..23 */
	uint8_t  min;	/* 0..59 */
	uint8_t  sec;	/* 0..59 */
	uint8_t  dst;	/* 0 Winter, !=0 Summer */
} RTC_t;
				      
/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
int  rtc_init(void);
void rtc_gettime (RTC_t*);					/* Get time */
char *  readRTCTime(RTC_t *rtc);
u32  getRealTime(void);
void rtcCorrectTimeFromGLONASS (RTC_t *rtc, nmeaINFO * infoPtr );
u32  rtcGetNaviTime ( nmeaINFO * infoPtr );
u16  getRTCTime(u8 * buf);
u32 rtcGetRealTime2( void );

#ifdef __cplusplus
}
#endif

#endif /* __TIME_RTC2_H */

/*******************  *****END OF FILE****/

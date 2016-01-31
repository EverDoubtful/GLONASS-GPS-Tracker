
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_H
#define __PLATFORM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

						      
/* Exported types ------------------------------------------------------------*/
				      
/* Exported constants --------------------------------------------------------*/
#define ONE_SEC       1000
#define MS_100		  100
#define MS_10         10
#define MS_1          1
#define TIME_FIRST_INI (ONE_SEC * 15) 

#define FLASH_PAGE_SIZE	   2048
#define PAGE_SIZE          FLASH_PAGE_SIZE + 10
#define FTP_PACKET_SIZE    1024
#define FTP_QTY_READ_TRY   3
/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

#ifndef __FTP_H
#define __FTP_H

#ifdef __cplusplus
 extern "C" {
#endif

//#define FLASH_PAGE_SIZE	   2048
//#define PAGE_SIZE          FLASH_PAGE_SIZE + 10
#define FTP_PACKET_SIZE    1024
#define FTP_QTY_READ_TRY   5



void loopFTP(void);
void handleTimerSendGSMFTP(void);
void ftpGSMPrepare(void);
//void handleTimerTimeoutGSMFTP(void);
void checkReadRawData(u8 data);
#ifdef __cplusplus
}
#endif

#endif  //__FTP_H

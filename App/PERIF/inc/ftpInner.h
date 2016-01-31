#ifndef __FTP_INNER_H
#define __FTP_INNER_H

#ifdef __cplusplus
 extern "C" {
#endif

void initializeSMFTP(void);
void setTimerSendFTP(u32 val);
void setTimerTimeoutFTP(u32 val);
u8   checkTimeoutFunFTP(void);
static void cmdPrepareSend(u8 * buf, u32 timerVal);
void handleRcvFTP(void);
static void setTimerSendAux(u32 val);
#ifdef __cplusplus
}
#endif

#endif  //__FTP_INNER_H

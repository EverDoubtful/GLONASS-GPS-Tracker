#ifndef __FTP_H
#define __FTP_H

void initializeSMFTP(void);
void loopFTP(void);
u8 getLenNum(u16 num);
int int2char(char * pbuf, int data, const char field_size, const char base);
int hex2char(int _C);
void handleTimerSendGSM(void);
void setTimerSend(u32 val);
void ftpGSMPrepare(void);
void handleTimerTimeoutGSM(void);
void setTimerTimeout(u32 val);
u8   checkTimeoutFun(void);

#endif  //__FTP_H

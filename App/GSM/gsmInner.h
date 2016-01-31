
#ifndef __GSM_INNER_H
#define __GSM_INNER_H
#ifdef __cplusplus
 extern "C" {
#endif 


void setTimerGSMPresence(u32 val);
void setTimerLoopQuery(u32 val);
void setTimerTimeout(u32 val);
void setTimerBalance(u32 val);
void setTimerPhNum(u32 val);
void setTimerNAVIAbs(u32 val);
void setTimerNAVIPres(u32 val);

u8   checkTimeoutFun(void);
void checkCalling(void);
void iniGSM(void);

void handleRcvGSM(void);
static void cmdPrepareSend(u8 * buf, u32 timerVal);
static void setTimerSendAux(u32 val);

void setMicrophoneSens(void);
void setSpeakerLevel(void);

void parseSMS(void);
void changeGPRSServer(void);
u8   checkPhones(u8 * buf);

#ifdef __cplusplus
}
#endif

#endif  //__GSM_INNER_H

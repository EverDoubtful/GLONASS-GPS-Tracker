
#ifndef __GSM_H
#define __GSM_H
#ifdef __cplusplus
 extern "C" {
#endif 

void loopGSM(void);
void setupGSM(void);
void handleTimerGSMPresence(void);
void setGPRSServer(u8 idxServer);
void setGSMOperator(u8 idxSimCard);
void setCUSDQuery(u8 idxSimCard);
void setCUSDPhNumQuery(u8 idxSimCard);


u8   getInfoGsm(void);
u16 getBalance(u8 * buf);
u16 getPhoneNumber(u8 * buf);

void callGSMFunction(u8 state);
					
#ifdef __cplusplus
}
#endif

#endif  //__GSM_H

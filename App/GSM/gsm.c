

#include "string.h"
#include "gps.h"
#include "gprs_modem.h"
#include "dio_led.h"
#include "hw_config.h"
#include "gsm.h"
#include "bkp.h"
#include "gsmInner.h"
#include "button.h"
#include "fifoPackets.h"
//#include "fifo.h"
#include "sdcardUser.h"
#include "flash.h"
#include "hand_conf.h"
#include "protocol_bin.h"
#include <stdlib.h>
#include "packets.h"

#define  TIME_INTRV_CMDS               (MS_100)
#define  TIME_INTRV_START_TCP          (MS_100 * 5)
#define  TIME_INTRV_CHANGE_SERVER      (ONE_SEC * 10)

//intervals
#define  TIMEOUT_1MIN            (ONE_SEC * 60)
#define  TIMEOUT_USUAL_CMD       (ONE_SEC * 3 )
#define  TIMEOUT_BEGIN_GPRS_CMD  (ONE_SEC * 10)
#define  TIMEOUT_END_GPRS_CMD    (ONE_SEC * 10)  
#define  TIMEOUT_CIPCLOSE        (ONE_SEC * 20)
#define  TIMEOUT_OPEN_TCP_CMD    (ONE_SEC * 30)     //TIMEOUT_1MIN
#define  TIMEOUT_WAIT_SEND_OK    (TIMEOUT_1MIN )
#define  TIMEOUT_GSM_PRESENCE    (TIMEOUT_1MIN * 10)
#define  TIMEOUT_CGATT1          (ONE_SEC * 10)


#define  RESEND_TIMEOUT_RETRY   (3)
#define  RESEND_ERROR_RETRY     (3)  

#define  SIZE_MAX_CUSD_BUFFER   (300)

//#define  SYSTEM_RESTART         led_mid(BOARD_LED_XOR)
////////////////////////temp sesctor  //////////////////////


void go_cmd(void);
static u16 cap;  //tempora
u8 getInfoGsm(void)
{
  return cap;
}

/////////////////////  end of temp sector////////////////////
typedef enum 
{
stCheckConn = 0,
stATE0,
stIPR,
stCPINASK,
stCSPN,
stCMGF,				 //5
stCMIC,
stCLVL,
stCSCS,
stCLIP,
stCSCLK,			 //10
stCGATT_ASK,
stCGATT1,
stCIPATS,
stCIPCSGP,
stIFC,				 //15
stEnTranspMode,
stCSTT,
stCIICR,
stCIFSR,
stCIPHEAD,			 //20
stCIPSTART,
stCIPSEND,
stRawData,
stCIPSHUT,
stCGATT0,			 //25
stCIPCLOSE,
stATH,
stATA,
stCUSD,
stCPHNUM,			 //30
stCIMI,
stCREG,
stCREGASK,
stCREGZERO,
stEmpty,			 //35
stNothing

}stateType;

stateType currState;
//stateType savedState;
stateType savedStateCNF;

void TbFunCheckConn(void);
void TbFunATE0(void);
void TbFunIPR(void);
void TbFunCPINASK(void);
void TbFunCSPN(void);
void TbFunCMGF(void);
void TbFunCMIC(void);
void TbFunCLVL(void);
void TbFunCSCS(void);
void TbFunCLIP(void);
void TbFunCSCLK(void);
void TbFunCGATT_ASK(void);
void TbFunCGATT1(void);
void TbFunCIPATS(void);
void TbFunCIPCSGP(void);
void TbFunIFC(void);
void TbFunEnTranspMode(void);
void TbFunCSTT(void);
void TbFunCIICR(void);
void TbFunCIFSR(void);
void TbFunCIPHEAD(void);
void TbFunCIPSTART(void);
void TbFunCIPSEND(void);
void TbFunRawData(void);
void TbFunCIPSHUT(void);
void TbFunCGATT0(void);
void TbFunCIPCLOSE(void);
void TbFunATH(void);
void TbFunATA(void);
void TbFunCUSD(void);
void TbFunCPHNUM(void);
void TbFunCIMI(void);
void TbFunCREG(void);
void TbFunCREGASK(void);
void TbFunCREGZERO(void);
void TbFunEmpty(void);
void TbFunNothing(void);

void  (*stateTable[])(void) ={
							    TbFunCheckConn,
								TbFunATE0,
								TbFunIPR,
								TbFunCPINASK,
								TbFunCSPN,
								TbFunCMGF,
								TbFunCMIC,
								TbFunCLVL,
								TbFunCSCS,
								TbFunCLIP,
								TbFunCSCLK,
								TbFunCGATT_ASK,
								TbFunCGATT1,
								TbFunCIPATS,
								TbFunCIPCSGP,
								TbFunIFC,
								TbFunEnTranspMode,
								TbFunCSTT,
								TbFunCIICR,
								TbFunCIFSR,
								TbFunCIPHEAD,
								TbFunCIPSTART,
								TbFunCIPSEND,
								TbFunRawData,
								TbFunCIPSHUT,
								TbFunCGATT0,
								TbFunCIPCLOSE,
								TbFunATH,
								TbFunATA,
								TbFunCUSD,
								TbFunCPHNUM,
								TbFunCIMI,
								TbFunCREG,
								TbFunCREGASK,
								TbFunCREGZERO,
								TbFunEmpty,
								TbFunNothing
							  
							 };
// ---------    new features   ------------

//   ---------------- end of new features


const u8 * QUERIES[] =     {
                        "AT\r\n",
   					    "AT+CREG=2\r\n",
						"AT+CGATT?\r\n",
						"AT+CSTT=",
						"AT+CIICR\r\n",
						"AT+CIFSR\r\n",	  //5
						"AT+CIPSTART=",
						"AT+CIPSEND\r\n",
						"AT+CIPCLOSE\r\n",
						"AT+CGATT=0\r\n",
						"AT+CIPMODE=0\r\n",	 //10
						"+++\r\n",
						"ATO\r\n",
						"AT+CIPSHUT\r\n",
						"AT+CSCLK=0\r\n",
						"AT+CGATT=1\r\n",		//15
						"AT+CGATT?\r\n",
						"AT+CIPATS=0\r\n",
						"AT+CIPCSGP=1,",
						"AT+IFC=0,0\r\n",
						"AT+CIPHEAD=1\r\n",	 //20
						"AT+CIPSEND=",		 //number of bytes to transfer
						"ATE0\r\n",
						"ATV0\r\n",
						"AT+CGATT=0\r\n",
						"AT+CPIN?\r\n",          //25		
						"AT+CLIP=1\r\n",
						"AT+CIPCLOSE\r\n",
						"ATH\r\n",
						"AT+CSPN?\r\n",
						"ATA\r\n",				//30
						"AT+CMGF=1\r\n",
						"AT+CSCS=\"GSM\"\r\n",
						"AT+CUSD=1,",
						"AT+CMIC=0,",
						"AT+CLVL=",				//35
						"AT+IPR=115200\r\n",
						"AT+CIMI\r\n",
						"AT+CREG?\r\n",
						"AT+CREG=0\r\n"
						};

//"AT+CUSD=1,\"*100#\",15\r\n"

																					  //6
const u8 * REPLIES[] = {
						"OK","ERROR","CONNECT OK","BUSY","NO CARRIER","NO ANSWER","NO DIALTONE",
						"+CGATT: ","CLOSED","SHUT OK","SEND OK","AT","+CPIN: READY",    /*12*/			  
						"+CLIP: ","ALREADY CONNECT","+CMT: ","CLOSE OK","+CSPN: ","STATE: TCP CLOSED","CONNECT FAIL",  /*19*/
						"+PDP: DEACT","+CPIN: NOT READY","+CUSD: ", "+CREG: 2,1," , "+CREG: 1," ,""
						};

#define REPL_OK_LEN              strlen((char *)REPLIES[0])      //OK
#define REPL_ERROR_LEN           strlen((char *)REPLIES[1])      //ERROR
#define REPL_CONNN_OK_LEN        strlen((char *)REPLIES[2])      //CONNECT OK
#define REPL_NO_CARRIER_LEN      strlen((char *)REPLIES[4])
#define REPL_CGATT_LEN           strlen((char *)REPLIES[7])      //CGATT
#define REPL_CLOSED_LEN	         strlen((char *)REPLIES[8])
#define REPL_SHUT_OK_LEN         strlen((char *)REPLIES[9])      //SHUT OK
#define REPL_SEND_OK_LEN	     strlen((char *)REPLIES[10])
#define REPL_CPIN_LEN            strlen((char *)REPLIES[12])     //+CPIN: READY
#define REPL_CLIP_LEN            strlen((char *)REPLIES[13])
#define REPL_ALR_CONN_LEN        strlen((char *)REPLIES[14])     //ALREADY CONNECT
#define REPL_CMT_LEN             strlen((char *)REPLIES[15])
#define REPL_CLOSE_OK_LEN        strlen((char *)REPLIES[16])     //CLOSE OK
#define REPL_CSPN_LEN            strlen((char *)REPLIES[17])
#define REPL_TCP_CLOSED_LEN      strlen((char *)REPLIES[18])
#define REPL_CON_FAIL_LEN        strlen((char *)REPLIES[19])
#define REPL_PDP_DEACT_LEN       strlen((char *)REPLIES[20])
#define REPL_CPIN_NOT_READY_LEN  strlen((char *)REPLIES[21])
#define REPL_CUSD_LEN            strlen((char *)REPLIES[22])
#define REPL_CREGASK_LEN         strlen((char *)REPLIES[23])
#define REPL_CREGURC_LEN         strlen((char *)REPLIES[24])

#define REPL_UPGRADE_LEN         strlen((char *)CRYPTO_PHRASE[0])

u16 packetCap;

const u8 * CRYPTO_PHRASE[] = {
                              "upgrade"
                             };

typedef struct _flagsGSMState
{
  u8 flagCPIN_ASK      : 1;           /* bit 0*/
  u8 volatile flagPrepareToSend : 1;
  u8 flagTimeToSend    : 1;
  u8 volatile flagStartLoopGSM  : 1;
  u8 flagOnceSended    : 1;   // when we first went through all configuration commands - no need to repeat them
  u8 volatile flagTimeout       : 1;
  u8 flagAlrSent       : 1;
  u8 flagCGATT_ASK_OK  : 1;
  u8 flagCIPSTARTError : 1;
  u8 flagGsmStart      : 1;
  u8 flagNewCycle      : 1;
  u8 flagLostPacket    : 1;
  u8 fTCPClosed        : 1;
  u8 flagIniFromFTP    : 1;
  u8 flagNaviAbsTimerStarted  : 1;
  u8 flagNaviAbs       : 1;
  u8 flagNaviPresTimerStarted  : 1;
  u8 flagNaviPres      : 1;
  u8 flagGotBalance    : 1; 
  u8 flagWaitBalance   : 1; 
  u8 resetBalance      : 1;
  u8 flagGotPhNum      : 1; 
  u8 flagWaitPhNum     : 1; 
  u8 resetPhNum        : 1;
  u8 flagTransaction   : 1;
  u8 flagCmdFromBuffer : 1;
}flagsGSMState;

flagsGSMState sGSMState;

extern u8  tempBuf[];   //REC_AN_BUF_SIZE
char       csttBuf[60];
char       cipstartBuf[50];
char       cipcsgpBuf[60];
char       cspnBuf[16];
char       cusdBuf[SIZE_MAX_CUSD_BUFFER];
char       cnfBuf[30];		 /*  "AT+CUSD=1,"",15\r\n"(17 bytes) + 8 max symbols of CUSD-query = 25*/

u8   bufTxGSM[MAX_SIZE_PACKET_GPRS_SIM900];
//u16  cntTxGSM;

u16  lenData = 0;
u8 volatile  flagNoGSM;

volatile u32  timerValueSend;
volatile u32  timerValueTimeout;
volatile u32  timerFDTimeout;
volatile u32  timerValueGSMPresence;
volatile u32  timerValueLoopQuery;
volatile u32  timerValueBalance;
volatile u32  timerValuePhNum;
volatile u32  timerValueNaviAbs;
volatile u32  timerValueNaviPres;

u8   numTryTimeoutResendAT;
u8   numTryTimeoutResendCmd;


extern fifo_packet fifoNaviPack;
u8 flagNaviValid;

u8 flagGoFTPLoad;
u8 flagGoFtpControl;
extern  u8  FTPStart;
extern configSet config;
extern t_innerState innerState;
extern bin_info  binInfo;
//extern fifo_buffer gsmTxRB;
//u8 * tmpGSMBuf;
node_struct   nodesGSMATCmd[8];
fifo_packet   fifoGSMATCmdPack;
u8            bufGSMFifoATCmd[50];
//-------------------------------------------------------------------------------------------
void iniGSM(void)
{
   setTimerSendAux(TIME_FIRST_INI);
   setTimerLoopQuery(MS_10);
   sGSMState.flagAlrSent = 0;

   flagNoGSM = 1;
   flagGoFtpControl = 0; //0 - gsm , 1- ftp
   sGSMState.flagOnceSended = 0;
   sGSMState.flagNewCycle = 0;
   currState = stCheckConn;
   innerState.dataSt = Ini;
   savedStateCNF = stNothing;
}
//-------------------------------------------------------------------------------------------

void setupGSM(void)
{ 

   setGSMOperator(innerState.activeSIMCard);
   setGPRSServer(innerState.currentServer);
   //setCUSDQuery(innerState.activeSIMCard);
   
   setTimerSendAux(TIME_FIRST_INI);
   setTimerLoopQuery(MS_10);
   sGSMState.flagAlrSent = 0;

   flagNoGSM = 1;
   flagGoFtpControl = 0; //0 - gsm , 1- ftp
   innerState.dataSt = Ini;
   sGSMState.flagIniFromFTP = 0;
   savedStateCNF = stNothing;
   
   fifoPacketInit(&fifoGSMATCmdPack,nodesGSMATCmd,sizeof(nodesGSMATCmd)/sizeof(nodesGSMATCmd[0]));
   //cntTxGSM = 0;
   sGSMState.flagTransaction   = 0;
   sGSMState.flagCmdFromBuffer = 0;
}
// ------------------------------------------------------------------------------
void changeGPRSServer(void)
{
   u32 temp;
   innerState.currentServer ^= 1;
   temp = innerState.activeSIMCard | (innerState.currentServer << 1);
   BKPWriteReg(BKP_DR_INNER_STATE_N1,temp);
   BKPWriteReg(BKP_DR_INNER_STATE_N2,temp >> 16);
  
   setGPRSServer(innerState.currentServer);
   innerState.dataSt = ReIni;
}

//-------------------------------------------------------------------------------------------
void setGPRSServer(u8 idxServer)
{
   if(idxServer == innerState.currentServer)
     {
	   memset(cipstartBuf,0,sizeof(cipstartBuf));
	   strcpy(cipstartBuf,(char *)QUERIES[6]);
	   //strncat(cipstartBuf,"\"TCP\",\"85.233.64.176\",\"8084\"\r\n",sizeof("\"TCP\",\"85.233.64.176\",\"8084\"\r\n") );
	   //strncat(cipstartBuf,"\"TCP\",\"31.13.128.67\",\"5000\"\r\n",sizeof("\"TCP\",\"31.13.128.67\",\"5000\"\r\n") );
	   //strncat(cipstartBuf,"\"TCP\",\"81.23.151.1\",\"5000\"\r\n",sizeof("\"TCP\",\"81.23.151.1\",\"5000\"\r\n") );	   //home
	   strncat(cipstartBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipstartBuf,(char *)"TCP",strlen((char *)"TCP") );
	   strncat(cipstartBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipstartBuf,",",strlen(",") );    /*add comma symbol*/
	   strncat(cipstartBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipstartBuf,(char *)config.server[idxServer],strlen((char *)config.server[idxServer]) );
	   strncat(cipstartBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipstartBuf,",",strlen(",") );    /*add comma symbol*/
	   strncat(cipstartBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   int2char((char * )&cipstartBuf[strlen(cipstartBuf)],config.port[idxServer],getLenNum(config.port[idxServer]),10);
	   strncat(cipstartBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipstartBuf,"\r\n",sizeof("\r\n") );

	   innerState.reconnectServer = 1;
	 }
}

//-------------------------------------------------------------------------------------------
void setGSMOperator(u8 idxSimCard)
{
   if(idxSimCard == innerState.activeSIMCard)
    {
	   memset(csttBuf,0,sizeof(csttBuf));
	   strcpy(csttBuf,(char *)QUERIES[3]);
	   strncat(csttBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(csttBuf,(char *)config.apn[idxSimCard],strlen((char *)config.apn[idxSimCard]) );
	   strncat(csttBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(csttBuf,",",strlen(",") );    /*add comma symbol*/
	   strncat(csttBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(csttBuf,(char *)config.username[idxSimCard],strlen((char *)config.username[idxSimCard]) );
	   strncat(csttBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(csttBuf,",",strlen(",") );    /*add comma symbol*/
	   strncat(csttBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(csttBuf,(char *)config.password[idxSimCard],strlen((char *)config.password[idxSimCard]) );
	   strncat(csttBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(csttBuf,"\r\n",sizeof("\r\n") );
	
	   memset(cipcsgpBuf,0,sizeof(cipcsgpBuf));
	   strcpy(cipcsgpBuf,(char *)QUERIES[18]);
	   //strncat(cipcsgpBuf,"\"internet.mts.ru\",\"mts\",\"mts\"\r\n",sizeof("\"internet.mts.ru\",\"mts\",\"mts\"\r\n") );
	   strncat(cipcsgpBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipcsgpBuf,(char *)config.apn[idxSimCard],strlen((char *)config.apn[idxSimCard]) );
	   strncat(cipcsgpBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipcsgpBuf,",",strlen(",") );    /*add comma symbol*/
	   strncat(cipcsgpBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipcsgpBuf,(char *)config.username[idxSimCard],strlen((char *)config.username[idxSimCard]) );
	   strncat(cipcsgpBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipcsgpBuf,",",strlen(",") );    /*add comma symbol*/
	   strncat(cipcsgpBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipcsgpBuf,(char *)config.password[idxSimCard],strlen((char *)config.password[idxSimCard]) );
	   strncat(cipcsgpBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	   strncat(cipcsgpBuf,"\r\n",sizeof("\r\n") );
    }
}
//-------------------------------------------------------------------------------------------
void setCUSDQuery(u8 idxSimCard)
{
	memset(cnfBuf,0,sizeof(cnfBuf));
	strcpy((char *)cnfBuf,(char *)QUERIES[33]);
	strncat(cnfBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	strncat(cnfBuf,(char *)config.getBalUSSD[idxSimCard],strlen((char *)config.getBalUSSD[idxSimCard]) );  
	strncat(cnfBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	strncat(cnfBuf,",15\r\n",strlen(",15\r\n") );  /*add  CR NL */
}
//-------------------------------------------------------------------------------------------
void setCUSDPhNumQuery(u8 idxSimCard)
{
	memset(cnfBuf,0,sizeof(cnfBuf));
	strcpy((char *)cnfBuf,(char *)QUERIES[33]);
	strncat(cnfBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	strncat(cnfBuf,(char *)config.getPhNumUSSD[idxSimCard],strlen((char *)config.getPhNumUSSD[idxSimCard]) );  
	strncat(cnfBuf,"\"",strlen("\"") );  /*add qoute symbol*/
	strncat(cnfBuf,",15\r\n",strlen(",15\r\n") );  /*add  CR NL */
}
//-------------------------------------------------------------------------------------------
void setMicrophoneSens(void)
{
	memset(cnfBuf,0,sizeof(cnfBuf));
	strcpy((char *)cnfBuf,(char *)QUERIES[34]);
    int2char((char * )&cnfBuf[strlen(cnfBuf)],config.microPhoneSens,getLenNum(config.microPhoneSens),10);
	strncat(cnfBuf,"\r\n",strlen("\r\n") );  /*add  CR NL */
}
//-------------------------------------------------------------------------------------------
void setSpeakerLevel(void)
{
	memset(cnfBuf,0,sizeof(cnfBuf));
	strcpy((char *)cnfBuf,(char *)QUERIES[35]);
    int2char((char * )&cnfBuf[strlen(cnfBuf)],config.dynamicLevel,getLenNum(config.dynamicLevel),10);
	strncat(cnfBuf,"\r\n",strlen("\r\n") );  /*add  CR NL */
}
/*******************************************************************************/
/* 				COMMANDS													   */
/*******************************************************************************/

void TbFunCheckConn(void)  //AT
{
  if( (sGSMState.flagGsmStart) || (!sGSMState.flagAlrSent) || (sGSMState.flagTimeout))
   {
    if(sGSMState.flagPrepareToSend)
	 { 
	  sGSMState.flagPrepareToSend = 0;
	  /*timeout handling*/
	  timerFDTimeout = TIMEOUT_USUAL_CMD;
	  /*ready to send*/
	  sGSMState.flagTimeToSend = 1;
	  //if(!checkTimeoutFun())
      cmdSend((u8 *)QUERIES[0]);
	  /*aux actions*/
	  sGSMState.flagAlrSent = 1;

	  /*timeout handling*/
      if(sGSMState.flagTimeout)
	   {
		numTryTimeoutResendAT++;
	    sGSMState.flagTimeout = 0;
	   }
	  else
	   {
		numTryTimeoutResendAT = 0;
	   }
	  /*aux actions*/
	  sGSMState.flagGsmStart = 0;
	  sGSMState.flagTransaction = 0;  /*free from blocking when transaction happened(just in case if there was no answer from 
	  								   at+cipsend(>) or from data send(send ok) ) */
      sGSMState.flagCmdFromBuffer = 0;
	 }

   }
}
//-------------------------------------------------------------------------------------------
void TbFunIPR(void)
{
    cmdPrepareSend((u8 *)QUERIES[36], TIMEOUT_USUAL_CMD);
}

//-------------------------------------------------------------------------------------------
void TbFunATE0(void)
{
    cmdPrepareSend((u8 *)QUERIES[22], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCPINASK(void)
{
      cmdPrepareSend((u8 *)QUERIES[25], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCSPN(void)
{
      cmdPrepareSend((u8 *)QUERIES[29], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCMGF(void)
{
      cmdPrepareSend((u8 *)QUERIES[31], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCMIC(void)
{
      //USBWriteStr("FUN CMIC called\r\n");
	  //setMicrophoneSens();
      //cmdPrepareSend((u8 *)cnfBuf, TIMEOUT_USUAL_CMD);
  if( (!sGSMState.flagAlrSent)  || (sGSMState.flagTimeout) )
     {
	    if(sGSMState.flagPrepareToSend)
		 { 
		  sGSMState.flagPrepareToSend = 0;
		  //USBWriteStr("FUN CMIC called\r\n");
	      setMicrophoneSens();
          /*timeout handling*/
		  timerFDTimeout = TIMEOUT_USUAL_CMD; 
		  /*ready to send*/
		  sGSMState.flagTimeToSend = 1;
		  if(!checkTimeoutFun())
		    {
	         cmdSend((u8 *)cnfBuf);
			}
		  /*aux actions*/
		  sGSMState.flagAlrSent = 1;
		 }
     }

}
//-------------------------------------------------------------------------------------------
void TbFunCLVL(void)
{
	  //setSpeakerLevel(); 
      //cmdPrepareSend((u8 *)cnfBuf, TIMEOUT_USUAL_CMD);
  if( (!sGSMState.flagAlrSent)  || (sGSMState.flagTimeout) )
     {
	    if(sGSMState.flagPrepareToSend)
		 { 
		  sGSMState.flagPrepareToSend = 0;
		  //USBWriteStr("FUN CLVL called\r\n");
	      setSpeakerLevel();
          /*timeout handling*/
		  timerFDTimeout = TIMEOUT_USUAL_CMD; 
		  /*ready to send*/
		  sGSMState.flagTimeToSend = 1;
		  if(!checkTimeoutFun())
		    {
	         cmdSend((u8 *)cnfBuf);
			}
		  /*aux actions*/
		  sGSMState.flagAlrSent = 1;
		 }
     }

}
//-------------------------------------------------------------------------------------------
void TbFunCSCS(void)
{
      cmdPrepareSend((u8 *)QUERIES[32], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCLIP(void)
{
		cmdPrepareSend((u8 *)QUERIES[26], TIMEOUT_USUAL_CMD);
}

//-------------------------------------------------------------------------------------------
void TbFunCSCLK(void)
{
		cmdPrepareSend((u8 *)QUERIES[14], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCGATT_ASK(void)
{
		cmdPrepareSend((u8 *)QUERIES[2], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCGATT1(void)
{
		cmdPrepareSend((u8 *)QUERIES[15], TIMEOUT_CGATT1);
}

//-------------------------------------------------------------------------------------------
void TbFunCIPATS(void)
{
		cmdPrepareSend((u8 *)QUERIES[17], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCIPCSGP(void)
{
      cmdPrepareSend((u8 *)cipcsgpBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunIFC(void)
{
    cmdPrepareSend((u8 *)QUERIES[19], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunEnTranspMode(void)
{
    cmdPrepareSend((u8 *)QUERIES[10], TIMEOUT_USUAL_CMD);
}
    
//-------------------------------------------------------------------------------------------
void TbFunCSTT(void)
{
    cmdPrepareSend((u8 *)csttBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCIICR(void)
{
   	  cmdPrepareSend((u8 *)QUERIES[4], TIMEOUT_BEGIN_GPRS_CMD);
}

//-------------------------------------------------------------------------------------------
void TbFunCIFSR(void) 
{
    cmdPrepareSend((u8 *)QUERIES[5], TIMEOUT_USUAL_CMD);	 //"AT+CIFSR\r\n"
}
//-------------------------------------------------------------------------------------------
void TbFunCIPHEAD(void)
{
    cmdPrepareSend((u8 *)QUERIES[20], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCIPSTART(void)
{
      if(innerState.reconnectServer)
	       innerState.reconnectServer = 0;
   	  cmdPrepareSend((u8 *)cipstartBuf, TIMEOUT_OPEN_TCP_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCIPSEND(void)
{
  u8 i;
  u8 locBuf[3+3];
  u16 debLen;
  u8 dumb;
  if(innerState.reconnectServer)
    {
	   innerState.reconnectServer = 0;
	   setTimerSendAux(TIME_INTRV_CMDS);
	   currState = stCIPCLOSE;
	} 
  else if( (!sGSMState.flagAlrSent)  || (sGSMState.flagTimeout) )
   {
    if(sGSMState.flagPrepareToSend)
     { 
	  if(!checkTimeoutFun())
	   {
	     
	     /*todo : should make some timer - not to query too often*/
		 cap = fifoPacketCount(&fifoNaviPack);
		 lenData = 0;
		 if(cap)
		    {
			  memset(bufTxGSM,0,sizeof(bufTxGSM)/sizeof(bufTxGSM[0]));
		      lenData = fifoPacketGet(&fifoNaviPack, bufTxGSM , &dumb);
			  /*
			  debLen = fifoPacketGetLen(&fifoNaviPack);
			  if( debLen )
			    {
			      if( debLen <= FIFO_Rest(&gsmTxRB))
			         lenData = fifoPacketGet(&fifoNaviPack, FIFO_PtrEnd(&gsmTxRB, debLen) );
				  else
				   {
					 tmpGSMBuf = malloc(debLen);
				     lenData = fifoPacketGet(&fifoNaviPack, tmpGSMBuf );
					 FIFO_PutAr(&gsmTxRB,tmpGSMBuf,lenData);
					 free( tmpGSMBuf );
				   }
				}
				*/
		    }
	      
   
		 if(lenData)
		   {
		     
		     while(lenData <= (MAX_SIZE_PACKET_GPRS_SIM900 - 0))
			   {
			     debLen = 0;
			     debLen = fifoPacketGetLen(&fifoNaviPack);
				 if( (debLen)	&& (( lenData + debLen ) <= MAX_SIZE_PACKET_GPRS_SIM900) )
				   {
		            fifoPacketGet(&fifoNaviPack, &bufTxGSM[lenData] , &dumb);
				   	lenData += debLen;
				   }
				 else
				    break;
			   }
			   /*
		     while(lenData <= (MAX_SIZE_PACKET_GPRS_SIM900 - 0))
			   {

				  debLen = fifoPacketGetLen(&fifoNaviPack);
				  if( (debLen)	&& (( lenData + debLen ) <= MAX_SIZE_PACKET_GPRS_SIM900) )
				    {
				      if( debLen <= FIFO_Rest(&gsmTxRB))
				         lenData += fifoPacketGet(&fifoNaviPack, FIFO_PtrEnd(&gsmTxRB, debLen) );
					  else
					   {
						 tmpGSMBuf = malloc(debLen);
					     debLen = fifoPacketGet(&fifoNaviPack, tmpGSMBuf );
						 lenData += debLen;
						 FIFO_PutAr(&gsmTxRB,tmpGSMBuf,debLen);
						 free( tmpGSMBuf );
					   }
					}
				  else 
				    break;
				}
 			 	 */

             sGSMState.flagPrepareToSend = 0;
	         cmdSend((u8 *)QUERIES[21]);
			 i = getLenNum(lenData);
			 int2char((char * )locBuf,lenData,i,10);
			 cmdSendLen(locBuf, i);
			 cmdSend((u8 *)"\r\n");
			 gsmWrite1Byte(CTRL_Z);
			 timerFDTimeout = TIMEOUT_USUAL_CMD;
		     sGSMState.flagAlrSent = 1;
	         sGSMState.flagTimeToSend = 1;
		     innerState.dataSt = DataMode;
			 sGSMState.flagTransaction = 1;
			 //USBWriteStr("\r\n");
		   }
       }
     }
   }
}
//-------------------------------------------------------------------------------------------
void TbFunRawData(void)
{
  //if(!flagNoGSM)
  if( (!sGSMState.flagAlrSent)  || (sGSMState.flagTimeout) )
   {

    if(sGSMState.flagPrepareToSend)
     { 
      sGSMState.flagPrepareToSend = 0;
      if(!checkTimeoutFun())
	    {
         cmdSendLen(bufTxGSM, lenData);
		}
	  else
	    {
	     sGSMState.flagLostPacket = 1;
		}

	  timerFDTimeout = TIMEOUT_WAIT_SEND_OK;
	  sGSMState.flagAlrSent = 1;
      sGSMState.flagTimeToSend = 1;
      innerState.dataSt = DataMode;
     }
   }
}
//-------------------------------------------------------------------------------------------
void TbFunCIPSHUT(void)
{
    cmdPrepareSend((u8 *)QUERIES[13], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCGATT0(void)
{
	  cmdPrepareSend((u8 *)QUERIES[9], TIMEOUT_END_GPRS_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCIPCLOSE(void)
{
    cmdPrepareSend((u8 *)QUERIES[27], TIMEOUT_CIPCLOSE);
}  
//-------------------------------------------------------------------------------------------
void TbFunATH(void)
{
    cmdPrepareSend((u8 *)QUERIES[28], TIMEOUT_USUAL_CMD);
}  
//-------------------------------------------------------------------------------------------
void TbFunATA(void)
{
    cmdPrepareSend((u8 *)QUERIES[30], TIMEOUT_USUAL_CMD);
}  
//-------------------------------------------------------------------------------------------
void TbFunCUSD(void)
{
      //cmdPrepareSend((u8 *)QUERIES[33], TIMEOUT_USUAL_CMD);
	  //USBWriteStr("FUN CUSD called\r\n");
	  //setCUSDQuery(innerState.activeSIMCard);
      //cmdPrepareSend((u8 *)cnfBuf, TIMEOUT_USUAL_CMD);
  if( (!sGSMState.flagAlrSent)  || (sGSMState.flagTimeout) )
     {
	    if(sGSMState.flagPrepareToSend)
		 { 
		  sGSMState.flagPrepareToSend = 0;
		  //USBWriteStr("FUN CUSD called\r\n");
	      setCUSDQuery(innerState.activeSIMCard);
          /*timeout handling*/
		  timerFDTimeout = TIMEOUT_USUAL_CMD; 
		  /*ready to send*/
		  sGSMState.flagTimeToSend = 1;
		  if(!checkTimeoutFun())
		    {
	         cmdSend((u8 *)cnfBuf);
			}
		  /*aux actions*/
		  sGSMState.flagAlrSent = 1;
		 }
     }

}
//-------------------------------------------------------------------------------------------
void TbFunCPHNUM(void)
{
	  setCUSDPhNumQuery(innerState.activeSIMCard);
      cmdPrepareSend((u8 *)cnfBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCIMI(void)
{
      cmdPrepareSend((u8 *)QUERIES[37], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCREG(void)
{
      cmdPrepareSend((u8 *)QUERIES[1], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCREGASK(void)
{
      cmdPrepareSend((u8 *)QUERIES[38], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunCREGZERO(void)
{
      cmdPrepareSend((u8 *)QUERIES[39], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunEmpty(void)
{
  
}
//-------------------------------------------------------------------------------------------
void TbFunNothing(void)
{
  
}
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
void loopGSM(void)
{
  u16 pos, len, k;
  u8  cap, stateTemp;
  if(sGSMState.flagStartLoopGSM )
   {
     setTimerLoopQuery(MS_10);
	 sGSMState.flagStartLoopGSM = 0;

	 if(!flagGoFtpControl)
	   {
	      if(sGSMState.flagIniFromFTP)	  /*When returning from FTP*/
		     {
			    sGSMState.flagIniFromFTP = 0;
			    gprsModemOff();
				if(innerState.flagDebug)
					USBWriteStr("FromFTP\r\n");
			    gprsModemOn(innerState.activeSIMCard);
				iniGSM();
			 }
		  stateTable[currState]();
		#if 1
		  if(numTryTimeoutResendAT >= RESEND_TIMEOUT_RETRY)
		     {
			   if(innerState.flagDebug)
			   		USBWriteStr("RES_T_R\r\n");
				
			   gprsModemOn(innerState.activeSIMCard);
			   iniGSM();
			   
			   numTryTimeoutResendAT = 0;
			   //currState = stCheckConn;
			   //setTimerSendAux(TIME_FIRST_INI);
			   //sGSMState.flagTimeToSend = 0;
			   //sGSMState.flagGsmStart = 1;
			 }
		#endif
		#if 0		//temporarily
		  if(sGSMState.flagLostPacket)
		    {
			  sGSMState.flagLostPacket = 0;

			  //if(flagNaviValid)  
			    //{
			     //saveSDInfo(bufTxGSM, lenData, SD_NOTSEND, SD_TYPE_MSG_GSM);  //ini
				 pos = 0;
				 k = GPS_SENDING_BUFFER_SIZE;
				 while(lenData)
				    {
					   if( lenData > GPS_SENDING_BUFFER_SIZE )
					     {
						   while( k > pos )
					        {
							   k--;
							   if( ( bufTxGSM[k] == '$' ) && (bufTxGSM[k-1] != 0xfe ) )  /* found border of packet*/
								   {
								      len = k - pos + 1;
									  break;
								   }
						    }
							lenData -= len;
						 }
					   else
					     {
						  len = lenData;
					      lenData = 0;
						 }

					   saveSDInfo(&bufTxGSM[pos], len, SD_NOTSEND, SD_TYPE_MSG_GSM);
					   pos = k+1;
					   k += GPS_SENDING_BUFFER_SIZE;
				 	}
				//}  //if(flagNaviValid)

		    }
		  #endif
		  if(sGSMState.flagTimeToSend)
		     {
			   sGSMState.flagTimeToSend = 0;
		       setTimerTimeout(timerFDTimeout);
		       checkForSend();
			 }
		  handleRcvGSM();
		  /* HERE we can try get our location by GSM*/
		  if(!getGLONASSStateValid())		   /* No GPS - then we start timer 1 min*/
		    {
			  setTimerNAVIPres(0); 
			  sGSMState.flagNaviPresTimerStarted = 0;
			  if(!sGSMState.flagNaviAbsTimerStarted)
			    {
			     setTimerNAVIAbs(ONE_SEC * 60);
				 sGSMState.flagNaviAbsTimerStarted = 1; 
				}
			}
		  else								  /* Got GPS - we reset timer*/
		    {
			  setTimerNAVIAbs(0); 
			  sGSMState.flagNaviAbsTimerStarted = 0;
			  innerState.flGotGSMLocation = 0;
			  if(!sGSMState.flagNaviPresTimerStarted)
			    {
			     setTimerNAVIPres(ONE_SEC * 30);
				 sGSMState.flagNaviPresTimerStarted = 1; 
				}
			}

		  if(sGSMState.flagNaviAbs)
		    {
			   sGSMState.flagNaviAbs = 0;
			   callGSMFunction(4);
		    }
		  if(sGSMState.flagNaviPres)
		    {
			   sGSMState.flagNaviPres = 0;
			   callGSMFunction(5);
		    }
		/* Protection if there was not one sending then we count timer(15min for example) and restart modem*/
          if(innerState.flagProtNoSend)
		    {
		      gprsModemOn(innerState.activeSIMCard);
		      iniGSM();
			}
		 /* Get AT command from buffer */
		  if( (!sGSMState.flagAlrSent) && (!sGSMState.flagTransaction) && (!sGSMState.flagCmdFromBuffer))
		    {
			  cap = fifoPacketCount(&fifoGSMATCmdPack);
		      if(cap)
			    {
				  fifoPacketGet(&fifoGSMATCmdPack, (u8 *)cnfBuf , &stateTemp);
    			  savedStateCNF = currState;
					if(innerState.flagDebug)
					 {
					   //USBWriteStr("GSMFB:");
				       //USBDebWrite(savedStateCNF);
					   //USBWriteStr("\r\n");
					 }
				  sGSMState.flagCmdFromBuffer = 1;
				  switch(stateTemp)
				   {
					  case 0 : currState = stCUSD;	   break;
					  case 1 : currState = stCMIC;	   break;
					  case 2 : currState = stCLVL;	   break;
					  case 3 : currState = stCPHNUM;   break;
					  case 4 : currState = stCREG;     break;
					  case 5 : currState = stCREGZERO; break;
					  case 6 : currState = stATA;      break;
					  case 7 : currState = stATH;      break;
					  default: sGSMState.flagCmdFromBuffer = 0; break;
				   }
					if(innerState.flagDebug)
					 {
					  //USBWriteStr("GSMFA:");
				      //USBDebWrite(currState);
					  //USBWriteStr("\r\n");
				     }
				  //cmdPrepareSend((u8 *)cnfBuf, TIMEOUT_USUAL_CMD);
			    }
		    }

	    }
	}
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void handleRcvGSM(void)
{
  //u32  timeIntrvRawData;
  static u8 cntPINError = 0;
  if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],REPL_OK_LEN)))  /*OK*/
	    { 
	      
		  switch(currState)
		   {
			 case  stCheckConn  :  currState = stATE0;
			 					   if(sGSMState.flagOnceSended)
		    							currState = stCGATT_ASK;
						           if(!sGSMState.flagNewCycle)
								     {
								       setTimerSendAux(TIME_INTRV_CMDS);
									 }
								   else
								     {
								       setTimerSendAux(TIMEOUT_1MIN);
								      }
										 break;
			 case  stATE0       :  setTimerSendAux(TIME_INTRV_CMDS);currState = stIPR; break;
			 case  stIPR        :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCPINASK; break;
			 case  stCPINASK	:  setTimerSendAux(TIME_INTRV_CMDS);
			                       if(sGSMState.flagCPIN_ASK)
										{
										  sGSMState.flagCPIN_ASK = 0;
										  currState = stCSPN;
#if defined (VER_3)
										  setDutyCycle(30);
#endif
										}
								   break;
			 case  stCSPN       :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCMGF; break;
			 case  stCMGF       :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCMIC; break;
			 case  stCMIC       :  setTimerSendAux(TIME_INTRV_CMDS);
								   //currState = stCLVL;
								   currState = (savedStateCNF == stNothing) ? stCLVL : savedStateCNF;
								   savedStateCNF = stNothing;
								   sGSMState.flagCmdFromBuffer = 0;
								   break;
			 case  stCLVL       :  setTimerSendAux(TIME_INTRV_CMDS);
			 	                   //currState = stCSCS;
								   currState = (savedStateCNF == stNothing) ? stCSCS : savedStateCNF;
								   savedStateCNF = stNothing;
								   sGSMState.flagCmdFromBuffer = 0;
								   break;
			 case  stCSCS       :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCIMI; break;
			 case  stCIMI       :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCREGZERO; break;
			 case  stCREGZERO   :  setTimerSendAux(TIME_INTRV_CMDS);
			                       //currState = stCLIP;
			 					   currState = (savedStateCNF == stNothing) ? stCLIP : savedStateCNF;
								   savedStateCNF = stNothing;
								   sGSMState.flagCmdFromBuffer = 0;
								   break;
			 case  stCLIP       :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCSCLK; break;
			 case  stCSCLK      :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCGATT_ASK; break;
			 case  stCGATT_ASK  :  setTimerSendAux(TIME_INTRV_CMDS);
                        		   if(sGSMState.flagCGATT_ASK_OK) 
									 {
									   sGSMState.flagCGATT_ASK_OK = 0;
									   currState = stCIPATS;
#if defined (VER_3)									   
									   setDutyCycle(50);
#endif
									 }
									else
									 {
									   currState = stCGATT1;  //next, cgatt = 1
									 }
			                        break;
			 case  stCGATT1     :  setTimerSendAux(TIME_INTRV_CMDS);
#if defined (VER_3)			 
			 					   setDutyCycle(50);
#endif								   
			 					   currState = stCIPATS; break;
			 case  stCIPATS     :  setTimerSendAux(TIME_INTRV_CMDS);
			                       currState = stCIPCSGP;
								   if(sGSMState.flagOnceSended)
								      currState = stIFC;
								    break;
			 case  stCIPCSGP    :  setTimerSendAux(TIME_INTRV_CMDS);currState = stIFC; break;
			 case  stIFC        :  setTimerSendAux(TIME_INTRV_CMDS);
			 					   currState = stEnTranspMode;
								   if(sGSMState.flagOnceSended)
								       currState = stCSTT;
								    break;
			 case  stEnTranspMode :setTimerSendAux(TIME_INTRV_CMDS);currState = stCSTT;sGSMState.flagOnceSended = 1; break;
			 case  stCSTT       :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCIICR; break;
			 case  stCIICR      :  setTimerSendAux(TIME_INTRV_START_TCP);currState = stCIFSR; break;
			 case  stCIFSR      :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCIPHEAD; break; 
			 case  stCIPHEAD    :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCIPSTART; break;
			 case  stCGATT0     :  setTimerSendAux(TIME_INTRV_CMDS);
#if defined (VER_3)			 
			 					   setDutyCycle(30);
#endif								   
			 					   currState = stCIPSHUT; flagNoGSM = 1;break;
			 case  stATH        :  setTimerSendAux(TIME_INTRV_CMDS); 
			 					   currState = savedStateCNF;
								   savedStateCNF = stNothing;
								   sGSMState.flagCmdFromBuffer = 0;
			                       //currState = savedState;
			     				   //innerState.dataSt = ReIni;
								   innerState.dataSt = innerState.savedDataSt;
			  																				break;
			 case  stATA        :  //if(innerState.dataSt != VoiceMode)
			 						 //{
				 					   setTimerSendAux(TIME_INTRV_CMDS);
				 					   currState = stEmpty;	//maybe should change to stNothing
			 					       innerState.dataSt = VoiceMode;
									   GSMSpeaker(1);
									   if(innerState.flagDebug)
									      USBWriteStr("Trace stATA\r\n");
								     //}
			  																				 break;
			 case  stCUSD       :  setTimerSendAux(TIME_INTRV_CMDS);
			 					   currState = savedStateCNF;
								   savedStateCNF = stNothing;
									if(innerState.flagDebug)
									 {
									  USBWriteStr("stCUSD recv:");
								      USBDebWrite(currState);
									  USBWriteStr("\r\n");
								     }

		 					       sGSMState.flagWaitBalance = 1;
								   sGSMState.flagCmdFromBuffer = 0;
			  																				 break;
			                        
			 case  stCPHNUM     :  setTimerSendAux(TIME_INTRV_CMDS);
			 					   currState = savedStateCNF;
								   savedStateCNF = stNothing;
		 					       sGSMState.flagWaitPhNum = 1;
								   sGSMState.flagCmdFromBuffer = 0;
			  																				 break;
			 case  stCREG       :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCREGASK;       break;
			 case  stCREGASK    :  setTimerSendAux(TIME_INTRV_CMDS);
			                       currState = savedStateCNF;
								   savedStateCNF = stNothing; 
			                       sGSMState.flagCmdFromBuffer = 0;  
			                       break;

			 default            : break;
		   }
		}
	  else if(tempBuf[0] == '>')	 // '>'
	    { 
		  if(currState == stCIPSEND)
		    {
		      currState = stRawData;  //next
	          setTimerSendAux(TIME_INTRV_CMDS);
		    }
		}
	  else if((!strncmp((char *)REPLIES[1],(char *)&tempBuf[0],REPL_ERROR_LEN)))  /*ERROR*/
	    { 
          switch(currState)
		   {
			 
			 case  stCPINASK     :  setTimerSendAux(TIME_INTRV_CMDS);
									if(++cntPINError == 3)
									   {
									     cntPINError = 0;
										 gprsChangeSIM();
										 iniGSM(); 
									   }
			                        break;
			 case  stCIPCSGP     :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCIPSHUT; break;
			 case  stCSTT        :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCIPSHUT; break;
			 case  stCIICR       :  setTimerSendAux(TIME_INTRV_START_TCP);currState = stCIPSHUT; break;
			 case  stCIFSR       :  setTimerSendAux(TIME_INTRV_START_TCP);currState = stCIPSHUT; break;
			 case  stCIPSTART    :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCIPSHUT; sGSMState.flagCIPSTARTError = 1; break;
			 case  stCIPCLOSE    :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCGATT0; break;
			 case  stCIPSEND     :  setTimerSendAux(TIME_INTRV_CMDS);currState = stCGATT0;sGSMState.flagLostPacket = 1; break;
		     case  stCGATT_ASK   :  setTimerSendAux(TIME_INTRV_CMDS);
									gprsChangeSIM();
									iniGSM();			 
			  						break;                 
			 default             : break;
            }			 
		}
	  else if(!strncmp((char *)REPLIES[12],(char *)&tempBuf[0],REPL_CPIN_LEN) )  /*+CPIN: READY*/
	    {
		  if(currState == stCPINASK)
		   {
		    sGSMState.flagCPIN_ASK = 1;
			cntPINError = 0;
		   }
	    }
	  else if(currState == stCIMI)
	    {
		    /* Get MCC and MNC*/
		   	memset(cnfBuf,0,sizeof(cnfBuf));
	        strncpy((char *)cnfBuf,(const char *)&tempBuf[0],3);
			binInfo.gsm_loc.mcc = atoi((const char *)cnfBuf);
		   	memset(cnfBuf,0,sizeof(cnfBuf));
	        strncpy((char *)cnfBuf,(const char *)&tempBuf[3],2);
			binInfo.gsm_loc.mnc = atoi((const char *)cnfBuf);
	    }
	  else if(!strncmp((char *)REPLIES[21],(char *)&tempBuf[0],REPL_CPIN_NOT_READY_LEN) )  /*+CPIN: NOT READY*/
	    {
		   gprsChangeSIM();
		   iniGSM();
	    }
	  else if(!strncmp((char *)REPLIES[7],(char *)&tempBuf[0],REPL_CGATT_LEN))        /*+CGATT: */
	   {
	     if(currState == stCGATT_ASK)
		   {
		      if(tempBuf[REPL_CGATT_LEN] == 0x31)   // 0x31 - ascii 1
		         sGSMState.flagCGATT_ASK_OK = 1;
			  else if(tempBuf[REPL_CGATT_LEN] == 0x30)   // 0x30 - ascii 0
		         sGSMState.flagCGATT_ASK_OK = 0;
		   }
	   }
	  else if(!strncmp((char *)REPLIES[17],(char *)&tempBuf[0],REPL_CSPN_LEN))        /* +CSPN: */
	   {
	     if(currState == stCSPN)
		   {
		      memset(cspnBuf,0,sizeof(cspnBuf));
			    /* +1 - because we missed a forward " */
              strncpy(cspnBuf,(char *)&tempBuf[REPL_CSPN_LEN+1], (strchr((char *)&tempBuf[REPL_CSPN_LEN+1],'\"') - (char *)&tempBuf[REPL_CSPN_LEN+1]) );   
			  innerState.flCellNameChanged = 1;
		   }
	   }
     else if((!strncmp((char *)REPLIES[2],(char *)&tempBuf[0],REPL_CONNN_OK_LEN)))  /*CONNECT OK */
	    { 
		  if(currState == stCIPSTART)
		   {
		      setTimerSendAux(TIME_INTRV_CMDS);
			  currState = stCIPSEND; //next
			  flagNoGSM = 0;  //make this flag zero we say there is connection(GSM)
			  setTimerGSMPresence(TIMEOUT_GSM_PRESENCE);
		      innerState.dataSt = Connected;
			  sayHello();
		   }
		}
      else if((!strncmp((char *)REPLIES[14],(char *)&tempBuf[0],REPL_ALR_CONN_LEN)))  /*ALREADY CONNECT*/
	    { 
		  if(currState == stCIPSTART)
		    {
			 sGSMState.flagAlrSent = 0;		   
    		 if(sGSMState.flagCIPSTARTError)
				{
				  sGSMState.flagCIPSTARTError = 0;
				  currState = stCIPSHUT; //next
				  setTimerSendAux(TIME_INTRV_START_TCP);
				}
			}
		}
      else if((!strncmp((char *)REPLIES[16],(char *)&tempBuf[0],REPL_CLOSE_OK_LEN)))  /*CLOSE OK*/
	    { 
		  if(currState == stCIPCLOSE)
		    {
			  setTimerSendAux(TIME_INTRV_CMDS);
			  currState = stCGATT0;
              flagNoGSM = 1;
			}
		}
      else if((!strncmp((char *)REPLIES[18],(char *)&tempBuf[0],REPL_TCP_CLOSED_LEN)))  /*STATE: TCP CLOSED*/
	    { 
		  if(currState == stCIPSTART)
		    {
			  sGSMState.fTCPClosed = 1;
			}
		}
      else if((!strncmp((char *)REPLIES[19],(char *)&tempBuf[0],REPL_CON_FAIL_LEN)))  /*CONNECT FAIL */
	    { 
		  if(currState == stCIPSTART)
		    {
			  if(sGSMState.fTCPClosed)
			     {
				   sGSMState.fTCPClosed = 0;
				   changeGPRSServer();
				   setTimerSendAux(TIME_INTRV_CHANGE_SERVER);
		      	   //setTimerSendAux(TIME_INTRV_CMDS);
			       currState = stCIPSTART;
				 }
			}
		}
      else if((!strncmp((char *)REPLIES[9],(char *)&tempBuf[0],REPL_SHUT_OK_LEN)))  /*SHUT OK*/
	    { 
		  if(currState == stCIPSHUT)
		   {
		      setTimerSendAux(TIME_INTRV_CMDS);
			  currState = stCheckConn;  //?
			  flagNoGSM = 1;
			  if(flagGoFTPLoad)
				 {
				   flagGoFTPLoad = 0;
				   flagGoFtpControl = 1;
				   sGSMState.flagIniFromFTP = 1;
				 }
		   }
		   
		}													
      else if((!strncmp((char *)REPLIES[10],(char *)&tempBuf[0],REPL_SEND_OK_LEN)))  /*SEND OK*/
	    { 
		  if(currState == stRawData)
		    {
			  currState = stCIPSEND;  
			  /*Dynamically change timer interval depends on packet's capacity*/
			  /*
			  packetCap = fifoPacketCount(&fifoNaviPack);
			  if(packetCap < (0.1 * FIFO_PACKET_NAVI_SIZE))
			     {
			       timeIntrvRawData = (config.intervalSendingPower * ONE_SEC) / 2;
				   //USBWriteChar('S');
			     }
			  else 
			     {
			       timeIntrvRawData = ONE_SEC / 2;	 //check it?
				   //USBWriteChar('L');
				 }

	          setTimerSendAux(timeIntrvRawData);
			  */
	          setTimerSendAux(TIME_INTRV_CMDS);
			  flagNoGSM = 0;  //make this flag zero we say there is connection(GSM)
			  setTimerGSMPresence(TIMEOUT_GSM_PRESENCE);
		    }
		  sGSMState.flagTransaction = 0;
		}													
      else if((!strncmp((char *)REPLIES[8],(char *)&tempBuf[0],REPL_CLOSED_LEN)))  /*CLOSED*/
	    { 
		  if(currState == stRawData)
		    {
		      setTimerSendAux(TIME_INTRV_CMDS);
			  currState = stCGATT0;  
			  sGSMState.flagLostPacket = 1;
	          flagNoGSM = 1;
		    }
		  else if(currState == stCIPSEND)
		    {
		      setTimerSendAux(TIME_INTRV_CMDS);
			  currState = stCGATT0;  
			  sGSMState.flagLostPacket = 1;
		    }
		}
      else if((!strncmp((char *)REPLIES[23],(char *)&tempBuf[0],REPL_CREGASK_LEN)))  /*+CREG: 2,1, catched*/
	    { 
			binInfo.gsm_loc.lac = strhex2int((u8 *)&tempBuf[REPL_CREGASK_LEN + 1]);   //+1 - because we miss "
			binInfo.gsm_loc.lac |= (strhex2int((u8 *)&tempBuf[REPL_CREGASK_LEN + 3]) << 8);   //+2 - because we miss XX
			binInfo.gsm_loc.cid = strhex2int((u8 *)&tempBuf[REPL_CREGASK_LEN + 8]);   //+1 - because we miss "XXXX","
			binInfo.gsm_loc.cid |= (strhex2int((u8 *)&tempBuf[REPL_CREGASK_LEN + 10]) << 8);   //+7 - because we miss XX
            innerState.flGotGSMLocation = 1;
		}	
      else if((!strncmp((char *)REPLIES[24],(char *)&tempBuf[0],REPL_CREGURC_LEN)))  /*+CREG: 1, catched*/
	    { 
			binInfo.gsm_loc.lac = strhex2int((u8 *)&tempBuf[REPL_CREGURC_LEN + 1]);   //+1 - because we miss "
			binInfo.gsm_loc.lac |= (strhex2int((u8 *)&tempBuf[REPL_CREGURC_LEN + 3]) << 8);   //+2 - because we miss XX
			binInfo.gsm_loc.cid = strhex2int((u8 *)&tempBuf[REPL_CREGURC_LEN + 8]);   //+1 - because we miss "XXXX","
			binInfo.gsm_loc.cid |= (strhex2int((u8 *)&tempBuf[REPL_CREGURC_LEN + 10]) << 8);   //+7 - because we miss XX
            innerState.flGotGSMLocation = 1;
		}	
      else if((!strncmp((char *)REPLIES[22],(char *)&tempBuf[0],REPL_CUSD_LEN)))  /*"+CUSD: " catched, Handling CUSD query*/
	    { 
		  if(sGSMState.flagWaitBalance)
		     sGSMState.flagGotBalance = 1;
		  else if(sGSMState.flagWaitPhNum)
		     sGSMState.flagGotPhNum = 1;

		  memcpy(cusdBuf,(char *)&tempBuf[7],MIN(sizeof(cusdBuf),(strlen((char *)tempBuf))));  // 7 - because we miss +CUSD: 
		  //if(innerState.flagDebug)
		   //  USBWriteStr("CUSD data rcv \r\n");

		}													
      else if((!strncmp((char *)REPLIES[4],(char *)&tempBuf[0],REPL_NO_CARRIER_LEN)))  /*"NO CARRIER " catched*/
	    { 
		  GSMSpeaker(0);
		  //currState = savedState;
          currState = savedStateCNF;
		  savedStateCNF = stNothing;
		  sGSMState.flagCmdFromBuffer = 0;
	      innerState.dataSt = innerState.savedDataSt;
	      if(innerState.flagDebug)
		    {
		      USBWriteStr("TRACE NO CARR:");
		      USBDebWrite(currState);
			  USBWriteStr("\r\n");
			}
		}	
      else if((!strncmp((char *)REPLIES[20],(char *)&tempBuf[0],REPL_PDP_DEACT_LEN)))  /*+PDP: DEACT*/
	    { 
		      setTimerSendAux(TIME_INTRV_CMDS);
			  currState = stCIPSHUT;  
			  flagNoGSM = 1;
		}													
	   else	if(currState == stCIFSR)   /* catch local ip-adres*/
	    {
		  setTimerSendAux(TIME_INTRV_CMDS);
		  currState = stCIPHEAD;
	    }
       else 
	     checkCalling();
   /*
       
 	    
	*/
	   
	   
    } 
   
}

// ------------------------------------------------------------------------------
// *************************************TIMERS**********************************
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
void cmdPrepareSend(u8 * buf, u32 timerVal)
{
  if( (!sGSMState.flagAlrSent)  || (sGSMState.flagTimeout) )
     {
	    if(sGSMState.flagPrepareToSend)
		 { 
		  sGSMState.flagPrepareToSend = 0;
		  /*timeout handling*/
		  timerFDTimeout = timerVal; //TIMEOUT_USUAL_CMD;
		  /*ready to send*/
		  sGSMState.flagTimeToSend = 1;
		  if(!checkTimeoutFun())
		    {
	         cmdSend(buf);
			}
		  /*aux actions*/
		  sGSMState.flagAlrSent = 1;
		 }
     }
}
// ------------------------------------------------------------------------------
// return 1 - if timeout happened > 3 times, 0 - no timeout
// ------------------------------------------------------------------------------
u8 checkTimeoutFun(void)
{
     u8 result = 0;
     if(sGSMState.flagTimeout)
	  {
	    numTryTimeoutResendCmd++;
		if(numTryTimeoutResendCmd > (RESEND_TIMEOUT_RETRY - 1))
		   {
		       numTryTimeoutResendCmd = 0;
		       currState = stCheckConn;
			   //flagToSend = 0;
			   sGSMState.flagNewCycle = 1;
			   result = 1;
		   }
	    //else
		//flagToSend = 1;
		//sGSMState.flagTimeToSend = 1;   // or may be flagPrepareToSend=1  ?
		sGSMState.flagPrepareToSend = 1;
	    sGSMState.flagTimeout = 0;
		//USBWriteStr("Timeout\r\n");
	  }
	 else
	  {
		numTryTimeoutResendCmd = 0;
	  }
	 return result;
}

// ------------------------------------------------------------------------------
void handleTimerGSMPresence(void)
{
    if(timerValueGSMPresence)
	  {
	   timerValueGSMPresence--;
	   if(!timerValueGSMPresence )
	      flagNoGSM = 1;
	  }
	if(timerValueLoopQuery)
	  {
	   timerValueLoopQuery--;
	   if(!timerValueLoopQuery )
	      sGSMState.flagStartLoopGSM = 1;
	  }
	if(timerValueSend)
	  {
	   timerValueSend--;
	   if(!timerValueSend )
	      sGSMState.flagPrepareToSend = 1;
	  }
	if(timerValueTimeout)
	  {
	   timerValueTimeout--;
	   if(!timerValueTimeout )
	    {
	      sGSMState.flagTimeout = 1;
          sGSMState.flagPrepareToSend = 1;
		}
	  }
	if(timerValueBalance)
	  {
	   timerValueBalance--;
	   if(!timerValueBalance )
		  sGSMState.resetBalance = 1;	      
	  }
	if(timerValuePhNum)
	  {
	   timerValuePhNum--;
	   if(!timerValuePhNum )
		  sGSMState.resetPhNum = 1;	      
	  }
	if(timerValueNaviAbs)
	  {
	   timerValueNaviAbs--;
	   if(!timerValueNaviAbs )
		  sGSMState.flagNaviAbs = 1;	      
	  }
	if(timerValueNaviPres)
	  {
	   timerValueNaviPres--;
	   if(!timerValueNaviPres )
		  sGSMState.flagNaviPres = 1;	      
	  }

}
// ------------------------------------------------------------------------------
void setTimerGSMPresence(u32 val)
{
   	timerValueGSMPresence = val;
}
// ------------------------------------------------------------------------------
void setTimerLoopQuery(u32 val)
{
   	timerValueLoopQuery = val;
}
// ------------------------------------------------------------------------------
void setTimerSendAux(u32 val)
{
    sGSMState.flagPrepareToSend = 0;
	sGSMState.flagAlrSent = 0;
   	timerValueSend = val;
    setTimerTimeout(0);
}
// ------------------------------------------------------------------------------
void setTimerTimeout(u32 val)
{
   	timerValueTimeout = val;
}
// ------------------------------------------------------------------------------
void setTimerBalance(u32 val)
{
   	timerValueBalance = val;
}
// ------------------------------------------------------------------------------
void setTimerPhNum(u32 val)
{
   	timerValuePhNum = val;
}
// ------------------------------------------------------------------------------
void setTimerNAVIAbs(u32 val)
{
   	timerValueNaviAbs = val;
}
// ------------------------------------------------------------------------------
void setTimerNAVIPres(u32 val)
{
   	timerValueNaviPres = val;
}

// ------------------------------------------------------------------------------
//  *****************************PHONES AND SMS *********************************
// ------------------------------------------------------------------------------
void checkCalling(void)
{
      static u8 flagPhNumAllowed = 0;
	  if((!strncmp((char *)REPLIES[13],(char *)&tempBuf[0],REPL_CLIP_LEN))) /*calling Handling	 "+CLIP: "*/
	   {   
	     setTimerSendAux(TIME_INTRV_CMDS);
		 //savedState = currState;
	     //currState = stATH;		  /* Cancel the ring */
	     if(checkPhones(&tempBuf[REPL_CLIP_LEN+1]))	 //+1 beacause we have to miss \" symbol
		   {
			 //currState = stATA;			 /* Picked up the phone and talk*/
			 callGSMFunction(6);			/* Picked up the phone and talk*/
		   }
         else
		   callGSMFunction(7);		  /* Cancel the ring */

		 innerState.savedDataSt = innerState.dataSt;  
	   }
	  else if((!strncmp((char *)REPLIES[15],(char *)&tempBuf[0],REPL_CMT_LEN))) /*SMS Handling "+CMT: " */
	    {   
		  if(checkPhones(&tempBuf[REPL_CMT_LEN+1]))  //+1 beacause we have to miss \" symbol
		   {
			flagPhNumAllowed = 1;
		   }
		  else
		    flagPhNumAllowed = 0;
	    }
	  //else if((!strncmp((char *)CRYPTO_PHRASE[0],(char *)&tempBuf[0],REPL_UPGRADE_LEN))) //'upgrade'
	  else if((!strncasecmp((char *)CRYPTO_PHRASE[0],(char *)&tempBuf[0],REPL_UPGRADE_LEN))) //'upgrade'
	  	{
		   if(flagPhNumAllowed)
		    {
			  parseSMS(); 
			  flagGoFTPLoad = 1;
			  currState = stCIPCLOSE;
			  flagPhNumAllowed = 0;
		    }
		}

}
// ------------------------------------------------------------------------------
void parseSMS(void)
{
 //todo - make check on max size len (Un,pw)
   //u8 pos = REPL_UPGRADE_LEN + 1,len, lenMsg;
   u8 * bufPtr;
   //char * msg = "upgrade 06.37.151.091 ship   gora";
   u8 fIP = 0,fUn = 0,fPw = 0, cntD = 0, cntP = 0, cnt = 0, lenMsg;
   //bufPtr = msg + REPL_UPGRADE_LEN;
   bufPtr = tempBuf + REPL_UPGRADE_LEN;
   lenMsg = strlen((char *)tempBuf);
   //lenMsg = strlen( msg );
   //strncpy((char *)tempBuf,msg, lenMsg);
   
   if( (tempBuf[REPL_UPGRADE_LEN] == ' ') && (lenMsg > (REPL_UPGRADE_LEN + 8)) )  //8 - because minimum 3 points + 4digits min + 1space
     {
	    while(*bufPtr++) 
		 {
		   if(!fIP)
		    {
		     if( ( *bufPtr >= '0') && (*bufPtr <= '9' ) )
			   {
			    cntD++;
				cnt++;
			   }
			 else if(*bufPtr == '.')
			   {
			    cntD = 0;
				cntP++;
				cnt++;
			   }
			 else if( (cntP == 3) && (( *bufPtr == ' ' ) || (*bufPtr == '\0') ) )
			   {
			     fIP = 1;
				 memset((char *)config.ftpServer,0,strlen((char *)config.ftpServer));
				 strncpy((char *)config.ftpServer,(char *)bufPtr - cnt, cnt);
				 cnt = 0;
			   }
			 else if( (cntD) || (cntP))  /* if not a digit and not a point inside IP-adres*/
			   break;
			 else if( *bufPtr != ' ' )	/* if not a digit and not a point in beginning of IP-adres*/
			   break;
			 if(cntD > 3)
			    break;
		    }
		  else if(!fUn)
		    {
			   if( 
			       ( *bufPtr >= '0') && (*bufPtr <= '9' )  
					                 ||
				   ( *bufPtr >= 'A') && (*bufPtr <= 'Z' )
				                     ||
				   ( *bufPtr >= 'a') && (*bufPtr <= 'z' )
				 )
			      cnt++;
			   else if( (cnt >= 3) && (( *bufPtr == ' ' ) || (*bufPtr == '\0')) )
			      {
				    fUn = 1;
					memset((char *)config.ftpUsername,0,strlen((char *)config.ftpUsername));
					strncpy((char *)config.ftpUsername,(char *)bufPtr - cnt, cnt);
					cnt = 0;
				  }
			   else if(cnt)
			      break;
		    }
		  else if(!fPw)
		    {
			   if( 
			       ( *bufPtr >= '0') && (*bufPtr <= '9' )  
					                 ||
				   ( *bufPtr >= 'A') && (*bufPtr <= 'Z' )
				                     ||
				   ( *bufPtr >= 'a') && (*bufPtr <= 'z' )
				 )
			      cnt++;
			   else if( (cnt >= 3) && (( *bufPtr == ' ' ) || (*bufPtr == '\0')) )
			      {
				    fPw = 1;
					memset((char *)config.ftpPassword,0,strlen((char *)config.ftpPassword));
					strncpy((char *)config.ftpPassword,(char *)bufPtr - cnt, cnt);
					cnt = 0;
				  }
			   else if(cnt)
			      break;
			   
		    }
		 } 
     }
}
// ------------------------------------------------------------------------------
u8 checkPhones(u8 * buf)
{
   u8 i, k, res = 1, t;
   //strncpy(config.contrNumber[1], "upgrade",strlen("upgrade") ); //deb
   //strncpy(tempBuf, "Upgrade",strlen("upgrade") ); //deb
   i = sizeof(config.contrNumber) / sizeof(config.contrNumber[0]);
   for(k = 0; k < i;k++)
      if(strlen((const char *)config.contrNumber[k]))
	     {
		   res = 0;
		   break;
		 }

   if(!res)
	   for(k = 0; k < i;k++)
	     {
		    t = strlen((const char *)config.contrNumber[k]);
			if(t)
  	          if(!strncmp((const char *)buf ,(const char *)config.contrNumber[k], t))
			     {
				  res = 1;
				  break;
			     } 
		 }
   return res;
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u16 getBalance(u8 * buf)
{
  u16 len = 0;
  #if 1			  //debugga
  static u8 state = 0;
  /* 1 - query, 2 - waiting, 3 - got balance*/

  if(sGSMState.resetBalance)	//by timer-not to often ask balance
   {
      sGSMState.resetBalance = 0;
	  sGSMState.flagWaitBalance = 0;
	  sGSMState.flagGotBalance = 0;
      state = 0;
   }

  if(state == 1)
    {
		if(sGSMState.flagWaitBalance)
		   {
		     state++; //became 2
		   }
    }
  else if(state == 0)  
   {
	 callGSMFunction(0);
     state++;	   //became 1
	 setTimerBalance(ONE_SEC * 60);
   }

  if(sGSMState.flagGotBalance)
     state = 3;

  *buf++ = state;

  if(sGSMState.flagGotBalance)
    {
	  len = strlen(cusdBuf);
      memcpy(buf,cusdBuf,len);
	  sGSMState.flagWaitBalance = 0;
    }
  len++;
  #endif
  
  //flagGoFTPLoad = 1;
  //currState = stCIPCLOSE;
  
  return len;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u16 getPhoneNumber(u8 * buf)
{
  u16 len = 0;
  #if 1			  //debugga
  static u8 state = 0;
  /* 1 - query, 2 - waiting, 3 - got phone number*/
  if(sGSMState.resetPhNum)	//by timer-not to often ask ph num
   {
      sGSMState.resetPhNum = 0;
	  sGSMState.flagWaitPhNum = 0;
	  sGSMState.flagGotPhNum = 0;
      state = 0;
   }
  if(state == 1)
    {
		if(sGSMState.flagWaitPhNum)
		   {
		     state++; //became 2
		   }
    }
  else if(state == 0)  
   {
	 callGSMFunction(3);
     state++;	   //became 1
	 setTimerPhNum(ONE_SEC * 60);
   }
  if(sGSMState.flagGotPhNum)
     state = 3;
  *buf++ = state;
  if(sGSMState.flagGotPhNum)
    {
	  len = strlen(cusdBuf);
      memcpy(buf,cusdBuf,len);
	  sGSMState.flagWaitPhNum = 0;
    }
  len++;
  #endif
  
  return len;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void callGSMFunction(u8 state)
{

    switch(state)
	{
	  case 0 : setCUSDQuery(innerState.activeSIMCard);	   break;
	  case 1 : setMicrophoneSens();    break;
	  case 2 : setSpeakerLevel();	   break;
	  case 3 : setCUSDPhNumQuery(innerState.activeSIMCard);   break;
	  case 4 : 
				memset(cnfBuf,0,sizeof(cnfBuf));
				strcpy((char *)cnfBuf,(char *)QUERIES[1]);
				strncat(cnfBuf,"\r\n",strlen("\r\n") );  /*add  CR NL */
	            break;
	  case 5 : 
				memset(cnfBuf,0,sizeof(cnfBuf));
				strcpy((char *)cnfBuf,(char *)QUERIES[39]);
				strncat(cnfBuf,"\r\n",strlen("\r\n") );  /*add  CR NL */
	            break;
	  case 6 : 	   break;   /*ATA*/
	  case 7 : 	   break;	/*ATH*/

	  default:                break;
	}
   //fifoPacketPut(&fifoGSMATCmdPack,bufGSMFifoATCmd, lenData);
   fifoPacketPut(&fifoGSMATCmdPack, (u8 *)cnfBuf, strlen(cnfBuf), state);
}

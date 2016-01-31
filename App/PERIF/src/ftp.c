
#include "stm32f10x.h"
#include "ftp.h"
#include "ftpInner.h"
#include "gprs_modem.h"
#include "common.h"
#include "hw_config.h"
#include "flash.h"
#include "protocol_bin.h"
#include "crc.h"
#include "rtc.h"
#include "hand_conf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define  TIME_INTRV_CMDS        MS_100
#define  TIME_FTP_READ_INTRV    MS_100 * 5
#define  TIME_BEGIN_GPRS_CMD    ONE_SEC * 10

//intervals
#define  TIMEOUT_USUAL_CMD       ONE_SEC * 3
#define  TIMEOUT_1MIN            ONE_SEC * 30	//60
#define  TIMEOUT_BEGIN_GPRS_CMD  ONE_SEC * 10
#define  TIMEOUT_END_GPRS_CMD    ONE_SEC * 10
#define  TIMEOUT_START_FTP_CMD   ONE_SEC * 10	//20
#define  TIMEOUT_OPEN_PUT_CMD    ONE_SEC * 10

#define  RESEND_TIMEOUT_RETRY   3
#define  RESEND_ERROR_RETRY     3  
////////////////////////temp sesctor  //////////////////////
extern u8  updateFlag;

 
u16 bytesFTPToReadForPacket;
u16 bytesFTPRealRead;

extern u8 flagGoFtpControl;
u16 byteCntPGSize;
u16 byteCntPacket;
extern u8      UpgradeBuffer[];   //UPGRADE_BUF_SIZE
extern u8      cntPageSaved;

extern RTC_t rtc;

/////////////////////  end of temp sector////////////////////
typedef enum 
{
  stCheckConn = 0,
  stATEOFF,
  stBearContype,
  stBearAPN,
  stBearUSER,
  stBearPWD,
  stServ,
  stUserName,
  stPass,
  stGetPath,
  stGetFileName,
  stPutPath,
  stPutFileName,
  stType,
  stMode,
  stBeginGPRS,
  stOpenFTP,
  stFTPRead,
  stEndGPRS, 
  stQueryBearer,
  //stFTPGetClose,
  stFTPPutClose,
  stOpenPutFTP,
  stFTPWrite,
  stFTPPutOpt

}stateType;

static stateType currState;

void TbFunCheckConn(void);
void TbFunATEOFF(void);
void TbBearContype(void);
void TbBearAPN(void);
void TbBearUSER(void);
void TbBearPWD(void);
void TbFunServ(void);
void TbFunUserName(void);
void TbFunPass(void);
void TbFunGetPath(void);
void TbFunGetFileName(void);
void TbFunPutPath(void);
void TbFunPutFileName(void);
void TbFunType(void);
void TbFunMode(void);
void TbFunBeginGPRS(void);
void TbFunOpenFTP(void);
void TbFunFTPRead(void);
void TbFunEndGPRS(void);
void TbFunQueryBearer(void);
//void TbFunFTPGetClose(void);
void TbFunFTPPutClose(void);
void TbFunOpenPutFTP(void);
void TbFunFTPWrite(void);
void TbFunFTPPutOpt(void);
//static void TbFunEmpty(void);

static void  (*stateTable[])(void) ={
							  TbFunCheckConn,
							  TbFunATEOFF,
							  TbBearContype,
							  TbBearAPN,
							  TbBearUSER,
							  TbBearPWD,
							  TbFunServ,
							  TbFunUserName,
							  TbFunPass,
							  TbFunGetPath,
							  TbFunGetFileName,
							  TbFunPutPath,
							  TbFunPutFileName,
							  TbFunType,
							  TbFunMode,
							  TbFunBeginGPRS,
							  TbFunOpenFTP,
							  TbFunFTPRead,
							  TbFunEndGPRS,
							  TbFunQueryBearer,
							  //TbFunFTPGetClose,
							  TbFunFTPPutClose,
							  TbFunOpenPutFTP,
							  TbFunFTPWrite,
							  TbFunFTPPutOpt
							  //TbFunEmpty
							 };
// ---------    new features   ------------
/*
typedef struct  
{
  stateType fDescState;
  void  (*stTable)(void);
  u8    timeoutRepeat;

}funDescType;

funDescType fdCheckConn = {stCheckConn, TbFunCheckConn, 3 };
funDescType fdFTPCmds[2];
 */
//   ---------------- end of new features

  
static const u8 * QUERIES[] =  {
                            "AT\r\n",			  //0
							"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n",
							"AT+SAPBR=3,1,\"APN\",\"",
							"AT+SAPBR=3,1,\"USER\",\"",
							"AT+SAPBR=3,1,\"PWD\",\"",
							"AT+FTPSERV=\"",		  //5
							"AT+FTPUN=\"",
							"AT+FTPPW=\"",
							"AT+FTPGETPATH=\"",
							"AT+FTPGETNAME=\"",	  
							"AT+FTPMODE=1\r\n",		  //10
							"AT+FTPTYPE=\"",
							"AT+FTPPUT=1\r\n",
							"AT+FTPGET=1\r\n",
							"AT+FTPGET=2,",		  
							"AT+FTPPUT=2,",       //15	  "AT+FTPPUT=2,"
							"AT+SAPBR=1,1\r\n",
							"AT+SAPBR=0,1\r\n",
							"AT+SAPBR=2,1\r\n", 
							"ATE0\r\n",
							"AT+FTPPUTPATH=\"",	  //20
							"AT+FTPPUTNAME=\"",
							"AT+FTPGET=2,0\r\n",   //should exclude		  
							"AT+FTPPUT=2,0\r\n",
							"AT+FTPPUTOPT=\"APPE\"\r\n"   
						};

static const u8 * REPLIES[] =  {
							"OK",				 //0
							"ERROR",
							"AT",
							"+FTPGET:1,1",
							"+FTPGET:2,",	 
							"+FTPGET:1,64",    // 5
							"+SAPBR: 1,",
							"+FTPPUT:2,",	
							"+FTPGET:1,0",
							"+FTPPUT:1,1,",
							"+FTPGET:1,72",	   /*10  - error when bad username*/
							"+FTPGET:1,66"	   /*11  - error when bad psw*/
						};

typedef struct _flagsFTPState
{
			  u8  flagPageDataReady  : 1;    /* bit 0*/
			  u8  flagLastPage       : 1;
			  u8  flagFTPStart       : 1;
			  u8  flagOpenFTP        : 1;
			  u8  flagOutBoot        : 1;
			  u8  flagAlrSent        : 1;
			  u8  flagTimeout        : 1;
			  u8  flagNewCycle       : 1;
			  u8  flagOnceSended     : 1;   // when we first went through all configuration commands - no need to repeat them
			  u8  flagIniFromGSM     : 1;
			  u8  flagBeginGPRS      : 1;
			  u8  flagFrontBlock     : 1;
			  u8  flagWriteRawData   : 1;
			  u8  flagReturnToGPRS   : 1;
			  u8  flagEndGetSession  : 1;
			  u8  flagServBufFill    : 1;
			  u8  flagUNBufFill      : 1;
			  u8  flagPswBufFill     : 1;
  volatile    u8  flagPrepareToSend  : 1;
  volatile    u8  flagTimeToSend     : 1;


}flagsFTPState;
flagsFTPState sFTPState;

u8  flagReadRawData;

#define REPL_OK_LEN              strlen((char *)REPLIES[0])  //OK
#define REPL_ERROR_LEN           strlen((char *)REPLIES[1])  //ERROR
#define REPL_FTPGET11_LEN        strlen((char *)REPLIES[3])  //+FTPGET:1,1
#define REPL_FTPGET164_LEN       strlen((char *)REPLIES[5])  //+FTPGET:1,64
#define REPL_FTPGET2_LEN         strlen((char *)REPLIES[4])  //+FTPGET:2
#define REPL_SAPBR2_LEN          strlen((char *)REPLIES[6])  //+SAPBR:1,1
#define REPL_FTPPUT2_LEN         strlen((char *)REPLIES[7])  //+FTPPUT:2,
#define REPL_FTPGET10_LEN        strlen((char *)REPLIES[8])  //+FTPGET:1,0
#define REPL_FTPPUT11_LEN        strlen((char *)REPLIES[9])  //+FTPPUT:1,1
#define REPL_FTPGET1E_LEN        strlen((char *)REPLIES[10])  //+FTPGET:1,72

extern u8 tempBuf[];    //REC_AN_BUF_SIZE

u32  timerValueSendFTP;
u32  timerValueTimeoutFTP;
static u32  timerFDTimeout;

static u8   numTryTimeoutResendAT;
static u8   numTryTimeoutResendCmd;

u8  numTryBegin ;
u8  cntZeroBytesRead;

extern t_innerState innerState;
extern configSet config;

#define FTP_SIZE_FRONT_BLOCK          64
u8 bufFrontBlock[FTP_SIZE_FRONT_BLOCK];

//u8  cntGPRSStarted;	/* when we count to 3 times podryad - then we can't start GPRS session*/

//"AT+SAPBR=3,1,\"APN\",\"",
char ftpBearAPNBuf[50];
//char * APNBear = "internet.mts.ru";
//"AT+SAPBR=3,1,\"USER\",\"",
char ftpBearUserBuf[30];
//char * UserBear = "mts";
//"AT+SAPBR=3,1,\"PWD\",\"",
char ftpBearPswBuf[30];
//char * PasswordBear = "mts";
//"AT+FTPSERV=",		  
char ftpServBuf[32];

//char * servAddress = "81.23.151.1";	//from Home
//char * servAddress = "31.13.128.67";		// on Work

//"AT+FTPUN=",		  
char ftpUnBuf[32];
//char * UserName = "gsm";
//AT+FTPPW=\"
char ftpPswBuf[32];
//char * Password = "qwe";
//"AT+FTPGETPATH=\"",
char ftpGetPathBuf[30];
char ftpPutPathBuf[30];
char * Path = "/";
//"AT+FTPGETNAME=\"",
char ftpGetFileNameBuf[40];
char ftpPutFileNameBuf[40];
char * FileName = "firmware.bin";
//char * FileName = "fw_tv2.bin";

//"AT+FTPTYPE=\"",
char ftpTypeBuf[30];
char * ftpType = "I";
//"AT+FTPGET=2,",
u8 iLen;
u16 nRead = FTP_PACKET_SIZE;
char ftpReadBuf[20];
char ftpReadNumber[5];
u8   ftpDataPut[60];
char ftpPutWriteBuf[20];
//-------------------------------------------------------------------------------------------

void ftpGSMPrepare(void)
{
   //fdFTPCmds[0] = fdCheckConn;
   //fdFTPCmds[0].fDescState = stCheckConn;
   //fdFTPCmds[0].stTable = TbFunCheckConn;
   //fdFTPCmds[0].timeoutRepeat = 3;

   //u8 * ptrData,* ptrData2;
   //u8 k = 0;

   sFTPState.flagOnceSended = 0;
   //"AT+SAPBR=3,1,\"APN\",\"",
   memset(ftpBearAPNBuf,0,sizeof(ftpBearAPNBuf));
   strcpy(ftpBearAPNBuf,(char *)QUERIES[2]);
   strncat(ftpBearAPNBuf,(char *)config.apn[innerState.activeSIMCard],strlen((char *)config.apn[innerState.activeSIMCard]) );
   strncat(ftpBearAPNBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+SAPBR=3,1,\"USER\",\"",
   memset(ftpBearUserBuf,0,sizeof(ftpBearUserBuf));
   strcpy(ftpBearUserBuf,(char *)QUERIES[3]);
   strncat(ftpBearUserBuf,(char *)config.username[innerState.activeSIMCard],strlen((char *)config.username[innerState.activeSIMCard]) );
   strncat(ftpBearUserBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+SAPBR=3,1,\"PWD\",\"",
   memset(ftpBearPswBuf,0,sizeof(ftpBearPswBuf));
   strcpy(ftpBearPswBuf,(char *)QUERIES[4]);
   strncat(ftpBearPswBuf,(char *)config.password[innerState.activeSIMCard],strlen((char *)config.password[innerState.activeSIMCard]) );
   strncat(ftpBearPswBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPSERV=",		  
   //memset(ftpServBuf,0,sizeof(ftpServBuf));
   //strcpy(ftpServBuf,(char *)QUERIES[5]);
   //strncat(ftpServBuf,(char *)config.ftpServer,strlen((char *)config.ftpServer) );
   //strncat(ftpServBuf,"\"\r\n",sizeof("\"\r\n") );
//   //"AT+FTPUN=",		  
//   memset(ftpUnBuf,0,sizeof(ftpUnBuf));
//   strcpy(ftpUnBuf,(char *)QUERIES[6]);
//   strncat(ftpUnBuf,(char *)config.ftpUsername,strlen((char *)config.ftpUsername) );   
//   strncat(ftpUnBuf,"\"\r\n",sizeof("\"\r\n") );
//   //AT+FTPPW=\"
//   memset(ftpPswBuf,0,sizeof(ftpPswBuf));
//   strcpy(ftpPswBuf,(char *)QUERIES[7]);
//   strncat(ftpPswBuf,(char *)config.ftpPassword,strlen((char *)config.ftpPassword) );  
//   strncat(ftpPswBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPGETPATH=\"",
   memset(ftpGetPathBuf,0,sizeof(ftpGetPathBuf));
   strcpy(ftpGetPathBuf,(char *)QUERIES[8]);
   strncat(ftpGetPathBuf,Path,strlen(Path) );
   strncat(ftpGetPathBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPGETNAME=\"",
   memset(ftpGetFileNameBuf,0,sizeof(ftpGetFileNameBuf));
   strcpy(ftpGetFileNameBuf,(char *)QUERIES[9]);
   strncat(ftpGetFileNameBuf,FileName,strlen(FileName) );
   strncat(ftpGetFileNameBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPPUTPATH=\"",
   memset(ftpPutPathBuf,0,sizeof(ftpPutPathBuf));
   strcpy(ftpPutPathBuf,(char *)QUERIES[20]);
   strncat(ftpPutPathBuf,Path,strlen(Path) );
   strncat(ftpPutPathBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPPUTNAME=\"",
   memset(ftpPutFileNameBuf,0,sizeof(ftpPutFileNameBuf));
   strcpy(ftpPutFileNameBuf,(char *)QUERIES[21]);
   snprintf((char * )&ftpPutFileNameBuf[strlen((char *)ftpPutFileNameBuf)],7,"%d",config.idBlock);
   strncat(ftpPutFileNameBuf,".log",strlen(".log") );
   strncat(ftpPutFileNameBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPTYPE=\"",
   memset(ftpTypeBuf,0,sizeof(ftpTypeBuf));
   strcpy(ftpTypeBuf,(char *)QUERIES[11]);
   strncat(ftpTypeBuf,ftpType,strlen(ftpType) );
   strncat(ftpTypeBuf,"\"\r\n",sizeof("\"\r\n") );

   setTimerSendFTP(TIME_FIRST_INI);
   sFTPState.flagIniFromGSM = 0;
   //cntGPRSStarted = 0;
   //flagGoFtpControl = 1;  //debugga
  
}

//-------------------------------------------------------------------------------------------

static void TbFunCheckConn(void)  //AT
{
  if( (sFTPState.flagFTPStart) || (!sFTPState.flagAlrSent) || (sFTPState.flagTimeout))
   {
    if(sFTPState.flagPrepareToSend)
	 { 
	  sFTPState.flagPrepareToSend = 0;
      if(sFTPState.flagTimeout)
	   {
	    sFTPState.flagTimeToSend = 1; 
		numTryTimeoutResendAT++;
	    sFTPState.flagTimeout = 0;
	   }
	  else
		numTryTimeoutResendAT = 0;
      cmdSend((u8 *)QUERIES[0]);
	  sFTPState.flagAlrSent = 1;
	  sFTPState.flagFTPStart = 0;
	  timerFDTimeout = TIMEOUT_USUAL_CMD;
	  sFTPState.flagTimeToSend = 1;
     }
   }
}

//-------------------------------------------------------------------------------------------
void TbFunATEOFF(void)
{
  cmdPrepareSend((u8 *)QUERIES[19], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbBearContype(void)
{
  cmdPrepareSend((u8 *)QUERIES[1], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbBearAPN(void)
{
  cmdPrepareSend((u8 *)ftpBearAPNBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbBearUSER(void)
{
  cmdPrepareSend((u8 *)ftpBearUserBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbBearPWD(void)
{
  cmdPrepareSend((u8 *)ftpBearPswBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunServ(void)
{
   if(!sFTPState.flagServBufFill)
     {
	   sFTPState.flagServBufFill = 1;
	   //"AT+FTPSERV=",		  
	   memset(ftpServBuf,0,sizeof(ftpServBuf));
	   strcpy(ftpServBuf,(char *)QUERIES[5]);
	   strncat(ftpServBuf,(char *)config.ftpServer,strlen((char *)config.ftpServer) );
	   strncat(ftpServBuf,"\"\r\n",sizeof("\"\r\n") );
	 }
   cmdPrepareSend((u8 *)ftpServBuf, TIMEOUT_USUAL_CMD); 
}

//-------------------------------------------------------------------------------------------
void TbFunUserName(void)
{	
   if(!sFTPState.flagUNBufFill)
     {
	   sFTPState.flagUNBufFill = 1;
	   //"AT+FTPUN=",		  
	   memset(ftpUnBuf,0,sizeof(ftpUnBuf));
	   strcpy(ftpUnBuf,(char *)QUERIES[6]);
	   strncat(ftpUnBuf,(char *)config.ftpUsername,strlen((char *)config.ftpUsername) );   
	   strncat(ftpUnBuf,"\"\r\n",sizeof("\"\r\n") );
	 }
   cmdPrepareSend((u8 *)ftpUnBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunPass(void)
{
  if(!sFTPState.flagPswBufFill)
     {
	   sFTPState.flagPswBufFill = 1;
	   //AT+FTPPW=\"
	   memset(ftpPswBuf,0,sizeof(ftpPswBuf));
	   strcpy(ftpPswBuf,(char *)QUERIES[7]);
	   strncat(ftpPswBuf,(char *)config.ftpPassword,strlen((char *)config.ftpPassword) );  
	   strncat(ftpPswBuf,"\"\r\n",sizeof("\"\r\n") );
     }
  cmdPrepareSend((u8 *)ftpPswBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunGetPath(void)
{
  cmdPrepareSend((u8 *)ftpGetPathBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunGetFileName(void)
{
  cmdPrepareSend((u8 *)ftpGetFileNameBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunPutPath(void)
{
  cmdPrepareSend((u8 *)ftpPutPathBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunPutFileName(void)
{
  cmdPrepareSend((u8 *)ftpPutFileNameBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunFTPPutOpt(void)
{
  cmdPrepareSend((u8 *)QUERIES[24], TIMEOUT_USUAL_CMD);
}

//-------------------------------------------------------------------------------------------
void TbFunType(void)
{
  cmdPrepareSend((u8 *)ftpTypeBuf, TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunMode(void)
{
  cmdPrepareSend((u8 *)QUERIES[10], TIMEOUT_USUAL_CMD);
}

//-------------------------------------------------------------------------------------------
void TbFunQueryBearer(void)
{
  cmdPrepareSend((u8 *)QUERIES[18], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunBeginGPRS(void)
{
  cmdPrepareSend((u8 *)QUERIES[16], TIMEOUT_BEGIN_GPRS_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunOpenFTP(void)
{
  cmdPrepareSend((u8 *)QUERIES[13], TIMEOUT_START_FTP_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunOpenPutFTP(void)
{
  if(sFTPState.flagEndGetSession)
    {
	  //sFTPState.flagEndGetSession = 0;
      cmdPrepareSend((u8 *)QUERIES[12], TIMEOUT_START_FTP_CMD);
	}
}

//-------------------------------------------------------------------------------------------
void TbFunFTPRead(void)
{
   //should make MAX checking
  static u16 nReadPrev = 0;
  if( (!sFTPState.flagAlrSent)  || (sFTPState.flagTimeout) )
   {
     if(!updateFlag)
	   {
	     if(sFTPState.flagLastPage)
		  {
		    //currState = stEndGPRS;  //was earlier - when there was not PUT-cmds
			currState = stOpenPutFTP;
			sFTPState.flagLastPage = 0;
			sFTPState.flagOutBoot = 1;
		  }
		 else  /* not last page*/
		  {
		   nRead = FTP_PACKET_SIZE - bytesFTPToReadForPacket;
		   if(sFTPState.flagFrontBlock)
		      nRead = FTP_SIZE_FRONT_BLOCK;
		   if(nRead != nReadPrev)  /* if we received less bytes we asked than we should scan added (to pageSize) number of bytes*/
		     {
				   //"AT+FTPGET=2,",
				   memset(ftpReadNumber,0,sizeof(ftpReadNumber));
				   iLen = getLenNum(nRead);
				   int2char((char * )ftpReadNumber,nRead,iLen,10);
				   memset(ftpReadBuf,0,sizeof(ftpReadBuf));
				   strcpy(ftpReadBuf,(char *)QUERIES[14]);
				   strncat(ftpReadBuf,ftpReadNumber,strlen(ftpReadNumber) );
				   strncat(ftpReadBuf,"\r\n",sizeof("\r\n") );
				   //sFTPState.flagScannedLess = 1;
			  }
			//else
			//   sFTPState.flagScannedLess = 0;

			 nReadPrev = nRead;
		     //at+ftpget=2,1024
			 if(sFTPState.flagPrepareToSend)
				{ 
				  sFTPState.flagPrepareToSend = 0;
				  if(!checkTimeoutFunFTP())
					 cmdSend((u8 *)ftpReadBuf);
				  timerFDTimeout = TIMEOUT_BEGIN_GPRS_CMD;
				  sFTPState.flagAlrSent = 1;
				  sFTPState.flagTimeToSend = 1;
				}
		  }
	   }
   }
}

//-------------------------------------------------------------------------------------------
void TbFunFTPWrite(void)  //"AT+FTPPUT=2,8\r\n",
{
   u8 * ptrDataDst,* ptrDataSrc;
   u8 k;
   u16 len;
   if(!sFTPState.flagWriteRawData)
     {
       memset(ftpDataPut,0,sizeof(ftpDataPut));
       strcpy((char *)ftpDataPut,"Updated ");
	   ptrDataDst = &ftpDataPut[strlen((char *)ftpDataPut)];
	   ptrDataSrc = (u8 *)readRTCTime(&rtc);
	   k = 0;
	   len = strlen((const char *)readRTCTime(&rtc));
	   while(k < len)
	     {  
		   k++;
		   *ptrDataDst++ = *ptrDataSrc++;
		 }

	   memset(ftpReadNumber,0,sizeof(ftpReadNumber));
	   len = strlen((char *) ftpDataPut );
	   iLen = getLenNum(len);
	   int2char((char * )ftpReadNumber,len,iLen,10);
	   memset(ftpPutWriteBuf,0,sizeof(ftpPutWriteBuf));
	   strcpy(ftpPutWriteBuf,(char *)QUERIES[15]);
	   strncat(ftpPutWriteBuf,ftpReadNumber,strlen((char *)ftpReadNumber) );
	   strncat(ftpPutWriteBuf,"\r\n",sizeof("\r\n") );
	    
       cmdPrepareSend((u8 *)ftpPutWriteBuf, TIMEOUT_OPEN_PUT_CMD);

	 }
   else if(sFTPState.flagPrepareToSend)
     {
	    sFTPState.flagPrepareToSend = 0;
		cmdSend(ftpDataPut);
		sFTPState.flagTimeToSend = 1;
	 }
}
//-------------------------------------------------------------------------------------------
//void TbFunFTPGetClose(void) //not using yet - because return : "operation not allowed"
//{
//  cmdPrepareSend((u8 *)QUERIES[22], TIMEOUT_USUAL_CMD);
//}
//-------------------------------------------------------------------------------------------
void TbFunFTPPutClose(void) //
{
  cmdPrepareSend((u8 *)QUERIES[23], TIMEOUT_USUAL_CMD);
}
//-------------------------------------------------------------------------------------------
void TbFunEndGPRS(void)
{
  cmdPrepareSend((u8 *)QUERIES[17], TIMEOUT_END_GPRS_CMD);
}
//-------------------------------------------------------------------------------------------
//void TbFunEmpty(void)
//{
  //USBWriteChar('v');  
//}

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
void checkReadRawData(u8 data)
{
   static u8  pageNum = 0;
   //static u8 i = 0;
  if(sFTPState.flagOpenFTP)
	   {
	    sFTPState.flagOpenFTP = 0;
	    pageNum = 0;
		byteCntPGSize = 0;
	   }
	   //we need to scan defined(exact)number of bytes
  if(byteCntPacket < (bytesFTPRealRead))
	{
	     byteCntPacket++;

		 if(sFTPState.flagFrontBlock)
			 bufFrontBlock[byteCntPGSize++] = data;
		 else
	         UpgradeBuffer[byteCntPGSize++] = data;

		 if(byteCntPacket == bytesFTPRealRead)  //debugga
           {
		     if(!sFTPState.flagFrontBlock)
			   {
				 bytesFTPToReadForPacket = bytesFTPToReadForPacket + bytesFTPRealRead;
				 if( bytesFTPToReadForPacket == FTP_PACKET_SIZE)
				     bytesFTPToReadForPacket = 0;
			   }
			 else	 /* we read front block(64 bytes) */
			   {
			     byteCntPGSize = 0;
				 sFTPState.flagFrontBlock = 0;
				/* to update there have to be the same VERSION_MAIN; current SUB_VERSION has to be less or equal than new;
				 current BUILD has to be less then new */
				 if(!( ( bufFrontBlock[0] == VERSION_MAIN ) && (bufFrontBlock[1] >= VERSION_SUB) && ( (bufFrontBlock[3] | (bufFrontBlock[2] << 8)) > VERSION_BUILD) ))
					 {
					    USBWriteStr("version wrong \r\n");
					    sFTPState.flagReturnToGPRS = 1;
						currState = stEndGPRS;
					 }
			     else
				    USBWriteStr("version right \r\n");
			   }
			 flagReadRawData = 0;
			 byteCntPacket = 0;
	       }

		 if (byteCntPGSize == FLASH_PAGE_SIZE)
		    {
				 sFTPState.flagPageDataReady = 1;
				 pageNum++;
		   }
	}

}
//-------------------------------------------------------------------------------------------
void loopFTP(void)
{
  if(flagGoFtpControl)
     {
      if( !sFTPState.flagIniFromGSM )
        {
           sFTPState.flagIniFromGSM = 1;
		   innerState.bootFTPStarted = 1;
           initializeSMFTP();
        }
	  stateTable[currState]();
	  if(numTryTimeoutResendAT >= RESEND_TIMEOUT_RETRY)
	     {
		   gprsModemOn(innerState.activeSIMCard);
		   numTryTimeoutResendAT = 0;
		   currState = stCheckConn;
		   setTimerSendFTP(TIME_FIRST_INI);
		   sFTPState.flagTimeToSend = 0;
		   sFTPState.flagFTPStart = 1;
		   sFTPState.flagOnceSended = 0;
		 }
	  if(sFTPState.flagTimeToSend)
	     {
		   sFTPState.flagTimeToSend = 0;
	       setTimerTimeoutFTP(timerFDTimeout);
	       checkForSend();
		 }
	  handleRcvFTP();
	 }
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/

void handleRcvFTP(void)
{
  u32 crcRcv, crcFlash;
  u8 i;
  if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],REPL_OK_LEN)))  //OK
	    { 
	      
		  switch(currState)
		   {
			 case  stCheckConn  :  setTimerSendAux(TIME_INTRV_CMDS);
							       currState = stATEOFF; //
								   if(sFTPState.flagOnceSended)
								     currState = stBeginGPRS; 
								   if(!sFTPState.flagNewCycle)
								     setTimerSendAux(TIME_INTRV_CMDS);
								   else
								     setTimerSendAux(TIMEOUT_1MIN);

  								   break;
			 case  stATEOFF     :  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stBearContype;
  								   break;
		   
			 case  stBearContype:  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stBearAPN;
  								   break;
			 case  stBearAPN    :  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stBearUSER;
								   break;
			 case  stBearUSER	:  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stBearPWD;
								   break;
			 case  stBearPWD    :  setTimerSendAux(TIME_INTRV_CMDS);
			                       currState = stServ;
								   break;
			 case  stServ       :  setTimerSendAux(TIME_INTRV_CMDS);
			 					   sFTPState.flagServBufFill = 0;
			                       currState = stUserName;
								   break;
			 case  stUserName   :  setTimerSendAux(TIME_INTRV_CMDS);
			 					   sFTPState.flagUNBufFill = 0;
                        		   currState = stPass;
			                       break;
			 case  stPass       :  setTimerSendAux(TIME_INTRV_CMDS);
			 					   sFTPState.flagPswBufFill = 0;
								   currState = stGetPath;
								   break;
			 case  stGetPath    :  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stGetFileName;
								   break;
			 case  stGetFileName:  setTimerSendAux(TIME_INTRV_CMDS);
			                       //currState = stType;
								   currState = stPutPath;
			                       break;
			 case  stPutPath    :  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stPutFileName;
								   break;
			 case  stPutFileName:  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stFTPPutOpt;
								   break;
			 case  stFTPPutOpt  :  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stType;
								   break;
			 case  stType       :  setTimerSendAux(TIME_INTRV_CMDS);
			                       currState = stMode;
			                       break;
			 case  stMode       :  setTimerSendAux(TIME_INTRV_CMDS);
								   //currState = stBeginGPRS;
								   currState = stQueryBearer;
								   break;
								   
			 case  stBeginGPRS  :  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stOpenFTP; //
								   sFTPState.flagOnceSended = 1;
								   sFTPState.flagNewCycle = 0;
								   break;
			 
			 
			 case  stEndGPRS    :  if(sFTPState.flagReturnToGPRS)
									 {
									   sFTPState.flagReturnToGPRS = 0;
									   flagGoFtpControl = 0;
									   sFTPState.flagIniFromGSM = 0;
									 }
								   else
								     {
				 					   setTimerSendAux(TIME_INTRV_CMDS);
				                       currState = stCheckConn;  
									   sFTPState.flagOnceSended = 1;
									   /* here checking CRC of written flash and received from file*/
									   //crcRcv = *(u32 *)&bufFrontBlock[4];
									   crcFlash = 0;
									   crcRcv = (bufFrontBlock[4] << 24) | (bufFrontBlock[5]<< 16) | (bufFrontBlock[6]<< 8) | (bufFrontBlock[7]);
									   for (i = 0; i < cntPageSaved; i++)
									     {
									          read_page(START_ADDRESS_FW + i * FLASH_PAGE_SIZE , UpgradeBuffer, FLASH_PAGE_SIZE);
											  crcFlash = calcBlockCont(UpgradeBuffer,FLASH_PAGE_SIZE);
									  	 }
	
									   if(crcRcv == crcFlash)
									     {
									       if(sFTPState.flagOutBoot)
										    {
											   SYSTEM_RESTART;	   //crc ok - save pages number and restart
											}
									     }
										else	//bad CRC - just restart - should redo to return to GPRS usual
										   {
											    __disable_irq();
											    // restart system
											    NVIC_SystemReset();
										   }
									  }
                                   break;
			 case  stQueryBearer:  setTimerSendAux(TIME_INTRV_CMDS);
								   currState = stEndGPRS;
								   if(sFTPState.flagBeginGPRS)
								       currState = stBeginGPRS; //
								   break;
			 case  stFTPRead    :  setTimerSendAux(TIME_FTP_READ_INTRV * (1 + cntZeroBytesRead ) );
							       currState = stFTPRead; //
									 
								   if(sFTPState.flagPageDataReady)
								     {
									    sFTPState.flagPageDataReady = 0;
										if(sFTPState.flagOpenFTP)	 // if there was just start FTP-session than it's first page
										 {
										  updateFlag = 1;
										 }
										else 
										  updateFlag = 2;
									 }
								    else if(cntZeroBytesRead > FTP_QTY_READ_TRY)
								      {
									    sFTPState.flagLastPage = 1;
									    cntZeroBytesRead = 0;
										//updateFlag = 2; 
									  }

								   break;
			 //case  stFTPGetClose : setTimerSendAux(TIME_INTRV_CMDS);
			 //                      currState = stOpenPutFTP;
			 //					   break;
			 //case  stOpenPutFTP  : setTimerSendAux(TIME_INTRV_CMDS);
			                       //currState = stFTPWrite;
			 //					   break;
			 case  stFTPWrite    : setTimerSendAux(TIME_INTRV_CMDS);
			                       currState = stFTPPutClose;
								   break;
			 case  stFTPPutClose : setTimerSendAux(TIME_INTRV_CMDS);
			                       currState = stEndGPRS;
								   break;
								   
		  
			 default            : break;
		   }
		}
	  else if((!strncmp((char *)REPLIES[1],(char *)&tempBuf[0],REPL_ERROR_LEN)))  //ERROR
	    { 
		  if(currState == stBeginGPRS)
		   {
			    setTimerSendAux(TIME_BEGIN_GPRS_CMD * (numTryBegin + 1)); 
				sFTPState.flagOnceSended = 1;
				currState = stBeginGPRS;
				if(numTryBegin++ >= RESEND_ERROR_RETRY)
					{
					  numTryBegin = 0;
					  sFTPState.flagNewCycle = 1; 
					  currState = stEndGPRS; //debugga
					}
				//currState = stEndGPRS;
            }			 
		}
	   else if((!strncmp((char *)REPLIES[3],(char *)&tempBuf[0],REPL_FTPGET11_LEN)))  //+FTPGET:1,1
	    { 
		  if(currState == stOpenFTP)
		    {
			  currState = stFTPRead; //
			  sFTPState.flagOnceSended = 1;
			  sFTPState.flagNewCycle = 0;
			  setTimerSendAux(TIME_FTP_READ_INTRV);
			  sFTPState.flagOpenFTP = 1;
			  bytesFTPToReadForPacket = 0;
		    }
		}
       else if((!strncmp((char *)REPLIES[5],(char *)&tempBuf[0],REPL_FTPGET164_LEN)))  //+FTPGET:1,64
	    { 
		  if(currState == stOpenFTP)
		    {
			  //Timeout happened
			  currState = stOpenFTP;
			  setTimerSendAux(TIME_FTP_READ_INTRV);
		    }
		}
       else if((!strncmp((char *)REPLIES[8],(char *)&tempBuf[0],REPL_FTPGET10_LEN)))  //+FTPGET:1,0  end of session
	    { 
			  currState = stOpenPutFTP;
			  setTimerSendAux(TIMEOUT_OPEN_PUT_CMD);
			  sFTPState.flagEndGetSession = 1;
			  //USBWriteStr("-E-");
		}
       else if((!strncmp((char *)REPLIES[9],(char *)&tempBuf[0],REPL_FTPPUT11_LEN)))  //+FTPPUT:1,1,    
	    { 
		      sFTPState.flagEndGetSession = 0;
			  if(currState == stOpenPutFTP)
			    {
				    setTimerSendAux(TIME_FTP_READ_INTRV);
				    currState = stFTPWrite;
				}
		}

       else if((!strncmp((char *)REPLIES[7],(char *)&tempBuf[0],REPL_FTPPUT2_LEN)))  //+FTPPUT:2,
	    { 
		  if(currState == stFTPWrite)
		    {
			  sFTPState.flagWriteRawData = 1;
			  setTimerSendAux(TIME_FTP_READ_INTRV);
		    }
		}
       else if((!strncmp((char *)REPLIES[10],(char *)&tempBuf[0],REPL_FTPGET1E_LEN)))  //+FTPGET:1,72 
	    { 
		    /* should mark error and return to main GSM*/
		    sFTPState.flagReturnToGPRS = 1;
			currState = stEndGPRS;
		}
       else if((!strncmp((char *)REPLIES[11],(char *)&tempBuf[0],REPL_FTPGET1E_LEN)))  //+FTPGET:1,66 
	    { 
		    /* should mark error and return to main GSM*/
		    sFTPState.flagReturnToGPRS = 1;
			currState = stEndGPRS;
		}
       else if( (!strncmp((char *)REPLIES[4],(char *)&tempBuf[0],REPL_FTPGET2_LEN)) ) //+FTPGET:2,
	    { 
		   // here  comes data  - we should read it and after that we wait 'OK'
		   if(currState == stFTPRead)
		     {
			   bytesFTPRealRead = atoi((const char *)&tempBuf[REPL_FTPGET2_LEN]);
			   
			   if(bytesFTPRealRead)
			      {
			       flagReadRawData = 1;
				   cntZeroBytesRead = 0;
				  }
			   else
			     {
			       cntZeroBytesRead++;
				 }
			 }
		}
       else if((!strncmp((char *)REPLIES[6],(char *)&tempBuf[0],REPL_SAPBR2_LEN)))  //+SAPBR: 1,  
	    { 
		  if(currState == stQueryBearer)
		    {
			  //currState = stEndGPRS;
			  sFTPState.flagBeginGPRS = 0;
			  if( (tempBuf[REPL_SAPBR2_LEN] == 0x32) || (tempBuf[REPL_SAPBR2_LEN] == 0x33) )
			    {
			      //currState = stBeginGPRS;
				  sFTPState.flagBeginGPRS = 1;
				}
		    }
		}
    } 
}

// ------------------------------------------------------------------------------
void cmdPrepareSend(u8 * buf, u32 timerVal)
{
  if( (!sFTPState.flagAlrSent)  || (sFTPState.flagTimeout) )
     {
	    if(sFTPState.flagPrepareToSend)
		 { 
		  sFTPState.flagPrepareToSend = 0;
		  /*timeout handling*/
		  timerFDTimeout = timerVal; 
		  /*ready to send*/
		  sFTPState.flagTimeToSend = 1;
		  if(!checkTimeoutFunFTP())
	         cmdSend(buf);
		  /*aux actions*/
		  sFTPState.flagAlrSent = 1;
		 }
     }
}

// ------------------------------------------------------------------------------
// *************************************TIMERS**********************************
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
void handleTimerSendGSMFTP(void)
{
    if(timerValueSendFTP)
	  {
	   timerValueSendFTP--;
	   if(!timerValueSendFTP )
	      sFTPState.flagPrepareToSend = 1;
	  }
	if(timerValueTimeoutFTP)
	  {
	   timerValueTimeoutFTP--;
	   if(!timerValueTimeoutFTP )
	    {
	      sFTPState.flagTimeout = 1;
          sFTPState.flagPrepareToSend = 1;
		}
	  }
}
// ------------------------------------------------------------------------------
void setTimerSendFTP(u32 val)
{
   	timerValueSendFTP = val;
    setTimerTimeoutFTP(0);
}

// ------------------------------------------------------------------------------
void setTimerTimeoutFTP(u32 val)
{
   	timerValueTimeoutFTP = val;
}
// ------------------------------------------------------------------------------
void setTimerSendAux(u32 val)
{
    sFTPState.flagPrepareToSend = 0;
	sFTPState.flagAlrSent = 0;
   	timerValueSendFTP = val;
    setTimerTimeoutFTP(0);
}

// ------------------------------------------------------------------------------
// return 1 - if timeout happened > 3 times, 0 - no timeout
// ------------------------------------------------------------------------------
u8 checkTimeoutFunFTP(void)
{
     u8 result = 0;
     if(sFTPState.flagTimeout)
	  {
		if(++numTryTimeoutResendCmd >= RESEND_TIMEOUT_RETRY)
		   {
		       numTryTimeoutResendCmd = 0;
		       currState = stCheckConn;
			   //setTimerSendFTP(TIME_INTRV_CMDS);
			   //flagToSend = 0;
			   sFTPState.flagNewCycle = 1;
			   result = 1;
		   }
	    sFTPState.flagPrepareToSend = 1; 
	    sFTPState.flagTimeout = 0;
	  }
	 else
		numTryTimeoutResendCmd = 0;

	 return result;
}

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u32 getFTPFlags(void)
{
   return 0; //flagsFTP;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
void initializeSMFTP(void)
{
   gprsModemOff();
   currState = stCheckConn;
   sFTPState.flagFTPStart = 1;
   sFTPState.flagPrepareToSend = 0;
   gprsModemOn(innerState.activeSIMCard);
   setTimerSendFTP(TIME_FIRST_INI);
   sFTPState.flagFrontBlock = 1;
   sFTPState.flagWriteRawData = 0;
   sFTPState.flagOnceSended = 0;
}


















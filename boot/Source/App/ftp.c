
#include "stm32f10x.h"
#include "ftp.h"
#include "string.h"
#include "gprs_modem.h"
#include "platform.h"
#include "dio_led.h"
#include <stdlib.h>
//#include "usart.h"
#define  TIME_INTRV_CMDS        MS_100
#define  TIME_FTP_READ_INTRV    MS_100 * 5

//intervals
#define  TIMEOUT_USUAL_CMD       ONE_SEC * 3
#define  TIMEOUT_1MIN            ONE_SEC * 30	//60
#define  TIMEOUT_BEGIN_GPRS_CMD  ONE_SEC * 10
#define  TIMEOUT_END_GPRS_CMD    ONE_SEC * 10
#define  TIMEOUT_START_FTP_CMD   ONE_SEC * 10	//20

#define  RESEND_TIMEOUT_RETRY   3
#define  RESEND_ERROR_RETRY     3  
////////////////////////temp sesctor  //////////////////////

extern u8  gsmStart;
extern u8  updateFlag;
extern u8  flagPageDataReady;
       u16 bytesFTPRealRead;
extern u8  flagLastPage;
	   u8  flagOpenFTP;
extern u16 byteCntPacket;
extern u16 bytesFTPToReadForPacket;
       u8  flagOutBoot;

void go_cmd(void);
/////////////////////  end of temp sector////////////////////
typedef enum 
{
  stCheckConn = 0,
  stBearContype,
  stBearAPN,
  stBearUSER,
  stBearPWD,
  stServ,
  stUserName,
  stPass,
  stGetPath,
  stGetFileName,
  stType,
  stMode,
  stBeginGPRS,
  stOpenFTP,
  stFTPRead,
  stFTPClose,
  stEndGPRS
}stateType;

stateType currState;

void TbFunCheckConn(void);
void TbBearContype(void);
void TbBearAPN(void);
void TbBearUSER(void);
void TbBearPWD(void);
void TbFunServ(void);
void TbFunUserName(void);
void TbFunPass(void);
void TbFunGetPath(void);
void TbFunGetFileName(void);
void TbFunType(void);
void TbFunMode(void);
void TbFunBeginGPRS(void);
void TbFunOpenFTP(void);
void TbFunFTPRead(void);
void TbFunFTPClose(void);
void TbFunEndGPRS(void);

void  (*stateTable[])(void) ={
							  TbFunCheckConn,
							  TbBearContype,
							  TbBearAPN,
							  TbBearUSER,
							  TbBearPWD,
							  TbFunServ,
							  TbFunUserName,
							  TbFunPass,
							  TbFunGetPath,
							  TbFunGetFileName,
							  TbFunType,
							  TbFunMode,
							  TbFunBeginGPRS,
							  TbFunOpenFTP,
							  TbFunFTPRead,
							  TbFunFTPClose,
							  TbFunEndGPRS
							 };
// ---------    new features   ------------
typedef struct  
{
  stateType fDescState;
  void  (*stTable)(void);
  u8    timeoutRepeat;

}funDescType;

funDescType fdCheckConn = {stCheckConn, TbFunCheckConn, 3 };
funDescType fdFTPCmds[2];

//   ---------------- end of new features

  
const u8 * QUERIES[] =  {
                            "AT\r\n",			  //0
							"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n",
							"AT+SAPBR=3,1,\"APN\",\"",
							"AT+SAPBR=3,1,\"USER\",\"",
							"AT+SAPBR=3,1,\"PWD\",\"",
							"AT+FTPSERV=\"",		  //5
							"AT+FTPUN=\"",
							"AT+FTPPW=\"",
							"AT+FTPGETPATH=\"",
							"AT+FTPGETNAME=\"",	  //
							"AT+FTPMODE=1\r\n",		  //10
							"AT+FTPTYPE=\"",
							"AT+FTPSCONT\r\n",
							"AT+FTPGET=1\r\n",
							"AT+FTPGET=2,",		  //
							"AT+FTPPUT=2,0\r\n",  //15
							"AT+SAPBR=1,1\r\n",
							"AT+SAPBR=0,1\r\n"
						};

const u8 * REPLIES[] =  {
							"OK",				 //0
							"ERROR",
							"AT",
							"+FTPGET:1,1",
							"+FTPGET:2,",	 //temporal decision - (4)	MAGIC_NB
							"+FTPGET:1,64"    // 5
						};


extern u8 tempBuf[64];

u32  timerValueSend;
u32  timerValueTimeout;
u32  timerFDTimeout;
u8   flagToSend;
u8   flagTimeout;
u8   numTryTimeoutResendAT;
u8   numTryTimeoutResendCmd;
u8   newCycle;
 
u8   flagReadRawData;
u8   flagOnceSended;   // when we first went through all configuration commands - no need to repeat them


//"AT+SAPBR=3,1,\"APN\",\"",
char ftpBearAPNBuf[50];
char * APNBear = "internet.mts.ru";
//"AT+SAPBR=3,1,\"USER\",\"",
char ftpBearUserBuf[30];
char * UserBear = "mts";
//"AT+SAPBR=3,1,\"PWD\",\"",
char ftpBearPswBuf[30];
char * PasswordBear = "mts";
//"AT+FTPSERV=",		  
char ftpServBuf[30];

//char * servAddress = "81.23.151.1";	//from Home
char * servAddress = "31.13.128.67";		// on Work

//"AT+FTPUN=",		  
char ftpUnBuf[30];
char * UserName = "gsm";
//AT+FTPPW=\"
char ftpPswBuf[30];
char * Password = "qwe";
//"AT+FTPGETPATH=\"",
char ftpGetPathBuf[30];
char * Path = "/";
//"AT+FTPGETNAME=\"",
char ftpGetFileNameBuf[30];
char * FileName = "fw_tv2.bin";
//"AT+FTPTYPE=\"",
char ftpTypeBuf[30];
char * ftpType = "I";
//"AT+FTPGET=2,",
u8 iLen;
u16 nRead = FTP_PACKET_SIZE;
char ftpReadBuf[20];
char ftpReadNumber[5];

void ftpGSMPrepare(void)
{
   fdFTPCmds[0] = fdCheckConn;
   //fdFTPCmds[0].fDescState = stCheckConn;
   //fdFTPCmds[0].stTable = TbFunCheckConn;
   //fdFTPCmds[0].timeoutRepeat = 3;

   flagOnceSended = 0;
   //"AT+SAPBR=3,1,\"APN\",\"",
   memset(ftpBearAPNBuf,0,sizeof(ftpBearAPNBuf));
   strcpy(ftpBearAPNBuf,(char *)QUERIES[2]);
   strncat(ftpBearAPNBuf,APNBear,strlen(APNBear) );
   strncat(ftpBearAPNBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+SAPBR=3,1,\"USER\",\"",
   memset(ftpBearUserBuf,0,sizeof(ftpBearUserBuf));
   strcpy(ftpBearUserBuf,(char *)QUERIES[3]);
   strncat(ftpBearUserBuf,UserBear,strlen(UserBear) );
   strncat(ftpBearUserBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+SAPBR=3,1,\"PWD\",\"",
   memset(ftpBearPswBuf,0,sizeof(ftpBearPswBuf));
   strcpy(ftpBearPswBuf,(char *)QUERIES[4]);
   strncat(ftpBearPswBuf,PasswordBear,strlen(PasswordBear) );
   strncat(ftpBearPswBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPSERV=",		  
   memset(ftpServBuf,0,sizeof(ftpServBuf));
   strcpy(ftpServBuf,(char *)QUERIES[5]);
   strncat(ftpServBuf,servAddress,strlen(servAddress) );
   strncat(ftpServBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPUN=",		  
   memset(ftpUnBuf,0,sizeof(ftpUnBuf));
   strcpy(ftpUnBuf,(char *)QUERIES[6]);
   strncat(ftpUnBuf,UserName,strlen(UserName) );
   strncat(ftpUnBuf,"\"\r\n",sizeof("\"\r\n") );
   //AT+FTPPW=\"
   memset(ftpPswBuf,0,sizeof(ftpPswBuf));
   strcpy(ftpPswBuf,(char *)QUERIES[7]);
   strncat(ftpPswBuf,Password,strlen(Password) );
   strncat(ftpPswBuf,"\"\r\n",sizeof("\"\r\n") );
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
   //"AT+FTPTYPE=\"",
   memset(ftpTypeBuf,0,sizeof(ftpTypeBuf));
   strcpy(ftpTypeBuf,(char *)QUERIES[11]);
   strncat(ftpTypeBuf,ftpType,strlen(ftpType) );
   strncat(ftpTypeBuf,"\"\r\n",sizeof("\"\r\n") );
   //"AT+FTPGET=2,",
//   iLen = getLenNum(nRead);
//   int2char((char * )ftpReadNumber,nRead,iLen,10);
//   memset(ftpReadBuf,0,sizeof(ftpReadBuf));
//   strcpy(ftpReadBuf,(char *)QUERIES[14]);
//   strncat(ftpReadBuf,ftpReadNumber,strlen(ftpReadNumber) );
//   strncat(ftpReadBuf,"\r\n",sizeof("\r\n") );
   setTimerSend(TIME_FIRST_INI);
}

//-------------------------------------------------------------------------------------------

void TbFunCheckConn(void)  //AT
{
  static u8 flag = 0;
  if( (gsmStart) || (!flag) || (flagTimeout))
   {
     if(flagTimeout)
	  {
	    flagToSend = 1; 
		numTryTimeoutResendAT++;
	    flagTimeout = 0;
	  }
	 else
		numTryTimeoutResendAT = 0;
     cmdSend((u8 *)QUERIES[0], strlen((const char *)QUERIES[0]));
	 flag = 1;

	 gsmStart = 0;
	 //setTimerTimeout(TIMEOUT_USUAL_CMD);

	 timerFDTimeout = TIMEOUT_USUAL_CMD;
	 //led_mid(BOARD_LED_XOR);

   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stBearContype; //
		  currState += 1;
		  if(flagOnceSended)
		    currState = stBeginGPRS; 
	      flag = 0;
          
		  if(!newCycle)
		    setTimerSend(TIME_INTRV_CMDS);
		  else
		    setTimerSend(TIMEOUT_1MIN);
		  //gprsModemOff();   //for debugging
  		  //setTimerTimeout(0);
		}
    } 
  //should make timeout repeat cmd and no conn signal
   
}
//-------------------------------------------------------------------------------------------
void TbBearContype(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
     if(!checkTimeoutFun())
         cmdSend((u8 *)QUERIES[1], strlen((const char *)QUERIES[1]));
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stBearAPN; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		  //setTimerTimeout(0);
		}
    } 
}
//-------------------------------------------------------------------------------------------
void TbBearAPN(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
        cmdSend((u8 *)ftpBearAPNBuf, strlen((const char *)ftpBearAPNBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stBearUSER; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 

}
//-------------------------------------------------------------------------------------------
void TbBearUSER(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
        cmdSend((u8 *)ftpBearUserBuf, strlen((const char *)ftpBearUserBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stBearPWD; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 

}
//-------------------------------------------------------------------------------------------
void TbBearPWD(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
         cmdSend((u8 *)ftpBearPswBuf, strlen((const char *)ftpBearPswBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stServ; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}

//-------------------------------------------------------------------------------------------

void TbFunServ(void)
{
   
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
         cmdSend((u8 *)ftpServBuf, strlen((const char *)ftpServBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stUserName; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}

//-------------------------------------------------------------------------------------------
void TbFunUserName(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
        cmdSend((u8 *)ftpUnBuf, strlen((const char *)ftpUnBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stPass; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}
//-------------------------------------------------------------------------------------------
void TbFunPass(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
        cmdSend((u8 *)ftpPswBuf, strlen((const char *)ftpPswBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stGetPath; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}
//-------------------------------------------------------------------------------------------
void TbFunGetPath(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
         cmdSend((u8 *)ftpGetPathBuf, strlen((const char *)ftpGetPathBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stGetFileName; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}
//-------------------------------------------------------------------------------------------
void TbFunGetFileName(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
         cmdSend((u8 *)ftpGetFileNameBuf, strlen((const char *)ftpGetFileNameBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stType; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}
//-------------------------------------------------------------------------------------------
void TbFunType(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
         cmdSend((u8 *)ftpTypeBuf, strlen((const char *)ftpTypeBuf));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stBeginGPRS; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}
//-------------------------------------------------------------------------------------------
void TbFunMode(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
     if(!checkTimeoutFun())
         cmdSend((u8 *)QUERIES[10], strlen((const char *)QUERIES[10]));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stBeginGPRS; //
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}
    
//-------------------------------------------------------------------------------------------
void TbFunBeginGPRS(void)
{
  static u8 flag = 0;
  static u8 numTryBegin = 0;
  if( (!flag)  || (flagTimeout) )
   {
	 timerFDTimeout = TIMEOUT_BEGIN_GPRS_CMD;
     if(!checkTimeoutFun())
         cmdSend((u8 *)QUERIES[16], strlen((const char *)QUERIES[16]));
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stOpenFTP; //
		  flagOnceSended = 1;
		  newCycle = 0;
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
      else if((!strncmp((char *)REPLIES[1],(char *)&tempBuf[0],strlen((char *)REPLIES[1]))))  //ERROR
	    { 
	      //currState = stOpenFTP; //
		  flagOnceSended = 1;
	      if(numTryBegin++ >= RESEND_ERROR_RETRY)
			{
			  numTryBegin = 0;
			  newCycle = 1; 
			}
		  currState = stEndGPRS;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
   
}
//-------------------------------------------------------------------------------------------
void TbFunOpenFTP(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
     if(!checkTimeoutFun())
         cmdSend((u8 *)QUERIES[13], strlen((const char *)QUERIES[13]));
	 timerFDTimeout = TIMEOUT_START_FTP_CMD;
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[3],(char *)&tempBuf[0],strlen((char *)REPLIES[3]))))  //+FTPGET:1,1
	    { 
	      //currState = stFTPRead; //
		  flagOnceSended = 1;
		  newCycle = 0;
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_FTP_READ_INTRV);
		  flagOpenFTP = 1;
		  bytesFTPToReadForPacket = 0;
		}
      else if((!strncmp((char *)REPLIES[5],(char *)&tempBuf[0],strlen((char *)REPLIES[5]))))  //+FTPGET:1,64
	    { 
	      //Timeout happened
		  currState = stOpenFTP;
	      flag = 0;
          setTimerSend(TIME_FTP_READ_INTRV);
		}
    } 
}
//-------------------------------------------------------------------------------------------
void TbFunFTPRead(void)
{
   //should make MAX checking
  static u8 flag = 0;
  static u8 cntZeroBytesRead = 0;
  static u16 nReadPrev = 0;
  u8 tb[5];
  //u16 tempVal;
  	   //debug
//   strcpy((char *)tempBuf,(char *)REPLIES[5]);
//   if( (!strncmp((char *)REPLIES[4],(char *)&tempBuf[0],strlen((char *)REPLIES[4])))   &&  //+FTPGET:2,
//	 (!strncmp(ftpReadNumber,(char *)&tempBuf[strlen((char *)REPLIES[4])],strlen(ftpReadNumber))) )  
//			   flag = 9;

  if(!flag)
   {
     if(!updateFlag)
	   {
	     if(flagLastPage)
		  {
		    currState = stEndGPRS;
			flagLastPage = 0;
			flagOutBoot = 1;
		  }
		 else
		 {
		   nRead = FTP_PACKET_SIZE - bytesFTPToReadForPacket;

		   if(nRead != nReadPrev)  // if we received less bytes we asked than we should scan added (to pageSize) number of bytes
		     {
				   //"AT+FTPGET=2,",
				   memset(ftpReadNumber,0,sizeof(ftpReadNumber));
				   iLen = getLenNum(nRead);
				   int2char((char * )ftpReadNumber,nRead,iLen,10);
				   memset(ftpReadBuf,0,sizeof(ftpReadBuf));
				   strcpy(ftpReadBuf,(char *)QUERIES[14]);
				   strncat(ftpReadBuf,ftpReadNumber,strlen(ftpReadNumber) );
				   strncat(ftpReadBuf,"\r\n",sizeof("\r\n") );
				   //debug
				   //USART1SendBuf("new nRead:");
   				   int2char((char * )tb,nRead,4,10);
				 //USART1_SendByte(tb[0]);
				 //USART1_SendByte(tb[1]);
				 //USART1_SendByte(tb[2]);
				 //USART1_SendByte(tb[3]);
				 //USART1_SendByte(':');

			  }

			 nReadPrev = nRead;

		     //at+ftpget=2,1024
		     cmdSend((u8 *)ftpReadBuf, strlen((const char *)ftpReadBuf));
			 //USART1_SendByte('Q');USART1_SendByte(':');
			 flag = 1;
		 }
	   }
   }
  else if(cmdReceive())
    {
      if( (!strncmp((char *)REPLIES[4],(char *)&tempBuf[0],strlen((char *)REPLIES[4]))) ) //+FTPGET:2,
	    { 
		   // here  comes data  - we should read it and after that we wait 'OK'
		   bytesFTPRealRead = atoi((const char *)&tempBuf[strlen((char *)REPLIES[4])]);
		   //USART1Write("bytes:", sizeof("bytes:"));
		   //USART1Write(&tempBuf[strlen((char *)REPLIES[4])], strlen((const char *)&tempBuf[strlen((char *)REPLIES[4])]));
 		   //USART1_SendByte('\n');

		   if(bytesFTPRealRead)
		      {
		       flagReadRawData = 1;
			   cntZeroBytesRead = 0;
			  }
		   else
		       cntZeroBytesRead++;

	      //currState = stFTPClose; //
	      //flag = 0;
          //setTimerSend(TIME_INTRV_CMDS);
		}
      else if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 

	      currState = stFTPRead; //
		  if(flagPageDataReady)
		     {
			    flagPageDataReady = 0;
				//currState = stEndGPRS;
				//USART1_SendByte('D');USART1_SendByte(':');
				if(flagOpenFTP)	 // if there was just start FTP-session than it's first page
				 {
				  //flagOpenFTP = 0;
				  updateFlag = 1;
				 }
				else 
				  updateFlag = 2;
			 }
		  else if(cntZeroBytesRead > FTP_QTY_READ_TRY)
		      {
			    flagLastPage = 1;
			    cntZeroBytesRead = 0;
				updateFlag = 2; 
			  }

	      flag = 0;
          setTimerSend(TIME_FTP_READ_INTRV);
		  
		  //flagReadRawData = 0;
		  //setTimerSend(ONE_SEC*15);
		}

    }
 	 
}

//-------------------------------------------------------------------------------------------
void TbFunFTPClose(void) //not using yet - because return : "operation not allowed"
{

  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
     if(!checkTimeoutFun())
          cmdSend((u8 *)QUERIES[15], strlen((const char *)QUERIES[15])); //AT+FTPGET=2,0\r\n
	 timerFDTimeout = TIMEOUT_USUAL_CMD;
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      //currState = stEndGPRS; //
		  flagOnceSended = 1;
		  currState += 1;
	      flag = 0;
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
   
}
//-------------------------------------------------------------------------------------------
void TbFunEndGPRS(void)
{
  static u8 flag = 0;
  if( (!flag)  || (flagTimeout) )
   {
     if(!checkTimeoutFun())
         cmdSend((u8 *)QUERIES[17], strlen((const char *)QUERIES[17]));

	 timerFDTimeout = TIMEOUT_END_GPRS_CMD;
	 flag = 1;
   }
  else if(cmdReceive())
    {
      if((!strncmp((char *)REPLIES[0],(char *)&tempBuf[0],strlen((char *)REPLIES[0]))))  //OK
	    { 
	      currState = stCheckConn; //
		  flagOnceSended = 1;
	      flag = 0;
		  if(flagOutBoot)
		      go_cmd();
          setTimerSend(TIME_INTRV_CMDS);
		}
    } 
}

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
void loopFTP(void)
{

  stateTable[currState]();
  //fdCheckConn.stTable();
#if 1
  if(numTryTimeoutResendAT >= RESEND_TIMEOUT_RETRY)
     {
	   gprsModemOn();
	   numTryTimeoutResendAT = 0;
	   currState = stCheckConn;
	   setTimerSend(TIME_FIRST_INI);
	   flagToSend = 0;
	 }
#endif
#if 0
  if(numTryTimeoutResendCmd >= RESEND_TIMEOUT_RETRY)
     {
	   numTryTimeoutResendCmd = 0;
	   currState = stCheckConn;
	   setTimerSend(TIME_INTRV_CMDS);
	   flagToSend = 0;
	   newCycle = 1;
	 }

#endif
  if(flagToSend)
     {
	   flagToSend = 0;
       setTimerTimeout(timerFDTimeout);
       checkForSend();

	 }
}
//-------------------------------------------------------------------------------------------
void initializeSMFTP(void)
{
   currState = stCheckConn;
   //timer = 0.0;
}
//-------------------------------------------------------------------------------------------
u8 getLenNum(u16 num)
{
	u8 i = 5;
	if(num < 10)         i = 1;
	else if(num < 100)   i = 2;
	else if(num < 1000)  i = 3;
	else if(num < 10000) i = 4;
	return i;
}

//-------------------------------------------------------------------------------------------

int int2char(char * pbuf, int data, const char field_size, const char base)
{
  u8 i;
  pbuf += field_size;
  
  for (i = 0; i<field_size; i++)
  {
    if (base == 10)
    {
      *(--pbuf) = data%10+'0';
      data = data/10;
    }
    else if (base==16)
    {
      *(--pbuf) = hex2char(data);
      data = data>>4;
    }
    else
      return -2;
  }
  return 1;
}
//-------------------------------------------------------------------------------------------
int hex2char(int _C)
{
  int x = _C & 0x0f;
  if(x >= 0 && x <= 9)
    return x+'0';
  else
    return x-10+'A';
}
// ------------------------------------------------------------------------------
// *************************************TIMERS**********************************
// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
void handleTimerSendGSM(void)
{
    if(timerValueSend)
	  {
	   timerValueSend--;
	   if(!timerValueSend )
	      flagToSend = 1;
	  }
}
// ------------------------------------------------------------------------------
void setTimerSend(u32 val)
{
   	timerValueSend = val;
    setTimerTimeout(0);
}

// ------------------------------------------------------------------------------
void handleTimerTimeoutGSM(void)
{
    if(timerValueTimeout)
	  {
	   timerValueTimeout--;
	   if(!timerValueTimeout )
	    {
	      flagTimeout = 1;

		}
	  }
}
// ------------------------------------------------------------------------------
void setTimerTimeout(u32 val)
{
   	timerValueTimeout = val;
}

// ------------------------------------------------------------------------------
// return 1 - if timeout happened > 3 times, 0 - no timeout
// ------------------------------------------------------------------------------
u8 checkTimeoutFun(void)
{
     u8 result = 0;
     if(flagTimeout)
	  {
		if(++numTryTimeoutResendCmd >= RESEND_TIMEOUT_RETRY)
		   {
		       numTryTimeoutResendCmd = 0;
		       currState = stCheckConn;
			   //setTimerSend(TIME_INTRV_CMDS);
			   //flagToSend = 0;
			   newCycle = 1;
			   result = 1;
		   }
	    //else
		flagToSend = 1; 
	    flagTimeout = 0;
	  }
	 else
		numTryTimeoutResendCmd = 0;

	 return result;
}
// ------------------------------------------------------------------------------


//   if(numTryTimeoutResendCmd >= RESEND_TIMEOUT_RETRY)
//     {
//	   numTryTimeoutResendCmd = 0;
//	   currState = stCheckConn;
//	   setTimerSend(TIME_INTRV_CMDS);
//	   flagToSend = 0;
//	   newCycle = 1;
//	 }




















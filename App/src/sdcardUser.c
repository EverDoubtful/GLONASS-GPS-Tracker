/******************** (C) COPYRIGHT 2011  ********************
* File Name          : 
* Author             : 
* Version            : 
* Date               : 
* Description        : 
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sdcardUser.h"
#include "ff.h"
#include "dio_led.h"
#include "string.h"
#include "common.h"
#include "hand_conf.h"
#include "bkp.h"
#include "hw_config.h"
#include "sdSDIODrv.h"
#include "rtc.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NULL                  0
#define YEAR_ADD              1980   //1920
#define MAX_SIZE_READ_BLOCK   1023
#define MAX_SIZE_WRITE_BLOCK  1047

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//----------------------FATFS------------------------
FATFS FAT_SDIO;		/* File system object */
FIL   fileAsciiLog;			/* File object */
FIL   fileNotSended;
const u8 * fnSavedInfo      = "day_XX.txt";         //fn = filename

/* binary files*/
static FIL   fileData;
#define NR_REC_PER_SUTKI         50   //17280  /*  Number of points to store for 1 sutki(uslovno) 12packets/1min * 60 * 24 */
/*  Number of points to store for 1 sutki(uslovno) 12packets/1min * 60 * 24 plus reserv */
#define NR_REC_PER_SUTKI_PLUS_RESERV      (NR_REC_PER_SUTKI + NR_REC_PER_SUTKI * 0.2)  

u32   indexNR_W,  byteWritten;
//u32   indexPosWrIndexPtr, indexPosReadIndexPtr;  /* pointers(usual numbers in this case) to write end of file and read begin of file with wait info*/
u32   indexPosWrDataPtr, indexPosReadDataPtr;  /* pointers(usual numbers in this case) to write end of file and read begin of file with wait info*/

#define SIZE_OF_ONE_RECORD_INDEX  2 * sizeof(u32)
//u32   savedNr;  //may be later should make for searching and so on(last 24 hours - then we'll make circle moving in file)
static u32 bufIndexData[2];
//const u8 * fnIndexBin = "index.bin";    //fn = filename
const u8 * fnDataBin  = "wait.bin";
/* info file with log*/
static FIL   fileBinLog;
const  u8 *  fnFileLog = "day_XX.bin";
u8  dirName[9];
u8  yearName[4];
u32          indexFLLen;
u8           bufIndexFLLen[5]; // len (4) + type(1)
u8           typeRecord;
#define SIZE_LENGTH_BLOCK  sizeof(u32) + sizeof(u8)  

FRESULT resultFAT;				/* Result code */

//u8  flagBusySD;
u32 badPacketPtr;
u8  flagNSOpened;    //flag file NOTSEND.txt opened
u8  flagAlrAskOpenForRead;
u8  fileName[13];
u8  curDirName[30];
//u8  fNoNeedToRead = 0;
u8 flagDirYAlreadyCreated = 0;
u8 flagDirMAlreadyCreated = 0;
u8 flagDirWAlreadyCreated = 0;

const u8 * events[] = {
                            "TURN ON BLOCK",
							"GPRS ON",
							"GPRS OFF",
							"GPS ON",
							"GPS OFF"

					  };
//extern u8 debSign;
//----------------------END OF FATFS------------------------
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void checkFileDir(u8 flags); 
//-------------------------------------------------------------------------------------------
void initSD(void)
{
  f_mount(0, &FAT_SDIO);
}
/*******************************************************************************/
/* READING INFO For retransmitting(packets that we couldn't send)           
                                                          */
/*******************************************************************************/
u16 readSDInfo(u8 * bufExt, u16 len)	   
{
   //if (debSign)
   u16 scanLen = 0;
   //if(SD_Detect()== SD_PRESENT)
   //{

   u8  flagChProt = SD_ASCII_PROTOCOL;
   if(getProtocol() == PROTOCOL_BINARY)
	       flagChProt = SD_BINARY_PROTOCOL;
   checkFileDir(SD_READ | flagChProt );

   if(flagNSOpened)
	     {
		    /* ACTUALLY READING INFO */
			  /* Binary */
	         if(flagChProt & SD_BINARY_PROTOCOL)  
	          {
				 if(indexPosReadDataPtr != indexPosWrDataPtr)
				   {
				     resultFAT = f_chdir((const TCHAR *)"0:/WAIT");
					 if(f_size(&fileData))  /*if file more than 0*/
					   {
						  /* BLOCK OF READING INFO */ 
			             resultFAT = f_lseek(&fileData, indexPosReadDataPtr);
						 if(f_eof(&fileData))  /*it's the end of the file*/
						     indexPosReadDataPtr = 0;
						 if(resultFAT == FR_OK)
							   {
				                  resultFAT = f_read(&fileData, bufIndexData, SIZE_OF_ONE_RECORD_INDEX , &byteWritten );
								  indexPosReadDataPtr += bufIndexData[1];
								  //scanLen = bufIndexData[1] - SIZE_OF_ONE_RECORD_INDEX;
								   //tempora
							      //if((bufIndexData[1] > MAX_SIZE_READ_BLOCK) || (bufIndexData[1] > len) ) /* Protection, in other case trying to scan 1024 bytes and more will hang system*/
								//	   bufIndexData[1] = MIN(MAX_SIZE_READ_BLOCK, len);
								  if(indexNR_W)
								      indexNR_W--;

								  //indexNR_R++;
				                  resultFAT = f_read(&fileData, bufExt, bufIndexData[1] - SIZE_OF_ONE_RECORD_INDEX, &byteWritten );
								  scanLen = byteWritten;
			                      BKPWriteReg(BKP_DR_INDEX_POSREAD_N1,indexPosReadDataPtr);
					              BKPWriteReg(BKP_DR_INDEX_POSREAD_N2,indexPosReadDataPtr >> 16);

//							      USBWriteStr("READ:");
//							      USBDebWrite(indexPosReadDataPtr);
//							      USBWriteStr("\r\n");

							   }
						   
						}
				   }
				 else	/*indexPosReadDataPtr = indexPosWrDataPtr - means readPtr became equal WrPtr*/
				   {
					 indexPosReadDataPtr = 0;
					 indexPosWrDataPtr   = 0;
					 indexNR_W = 0;
                     BKPWriteReg(BKP_DR_INDEX_POSWR_N1,indexPosWrDataPtr);
		             BKPWriteReg(BKP_DR_INDEX_POSWR_N2,indexPosWrDataPtr >> 16);
                     BKPWriteReg(BKP_DR_INDEX_POSREAD_N1,indexPosReadDataPtr);
		             BKPWriteReg(BKP_DR_INDEX_POSREAD_N2,indexPosReadDataPtr >> 16);

				   }
	           }
		 }
    
	//}
	return scanLen;
    	  
}

/*******************************************************************************/
/* 	flags: 
    1. point sended or wait						   */
/*******************************************************************************/
void saveSDInfo(u8 * bufExt,  u16 nSize, u8 flags, sdTypeMsg typeMsg)	   // 
{
   u32 temp;
   //if(SD_Detect()== SD_PRESENT)
   //{
   u8  flagChProt = SD_ASCII_PROTOCOL;
   if(getProtocol() == PROTOCOL_BINARY)
       flagChProt = SD_BINARY_PROTOCOL;
   flags |= flagChProt;
   checkFileDir(SD_WRITE | flags);

   if(flags & SD_SENDED)  /* sended */
	 {
			if(flags & SD_BINARY_PROTOCOL)  /* binary */
			  {
				  indexFLLen    = nSize + SIZE_LENGTH_BLOCK;
				  if(typeMsg == SD_TYPE_MSG_LOG)
				    {
				      temp = rtcGetRealTime2();
					  indexFLLen += 4;
					}
			      *(u32 *)bufIndexFLLen  = indexFLLen;
				  bufIndexFLLen[4] = typeMsg;
				  resultFAT = f_chdir((const TCHAR *)"0:/");
				  resultFAT = f_chdir((const TCHAR *)yearName);
				  resultFAT = f_chdir((const TCHAR *)dirName);
                  /* Fill by length of block(4bytes) -  Move to end of the file to append data */
	              resultFAT = f_lseek(&fileBinLog, f_size(&fileBinLog));
				  if(resultFAT == FR_OK)
				    {
		              resultFAT = f_write(&fileBinLog, bufIndexFLLen,SIZE_LENGTH_BLOCK,&byteWritten );
		              /* Fill by INFO -  Move to end of the file to append data */
					  //if( nSize > MAX_SIZE_WRITE_BLOCK )  //tempora
					  //   nSize = MAX_SIZE_WRITE_BLOCK;
		              resultFAT = f_write(&fileBinLog, bufExt, nSize ,&byteWritten );
					  if(typeMsg == SD_TYPE_MSG_LOG)
					    {
		                   resultFAT = f_write(&fileBinLog, &temp, 4 ,&byteWritten );
						}
		              f_sync(&fileBinLog);
				    }
			  }

      }
    else if(flagNSOpened)                  /* wait */
      {
        if(flags & SD_BINARY_PROTOCOL)  /* binary */
          {
  		     resultFAT = f_chdir((const TCHAR *)"0:/WAIT");
			 if( (indexNR_W  ) < NR_REC_PER_SUTKI)
			   {
			     /* BLOCK OF WRITING INFO */
	              bufIndexData[0] = indexNR_W;
	              bufIndexData[1] = nSize + SIZE_OF_ONE_RECORD_INDEX;
				  indexNR_W++;
	              /* Fill by INFO -  Move to pointer of the  file to append data */
	              resultFAT = f_lseek(&fileData, indexPosWrDataPtr);
				  if(resultFAT == FR_OK)
				    {
					  /* write 8 byte - number of record and len of block*/
		              resultFAT = f_write(&fileData, bufIndexData,SIZE_OF_ONE_RECORD_INDEX,&byteWritten );
					  
					  if( nSize > MAX_SIZE_WRITE_BLOCK )
						     nSize = MAX_SIZE_WRITE_BLOCK;
					  /*Write data*/
		              resultFAT = f_write(&fileData, bufExt, nSize ,&byteWritten );
	
					  if((byteWritten) && (resultFAT == FR_OK))
		                 f_sync(&fileData);
				      indexPosWrDataPtr   += (nSize + SIZE_OF_ONE_RECORD_INDEX);
			          BKPWriteReg(BKP_DR_INDEX_POSWR_N1,indexPosWrDataPtr);
			          BKPWriteReg(BKP_DR_INDEX_POSWR_N2,indexPosWrDataPtr >> 16);

				      //USBWriteStr("s1:");
				      //USBDebWrite(indexPosWrDataPtr);
				      //USBWriteStr("\r\n");

				    }
				}
			  else if( (indexNR_W  ) < NR_REC_PER_SUTKI_PLUS_RESERV)
			    {
			     /* BLOCK OF WRITING INFO */
	              bufIndexData[0] = indexNR_W;
	              bufIndexData[1] = nSize + SIZE_OF_ONE_RECORD_INDEX;
				  indexNR_W++;
	              /* Fill by INFO -  Move to pointer of the  file to append data */
	              resultFAT = f_lseek(&fileData, indexPosWrDataPtr);
				  if(resultFAT == FR_OK)
				    {
					  /* write 8 byte - number of record and len of block*/
		              resultFAT = f_write(&fileData, bufIndexData,SIZE_OF_ONE_RECORD_INDEX,&byteWritten );
					  if( nSize > MAX_SIZE_WRITE_BLOCK )
						     nSize = MAX_SIZE_WRITE_BLOCK;
					  /*Write data*/
		              resultFAT = f_write(&fileData, bufExt, nSize ,&byteWritten );
	
					  if((byteWritten) && (resultFAT == FR_OK))
		                 f_sync(&fileData);
				      indexPosWrDataPtr   += (nSize + SIZE_OF_ONE_RECORD_INDEX);
			          BKPWriteReg(BKP_DR_INDEX_POSWR_N1,indexPosWrDataPtr);
			          BKPWriteReg(BKP_DR_INDEX_POSWR_N2,indexPosWrDataPtr >> 16);

				      //USBWriteStr("s2:");
				      //USBDebWrite(indexPosWrDataPtr);
				      //USBWriteStr("\r\n");

				    }
				  /* BLOCK OF READING INFO */ 
	              resultFAT = f_lseek(&fileData, indexPosReadDataPtr);
				  if(resultFAT == FR_OK)
					{
					  //indexNR_R++;
	                  resultFAT = f_read(&fileData, bufIndexData, SIZE_OF_ONE_RECORD_INDEX , &byteWritten );
					  indexPosReadDataPtr += bufIndexData[1];
                      BKPWriteReg(BKP_DR_INDEX_POSREAD_N1,indexPosReadDataPtr);
		              BKPWriteReg(BKP_DR_INDEX_POSREAD_N2,indexPosReadDataPtr >> 16);

				      //USBWriteStr("r2:");
				      //USBDebWrite(indexPosReadDataPtr);
				      //USBWriteStr("\r\n");

					}

			    }
			  else	 /* (indexNR_W = NR_REC_PER_SUTKI_PLUS_RESERV) */
			    {
			      /* BLOCK OF WRITING INFO */
	              bufIndexData[0] = indexNR_W;
	              bufIndexData[1] = nSize + SIZE_OF_ONE_RECORD_INDEX;
	              /* Fill by INFO -  Move to pointer of the  file to append data */
				  //if(indexPosWrDataPtr > indexPosReadDataPtr)
	              resultFAT = f_lseek(&fileData, indexPosWrDataPtr);
 				  if(f_eof(&fileData))  /*it's the end of the file*/
				     {
				       indexPosWrDataPtr = 0;
	                   resultFAT = f_lseek(&fileData, indexPosWrDataPtr);
//					   USBWriteStr("-----Write-END-----");
//				       USBWriteStr("\r\n");
					 }
				  if(resultFAT == FR_OK)
				    {
					  /* write 8 byte - number of record and len of block*/
		              resultFAT = f_write(&fileData, bufIndexData,SIZE_OF_ONE_RECORD_INDEX,&byteWritten );
					  //if( nSize > MAX_SIZE_WRITE_BLOCK )
					  //	     nSize = MAX_SIZE_WRITE_BLOCK;
		              resultFAT = f_write(&fileData, bufExt, nSize ,&byteWritten );
	
					  if((byteWritten) && (resultFAT == FR_OK))
		                 f_sync(&fileData);
				      indexPosWrDataPtr   += (nSize + SIZE_OF_ONE_RECORD_INDEX);
			          BKPWriteReg(BKP_DR_INDEX_POSWR_N1,indexPosWrDataPtr);
			          BKPWriteReg(BKP_DR_INDEX_POSWR_N2,indexPosWrDataPtr >> 16);

//				      USBWriteStr("s3:");
//				      USBDebWrite(indexPosWrDataPtr);
//				      USBWriteStr(" len:");
//				      USBDebWrite(bufIndexData[1]);
//				      USBWriteStr("\r\n");

				    }
				  /* BLOCK OF READING INFO */ 
	              resultFAT = f_lseek(&fileData, indexPosReadDataPtr);
 				  if(f_eof(&fileData))  /*it's the end of the file*/
				    {
					     indexPosReadDataPtr = 0;
	                     resultFAT = f_lseek(&fileData, indexPosReadDataPtr);
//					     USBWriteStr("---READ-END-----");
//				         USBWriteStr("\r\n");

					}
				  if(resultFAT == FR_OK)
					{
					  //indexNR_R++;
	                  resultFAT = f_read(&fileData, bufIndexData, SIZE_OF_ONE_RECORD_INDEX , &byteWritten );
					  indexPosReadDataPtr += bufIndexData[1];
                      BKPWriteReg(BKP_DR_INDEX_POSREAD_N1,indexPosReadDataPtr);
		              BKPWriteReg(BKP_DR_INDEX_POSREAD_N2,indexPosReadDataPtr >> 16);

//				      USBWriteStr("r3:");
//				      USBDebWrite(indexPosReadDataPtr);
//				      USBWriteStr(" len:");
//				      USBDebWrite(bufIndexData[1]);
//				      USBWriteStr("\r\n");

					}
				   
				}
          }
     }
   //}

}
/*******************************************************************************/
/* flags (2 variants): protocol + if write then send|notsend    
		               protocol + read			   */
/*******************************************************************************/

void checkFileDir(u8 flags) //
{
	static u8 flagFileAlreadyCreated = 0;
	
	static u8 dayCur = 0;
	static u8 dayPrev = 0;
	static u8 monCur = 0;
	static u8 monPrev = 0;
	static u16 yearCur = 0;
	static u16 yearPrev = 0;

	u32 time;
	//u8  fileName[13];
	time = get_fattime();
	dayCur = (time >> 16) & 0x1F;
	monCur = (time >> 21) & 0x0F;
    yearCur = ((time >> 25) & 0xfF) + YEAR_ADD;
 

	if(monCur != monPrev)
	  {
		monPrev = monCur;
		/*  Up one level  */
		if(flagDirMAlreadyCreated)
		  {
			flagDirMAlreadyCreated = 0;
			resultFAT = f_chdir("..");	    
	      }
		if(yearCur != yearPrev)
		  {
			yearPrev = yearCur;
			/*  Up one level  */
			if(flagDirYAlreadyCreated)
			  {
				flagDirYAlreadyCreated = 0;
				resultFAT = f_chdir("..");	    
		      }
		  }
	  }
	  /* Make year dirname */
	if(!flagDirYAlreadyCreated)         
	  {
	    memset(yearName,0,sizeof(yearName));
	    flagDirYAlreadyCreated = 1;
		int2char((char * )yearName,yearCur,4,10);  //year
		resultFAT = f_mkdir((const TCHAR *)yearName);	
	    resultFAT = f_chdir((const TCHAR *)yearName); /* go inside year directory*/	 
	  }	
	  /* Make month dirname */
	if(!flagDirMAlreadyCreated)         
	  {
	    memset(dirName,0,sizeof(dirName));
	    flagDirMAlreadyCreated = 1;
        int2char((char * )dirName,monCur,2,10);  //year
		resultFAT = f_mkdir((const TCHAR *)dirName);	
	    resultFAT = f_chdir((const TCHAR *)dirName); /* go inside year directory*/		
	  }	
	  /* Make WAIT dirname */
	if(!flagDirWAlreadyCreated)         
	  {
	    flagDirWAlreadyCreated = 1;
		resultFAT = f_mkdir((const TCHAR *)"0:/WAIT");	
	  }	
			 

   
	//if(flags & SD_WRITE)		 //write
	//  {
	    /* MAKING SPACE FOR SENDED INFO */
	    if(flags & SD_SENDED)
	      {
			if(dayCur != dayPrev)	 /*  Close previous file  */
			  {
				dayPrev = dayCur;
				if(flagFileAlreadyCreated)
				  {
				    if(flags & SD_BINARY_PROTOCOL)	/* Binary */
					  resultFAT = f_close(&fileBinLog);
					flagFileAlreadyCreated = 0;
			      }
			  }
			if(!flagFileAlreadyCreated)	/* Make filename*/
			  {
			    flagFileAlreadyCreated = 1;
				if(flags & SD_BINARY_PROTOCOL)	 /* Binary */
				  {
				    strcpy((char *)fileName,(char *)fnFileLog);
					int2char((char * )&fileName[4],dayCur,2,10);
					resultFAT = f_open(&fileBinLog, (const TCHAR *)fileName, FA_OPEN_ALWAYS | FA_WRITE);	 //
				  }
			  }
	   	   }
	       /*MAKING SPACE FOR DELAYED(NOT SENDED) INFO*/	   	
	     else if(!flagNSOpened)
			  {
	             /* Binary */
	            if(flags & SD_BINARY_PROTOCOL)
	             {
					resultFAT = f_chdir((const TCHAR *)"0:/WAIT");
	    		    strcpy((char *)fileName,(char *)fnDataBin);
	     	        resultFAT = f_open(&fileData, (const TCHAR *)fileName, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
					if(resultFAT == FR_OK)
					  {
						if(!f_size(&fileData))  /*if file just created and its size = 0*/
						   {
						     indexPosWrDataPtr = 0;
						     indexPosReadDataPtr = 0;
							 indexNR_W  = 0;
				 			 //indexNR_R = 0;
		                     BKPWriteReg(BKP_DR_INDEX_POSWR_N1,indexPosWrDataPtr);
				             BKPWriteReg(BKP_DR_INDEX_POSWR_N2,indexPosWrDataPtr >> 16);
		                     BKPWriteReg(BKP_DR_INDEX_POSREAD_N1,indexPosReadDataPtr);
				             BKPWriteReg(BKP_DR_INDEX_POSREAD_N2,indexPosReadDataPtr >> 16);
							 //USBWriteChar('N');
						   }
						else	/*if it was already existed*/
						   {
	  						 indexPosWrDataPtr   = BKPReadReg(BKP_DR_INDEX_POSWR_N1)  | ( (u32)(BKPReadReg(BKP_DR_INDEX_POSWR_N2) << 16) );
	  						 indexPosReadDataPtr = BKPReadReg(BKP_DR_INDEX_POSREAD_N1)  | ( (u32)(BKPReadReg(BKP_DR_INDEX_POSREAD_N2) << 16) );
							 //USBWriteChar('A');
						   }
	    		        flagNSOpened = 1;
					   }
	             }
		      }	 //if(!flagNSOpened)
       //}    //if(flags & SD_WRITE)
}

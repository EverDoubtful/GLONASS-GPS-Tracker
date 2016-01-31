/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : gps.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : This file provides a set of functions needed to manage the
*                      communication between SPI peripheral and SPI M25P64 FLASH.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "protocol_bin.h"
#include <string.h>
#include "bin_handle.h"
#include "crc.h"
#include "menu.h"
#include "hand_conf.h"
#include "flash.h"
#include "hw_config.h"
#include "ais326dq.h"
#include "protocol_ascii.h"
#include "gps.h"
#include "packets.h"
#include "gsm.h"
#include "fifoPackets.h"
#include "hand_conf.h"
#include "dio_led.h"
#include "adc.h"
#include "rtc.h"
#include "sdcardUser.h"
#include "gprs_modem.h"   //debug
#include "gsmInner.h"            //debug
extern u8 chooseProtocol;
//extern u8 bufRxOperGSM[MAX_SIZE_PACKET_GPRS_SIM900];
extern u8 bufRxOperGSM[];	//SIZE_PACKET_GPRS_OPER
	 
extern fifo_packet   fifoRecBufPack;
extern configSet config;
extern t_innerState innerState;
extern fifo_packet fifoNaviPack;

//u16 readSDBlock( u16 len );   //debugga
//u32 saveSDBlock(  u16 nSize );

u8 bufUnstaffedTmp[SIZE_PACKET_GPRS_OPER];

/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u16 stuffData(u8 * buf, u16 nSize)
{
  u16 nCountMoveMem=nSize, i;
  for(i=0; i < nSize; ++i, --nCountMoveMem)
  {
    switch (buf[i])
    {
      case '@':
      case '$':
      case '+':
      case 0x0A:
      case 0x0D:
      case 0xFE:
      {
        memmove(&buf[i+1], &buf[i], nCountMoveMem);
        buf[i++] = 0xFE;
        ++nSize;
      }
    }
  }
  return nSize;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u16  unstuffData(u8 * bufSrc, u16 nSize)
{
  static u8 bByteStaff = 0, bPrefixPack = 0;
  u16 oldIndex = 0;
  u16 newIndex = 0;
  u8  bGotPacket = 0;
  static u16 savedIndex = 0;
  // Анализирование получения данных 
  while(nSize--)
  {
    switch (bufSrc[oldIndex])
    {
      // Обработка байт-стафигна       
      case 0xFE:
      {
        if (bByteStaff)
         {
          bByteStaff = 0;
          if (bPrefixPack)
           {
			 bufSrc[newIndex++] = bufSrc[oldIndex];
           }
         }
        else
         {
          bByteStaff = 1; 
         }
      }
      break;
      // Обработка префикса
      case '@':
      {
        if (bByteStaff)
        {
          bByteStaff = 0;
          if (bPrefixPack)
           {
			 bufSrc[newIndex++] = bufSrc[oldIndex];
           }
        }
        else
        {
          // Получен префикс пакета
          bPrefixPack = 1;
        }
      }
      break;
      // Обработка постфикса
      case '$':
      {
        if (bByteStaff)
        { 
          bByteStaff = 0;
          if (bPrefixPack)
           {
			 bufSrc[newIndex++] = bufSrc[oldIndex];
           }
        }
        else
        {
          // Получен постфикс пакета
          bPrefixPack = 0;
		  //fifoPacketPut(&fifoRecBufPack,bufSrc, newIndex);
		  //newIndex = 0;
		  if(savedIndex == 0)  //it's one packet(not separated)
		     fifoPacketPut(&fifoRecBufPack,bufSrc, newIndex,0);
		  else   //it's complicated packet
		    {
       		  memcpy( &bufUnstaffedTmp[savedIndex],bufSrc,newIndex);	//add data and finish packet
			  savedIndex += newIndex;
		      fifoPacketPut(&fifoRecBufPack,bufUnstaffedTmp, savedIndex,0);
			  memset(bufUnstaffedTmp,0,sizeof(bufUnstaffedTmp));
			  savedIndex = 0;
			  //USBWriteStr("2P\r\n");
			}
		  bGotPacket = 1;
        } 
      }
	  break;
      case 0x0A:
	  case 0x0D:
	  case '+':
      {
        if (bByteStaff)
        {
          bByteStaff = 0;
          if (bPrefixPack)
           {
 			 bufSrc[newIndex++] = bufSrc[oldIndex];
           }
        }
      }
      break;
      default:
      {
        bByteStaff = 0;
        if (bPrefixPack)
          {
		    bufSrc[newIndex++] = bufSrc[oldIndex]; 
          } 
      }
    } //switch
	 oldIndex++;
  } //while
  if(!bGotPacket)  //there was not packet
    {
       memcpy( &bufUnstaffedTmp[savedIndex],bufSrc,newIndex);
	   savedIndex += newIndex;
	   //USBWriteStr("1P\r\n");
	}
  return newIndex;
}
/*******************************************************************************/
/* 	flags: bit0: 0 - self initiative, 1- reply		GSM	
           bit1: 0 - don't save on SD, 1 - save on SD instead of sending on GSM */	  
/*******************************************************************************/
u16  prepareForSend(u8 * bufSrc, u16 nSize, u8 flags, u8 cmd, u8 nrLong)
{
    u8 * pBuf = bufSrc;
    u16 lenTemp = nSize;
	u16 index = 0;
	//u8 res = 0;
    cmd_info wrap;
	if((flags & 0x01) == 0)
	  {
		u8  temp = SIZE_WRAP_BLOCKID + SIZE_WRAP_CMD + SIZE_WRAP_AUX + SIZE_WRAP_LEN;
		if(nSize)
		   memmove(&bufSrc[temp],bufSrc,nSize);
	  }
    /* Insert UID of the block*/
    *(u32*)pBuf = config.idBlock;
    pBuf     += SIZE_WRAP_BLOCKID;
    lenTemp  += SIZE_WRAP_BLOCKID;
    /* Insert CMD of the answer */
	wrap.cmd = cmd;
    *pBuf  = wrap.cmd;
    pBuf     += SIZE_WRAP_CMD;
    lenTemp  += SIZE_WRAP_CMD;

    /* Insert AUX byte of the answer*/
	wrap.aux.reply  = AUX_SELF_INTV;
	if(flags & 0x01)
	   wrap.aux.reply  = AUX_REPLY;
	wrap.aux.cypher = AUX_OPENED;
	wrap.aux.longPack = AUX_SHORT;
    if(nrLong)
	  {
	   wrap.aux.longPack = AUX_LONG;
	   memmove(&bufSrc[9],&bufSrc[8],nSize);
	   bufSrc[8] = nrLong;
	   nSize += 1;
	   lenTemp += 1;
	  }
    *pBuf  = *(u8 *)&wrap.aux;
    pBuf     += SIZE_WRAP_AUX;
    lenTemp  += SIZE_WRAP_AUX;
    /* Insert Length of the block of pure data (without cmd, id, crc ...)*/
	wrap.len = nSize;
    *(u16*)pBuf  = wrap.len;
    pBuf     += SIZE_WRAP_LEN;
    lenTemp  += SIZE_WRAP_LEN;
   /* Add CRC to the trail of the packet, crc is calculating on whole packet*/
    wrap.crc = calcBlock(bufSrc, lenTemp);
    //wrap.crc = 0x10040502;
    //*(u32*)&bufSrc[lenTemp]  = wrap.crc;
	//bufSrc += temp + nSize;
	pBuf     += nSize;
	
    *(u32*)pBuf  = wrap.crc;

    //bufSrc   += sizeof(wrap.crc);
    lenTemp  += SIZE_WRAP_CRC;
	/* Stuffing and so on*/
    lenTemp = stuffData(bufSrc, lenTemp);
    memmove(&bufSrc[1],&bufSrc[0],lenTemp);
    bufSrc[0] = 0x40;
    lenTemp++;
    bufSrc[lenTemp++] = 0x24;
	/* Here comes check on max length of SIM900 packet(1360 bytes)*/
	while(lenTemp > (MAX_SIZE_PACKET_GPRS_SIM900))
	  {
	    /* put handled packet into the send packet buffer*/
	  	fifoPacketPut(&fifoNaviPack,&bufRxOperGSM[index], (MAX_SIZE_PACKET_GPRS_SIM900),0);
		lenTemp -= (MAX_SIZE_PACKET_GPRS_SIM900 );
		index += ( MAX_SIZE_PACKET_GPRS_SIM900 );
	  }
	/* put handled packet into the send packet buffer*/
	if(!(flags & 0x02))
		fifoPacketPut(&fifoNaviPack,&bufRxOperGSM[index], lenTemp,0);
	else if(flags & 0x02)
		saveSDInfo(&bufRxOperGSM[index], lenTemp, SD_NOTSEND, SD_TYPE_MSG_GSM);
//    if(innerState.flagDebug)
//	 {
//	  USBWriteStr("cmd:");
//	  USBDebWrite(wrap.cmd);
//	  USBWriteStr("\r\n");
//	 }
    return lenTemp;
}
/*******************************************************************************/
/* 																			   */
/*******************************************************************************/
u16  prepareForSendUSB(u8 * bufSrc, u16 nSize, u8 nrLong)
{
    u16 lenTemp;
    cmd_info wrap;
	u8 * pBuf;

    bufSrc -= 5;
	pBuf = bufSrc;
	lenTemp = SIZE_HEADER_PACKET; 

    pBuf     += SIZE_WRAP_PIL + SIZE_WRAP_CMD;
    /* Insert AUX byte of the answer*/
	wrap.aux.reply  = AUX_REPLY;
	wrap.aux.cypher = AUX_OPENED;
	wrap.aux.longPack = AUX_SHORT;
    if(nrLong)
	  {
	   wrap.aux.longPack = AUX_LONG;
	   memmove(&bufSrc[6],&bufSrc[5],nSize);
	   bufSrc[5] = nrLong;
	   nSize += 1;
	  }
	wrap.aux.intrf = INTRF_USB;
    *pBuf     = *(u8 *)&wrap.aux;
    pBuf     += SIZE_WRAP_AUX;
    /* Insert Length of the block of pure data (without cmd, id, crc ...)*/
	wrap.len = nSize;
    *(u16*)pBuf  = wrap.len;
    pBuf   += SIZE_WRAP_LEN;
    /*Add DATA Already there*/
    pBuf    += nSize;
	lenTemp += nSize; 
    /* Add CRC to the trail of the packet, crc is calculating on whole packet*/
    wrap.crc = calcBlock(bufSrc, lenTemp );  // (-4 CRC) 
    *(u32*)pBuf  = wrap.crc;
	pBuf   += SIZE_WRAP_CRC;
	lenTemp += SIZE_WRAP_CRC;

    return lenTemp;
}

/*******************************************************************************/
/* 	get unstuffed data	and execute from GSM					           */
/*******************************************************************************/
u16 getCmd(u8 * buf)
{
    u16 len = 0;
    cmd_info wrap;
	u8 nrLongPack = 0;
	u16 limit = 2000;  //exact

    buf  += SIZE_WRAP_BLOCKID;

    wrap.cmd  = *buf;               //cmd
    buf  += SIZE_WRAP_CMD;

    buf  += SIZE_WRAP_AUX;  //aux

    wrap.len = *(u16*)buf;   //length
    buf  += SIZE_WRAP_LEN;
    
    /*Executing commands and handling data*/
    switch(wrap.cmd)
     {
	   case  CMD_ALL_INFO        :     len = genInfo(buf, 1);    		  break;
       case  CMD_READ_CONFIG     :     len = menuDisplay(buf, &nrLongPack, limit);            break;
       case  CMD_WRITE_CONFIG    :     len = writeConfig(wrap.len, buf ); break;
       case  CMD_PSW_CHECK       :     len = checkPsw( buf );    		  break;
       case  CMD_BL_ENTER        :     len = blEnter();                   break;
       case  CMD_BL_PORTION      :     len = addPortion(wrap.len, buf);   break;
       case  CMD_BL_ADD_PAGE     :     len = addPage(wrap.len, buf);      break;
       case  CMD_BL_CHECK_CRC    :     len = blCheckCRC(buf);    		  break;
	   case  CMD_BL_RESTART   	 :     len = systemRestartDelayed();      break;
	   case  CMD_GET_VERSION  	 :     len = getVersion(buf);    		  break;
	   case  CMD_TU           	 :     len = controlTU(buf);     		  break;
	   case  CMD_QUITTANCE    	 :     len = quittance();                 break;
	   case  CMD_GET_BALANCE     :     len = getBalance(buf);             break;
	   case  CMD_GET_GPS_PACKET  :     len = getGpsPacket(buf);           break;
	   case  CMD_GET_PHONE_NUMBER:     len = getPhoneNumber(buf);         break;
	   case  CMD_GET_RTC_TIME    :     len = getRTCTime(buf);             break;
	   case  CMD_MK_ZERO_MILEAGE :     len = zeroMileage(buf);            break;

							   
       default :    break;
     }

	//if(len)
        len = prepareForSend(bufRxOperGSM,len, 0x01, wrap.cmd, nrLongPack);   /* filling cmd,uid,len crc ...*/
    /* todo: should make some error codes and results*/
    return len;
}
/*******************************************************************************/
/* 	get  data								           */
/*******************************************************************************/
u16 getCmdUSB(u8 * buf)
{
    u16 len = 0;
    cmd_info wrap;
	u8 * pBuf = buf;
	u8 nrLongPack = 0;
	u16 limit = 2000;

    pBuf++;        //missing pilot
    wrap.cmd  = *pBuf;
    pBuf  += SIZE_WRAP_CMD;

    pBuf  += SIZE_WRAP_AUX;  //missing aux

    wrap.len = *(u16*)pBuf;   //length
    pBuf  += SIZE_WRAP_LEN;

	buf += SIZE_HEADER_PACKET;

    /*Executing commands and handling data*/
    switch(wrap.cmd)
     {
	   case  CMD_ALL_INFO        :  len = genInfo(buf, 0);  			 break;
       case  CMD_READ_CONFIG     :  len = menuDisplay(buf, &nrLongPack, limit);break;
       case  CMD_WRITE_CONFIG    :  len = writeConfig(wrap.len, buf );   break;
       case  CMD_PSW_CHECK       :  len = checkPsw( buf );  			 break;
       case  CMD_BL_ENTER        :  len = blEnter();  					 break;
       case  CMD_BL_PORTION      :  len = addPortion(wrap.len, buf);     break;
       case  CMD_BL_ADD_PAGE     :  len = addPage(wrap.len, buf);        break;
       case  CMD_BL_CHECK_CRC    :  len = blCheckCRC(buf);  			 break;
	   case  CMD_BL_RESTART      :  len = systemRestartDelayed();  	     break;
	   case  CMD_GET_VERSION     :  len = getVersion(buf);  			 break;
	   case  CMD_ACCEL           :  len = accelHandle(buf);  			 break;
	   case  CMD_TU              :  len = controlTU(buf);  			     break;
	   case  CMD_DEBUG           :  len = debScan(buf); 				 break;
	   case  CMD_QUITTANCE       :  len = quittance();  				 break;
	   case  CMD_GET_BALANCE     :  len = getBalance(buf);               break;
	   case  CMD_GET_GPS_PACKET  :  len = getGpsPacket(buf);             break;
	   case  CMD_SET_CALIBR_KOEF :  len = setCalibrKoef(buf);            break;
	   case  CMD_GET_PHONE_NUMBER:  len = getPhoneNumber(buf);           break;
	   case  CMD_GET_RTC_TIME    :  len = getRTCTime(buf);               break;
	   case  CMD_MK_ZERO_MILEAGE :  len = zeroMileage(buf);              break;

       default :    break;
     }

	len = prepareForSendUSB(buf,len,nrLongPack);
     
    /* todo: should make some error codes and results*/

    return len;
}

/*******************************************************************************/
/* 	GeneralInfo								           */
/*******************************************************************************/

u16 genInfo(u8 *buf, u8 flag)
{
  /* ASCII OR BINARY Protocol*/
  u16 len;
  if(chooseProtocol == PROTOCOL_ASCII)
    {
      len  = pgioPassed(0,buf);	   
      len += gprmcPassed(len,buf);  
      len += gngsaPassed(len,buf);
      len += mlgPassed(len,buf);
    }
  else	//binary protocol
    {
      len = prepareBinData(buf, flag);	      /* initial collect of data*/
    }
  return len;
}
/*******************************************************************************/
/* 	scan various info for USB								           */
/*******************************************************************************/
#if 1
u16 debScan(u8 *buf)
{
   u8 subCmd = *buf++;
   u8 nSize = 1;  //plus 1 byte of subcmd
   //*buf++ = getInfo(); //cap of packets
   //*buf++ = getInfoGsm();
   //*buf++ = 1;
   //nSize += 3;
   
   switch(subCmd)
    {

	  case 0 :   innerState.needRestart = 1;             
	             break;
	  case 1 :   innerState.flagDebug = 0;             
	             break;
	  case 2 :   innerState.flagDebug = 1;             
	             break;
	  case 3 :   innerState.flagTmpDebug = 0;  //go gsm 
                 gprsModemOn(innerState.activeSIMCard);
		         iniGSM();	  
	             break;
	  case 4 :   innerState.flagTmpDebug = 1;  //stop gsm
	             gprsModemOff();
	             break;

	  default : break;
    }
	 
   return nSize; 
}
#endif

#if 0 
u16 debScan(u8 *buf)
{
   u16 nSize = 256; 
   ainDebRead(buf, nSize);
   nSize *= 2;
   return nSize; 
}
#endif

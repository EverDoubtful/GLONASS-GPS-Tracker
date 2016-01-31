
/* Functional Description: Generic FIFO library for deeply
   embedded system. See the unit tests for usage examples. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fifoPackets.h"

// ------------------------------------------------------------------------------
void fifoPacketInit(fifo_packet *fp,node_struct * nodes, u16 len)
{
   fp->nodePtr = nodes;
   fp->head = fp->tail = 0;
   fp->buffer_len = len;
}
// ------------------------------------------------------------------------------
bool fifoPacketFull(fifo_packet *fp)
{
  return (fp ? (fifoPacketCount(fp) == fp->buffer_len) : true);
}
// ------------------------------------------------------------------------------

bool fifoPacketPut(fifo_packet *fp,u8 * buf, u16 len, u8 sign)
{
    u8 status = false;        /* return value */
	u16 index;
   if(fp)
    {
       if(!fifoPacketFull(fp))
	     {
	       index = fp->head % fp->buffer_len;
		   fp->nodePtr[index].sign = sign; 
		   fp->nodePtr[index].len  = len;
		   if(fp->nodePtr[index].info)
		      free(fp->nodePtr[index].info);
		   fp->nodePtr[index].info = (u8 *)malloc(len);
		   memcpy(fp->nodePtr[index].info,buf,len);
		   fp->head++;
		   if( fp->head == fp->buffer_len)
		     {
		      fp->head = 0;
			 }
		   status = true; 
	     }
    }
    return status;
          
}
// ------------------------------------------------------------------------------

u16 fifoPacketGet(fifo_packet *fp, u8 * buf, u8 * sign)
{
   u16 len = 0;
   u16 index;
   if(!fifoPacketEmpty(fp))
     {
	   index = fp->tail % fp->buffer_len;
	   memcpy(buf,fp->nodePtr[index].info,fp->nodePtr[index].len);
	   len = fp->nodePtr[index].len;
	   *sign = fp->nodePtr[index].sign;
	   fp->tail++; 
	   if(fp->tail == fp->buffer_len)
		   fp->tail = 0;       
     }
   return len;
}
// ------------------------------------------------------------------------------

u16 fifoPacketGetLen(fifo_packet *fp)
{
   if(!fifoPacketEmpty(fp))
     {
	   return fp->nodePtr[fp->tail % fp->buffer_len].len;
     }
   return 0;
}

// ------------------------------------------------------------------------------
u16 fifoPacketCount(fifo_packet *fp)
{
    u16 res = 0;
	if(fp)
	 {
	   if(fp->head >= fp->tail)
	      res = (fp->head - fp->tail);
	   else
	      res = (fp->buffer_len - fp->tail + fp->head);
	 }
 	//res = (fp ? (fp->head - fp->tail) : 0);
	return res;
}
// ------------------------------------------------------------------------------

void fifoPacketCountDebug(fifo_packet *fp, u16 * buf)
{
    //u16 res;
	if(fp)
	  {
		*buf++ = fp->head;
		*buf++ = fp->tail;
		 if(fp->head >= fp->tail)
	      *buf = (fp->head - fp->tail);
	     else
	      *buf = (fp->buffer_len - fp->tail + fp->head);
		//*buf = (fp->head - fp->tail);
	  }
 	//res = (fp ? (fp->head - fp->tail) : 0);
	//return res;
}
// ------------------------------------------------------------------------------
bool fifoPacketEmpty(fifo_packet *fp)
{
    return (fp ? (fifoPacketCount(fp) == 0) : true);
}
// ------------------------------------------------------------------------------

/*
void addNode(u8 * buf, u16 len)
{

   sNodes[indexNode] = malloc(sizeof(node_struct));

   sNodes[indexNode]->sign = 'U';    //undefined
   sNodes[indexNode]->info = (u8 *)malloc(len);
   memcpy(sNodes[indexNode]->info,buf,len); 

   //sNodes.sign = 'U';
   //sNodes.info = (u8 *)malloc(len);
   //memcpy(sNodes.info,buf,len);
   indexNode++;        
}


void delNode(void)
{
   if(indexNode)
    {
	  indexNode--;
      free(sNodes[indexNode]->info);
	  free(sNodes[indexNode]);
	  //free(sNodes.info);
      
    }
}
*/




/* Functional Description: Generic FIFO library for deeply
   embedded system. See the unit tests for usage examples.
   This library only uses a byte sized chunk.
   This library is designed for use in Interrupt Service Routines
   and so is declared as "static inline" */

#ifndef __FIFO_PACKETS_H
#define __FIFO_PACKETS_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct __node
{
   u16   len;
   u8    sign;
   u8  * info;
}node_struct;

/*
typedef struct __node_Bin
{
   u8       sign;
   u16      len;
   //cmd_info cmdWrap;
   u8  *    info;
}node_structBIN;
*/
struct fifo_packets_t {
    volatile u16 head;           /* first packet of data */
    volatile u16 tail;           /* last packet of data */
	         u16 buffer_len;     /* length of the data */
	node_struct * nodePtr;
};

typedef struct fifo_packets_t fifo_packet;


bool fifoPacketPut(fifo_packet *fp,u8 * buf, u16 len, u8 sign);
u16  fifoPacketGet(fifo_packet *fp,u8 * buf, u8 * sign);
u16  fifoPacketGetLen(fifo_packet *fp);
u16  fifoPacketCount(fifo_packet *fp);
bool fifoPacketEmpty(fifo_packet *fp);
void fifoPacketInit(fifo_packet *fp,node_struct * nodes, u16 len);
bool fifoPacketFull(fifo_packet *fp);

void fifoPacketCountDebug(fifo_packet *fp, u16 * buf);

//void addNode(u8 * node, u16 len);
//void delNode(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif //__FIFO_PACKETS_H

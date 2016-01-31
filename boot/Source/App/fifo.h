
/* Functional Description: Generic FIFO library for deeply
   embedded system. See the unit tests for usage examples.
   This library only uses a byte sized chunk.
   This library is designed for use in Interrupt Service Routines
   and so is declared as "static inline" */

#ifndef __FIFO_H
#define __FIFO_H

#include <stdint.h>
//#include <stdbool.h>
#include "stm32f10x.h"

#define true 1
#define false    !true

struct fifo_buffer_t {
    volatile u16 head;      /* first byte of data */
    volatile u16 tail;     /* last byte of data */
    volatile u8 *buffer; /* block of memory or array of data */
             u16 buffer_len;     /* length of the data */
};
typedef struct fifo_buffer_t fifo_buffer;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

u8 FIFO_Empty(
        fifo_buffer const *b);

u8 FIFO_Peek(
        fifo_buffer const *b);

u8 FIFO_Get(
        fifo_buffer * b);

u8 FIFO_Put(
        fifo_buffer * b,
        u8 data_byte);

/* note: buffer_len must be a power of two */
void FIFO_Init(
        fifo_buffer * b,
        volatile u8 *buffer,
        u16 buffer_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif //__FIFO_H

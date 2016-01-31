
/* Functional Description: Generic FIFO library for deeply
   embedded system. See the unit tests for usage examples. */

#include <stddef.h>
#include <stdint.h>
#include "fifo.h"

/****************************************************************************
* DESCRIPTION: Returns the number of elements in the ring buffer
* RETURN:      Number of elements in the ring buffer
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
static u16 FIFO_Count (fifo_buffer const *b)
{
	u16 cpsr, res;
	cpsr = __disable_irq();
    res = (b ? (b->head - b->tail) : 0);
	if(!cpsr)
     	__enable_irq();
	return res;
}

/****************************************************************************
* DESCRIPTION: Returns the empty/full status of the ring buffer
* RETURN:      true if the ring buffer is full, false if it is not.
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
static u8 FIFO_Full (fifo_buffer const *b)
{
    return (b ? (FIFO_Count(b) == b->buffer_len) : true);
}

/****************************************************************************
* DESCRIPTION: Returns the empty/full status of the ring buffer
* RETURN:      true if the ring buffer is empty, false if it is not.
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
u8 FIFO_Empty(fifo_buffer const *b)
{
    return (b ? (FIFO_Count(b) == 0) : true);
}

/****************************************************************************
* DESCRIPTION: Looks at the data from the head of the list without removing it
* RETURN:      byte of data, or zero if nothing in the list
* ALGORITHM:   none
* NOTES:       Use Empty function to see if there is data to retrieve
*****************************************************************************/
u8 FIFO_Peek(fifo_buffer const *b)
{
    if (b)
	 {
        return (b->buffer[b->tail % b->buffer_len]);
     }

    return 0;
}

/****************************************************************************
* DESCRIPTION: Gets the data from the front of the list, and removes it
* RETURN:      the data, or zero if nothing in the list
* ALGORITHM:   none
* NOTES:       Use Empty function to see if there is data to retrieve
*****************************************************************************/
u8 FIFO_Get(fifo_buffer * b)
{
    u8 data_byte = 0;
	u16 cpsr;
    if (!FIFO_Empty(b)) {
        data_byte = b->buffer[b->tail % b->buffer_len];
		cpsr = __disable_irq();
        b->tail++;
		if(b->tail == b->buffer_len)
		   b->tail = 0;
		if(!cpsr)
     		__enable_irq();

    }
    return data_byte;
}

/****************************************************************************
* DESCRIPTION: Adds an element of data to the FIFO
* RETURN:      true on succesful add, false if not added
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
u8 FIFO_Put(fifo_buffer * b,u8 data_byte)
{
    u8 status = false;        /* return value */
	u16 cpsr;
    if (b) {
        /* limit the ring to prevent overwriting */
        if (!FIFO_Full(b)) {
            b->buffer[b->head % b->buffer_len] = data_byte;
		    cpsr = __disable_irq();
            b->head++;
		    if(b->head == b->buffer_len)
		        b->head = 0;
		    if(!cpsr)
     		    __enable_irq();
            status = true;
        }
    }

    return status;
}

/****************************************************************************
* DESCRIPTION: Configures the ring buffer
* RETURN:      none
* ALGORITHM:   none
* NOTES:        buffer_len must be a power of two
*****************************************************************************/
void FIFO_Init(fifo_buffer * b,volatile u8 *buffer,u16 buffer_len)
{
    u16 cpsr;
    if (b) {
		cpsr = __disable_irq();
        b->head = 0;
        b->tail = 0;
        b->buffer = buffer;
        b->buffer_len = buffer_len;
		if(!cpsr)
     		    __enable_irq();
    }

    return;
}

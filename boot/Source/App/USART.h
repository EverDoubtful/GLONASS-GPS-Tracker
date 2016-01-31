#ifndef __USART_H
#define __USART_H
#ifdef __cplusplus
 extern "C" {
#endif

void USART1_SendByte(u16 data);
void USART1Write(u8* data,u16 len);
void USART1SendBuf(u8* data);
void USART1_Configuration(void);

#ifdef __cplusplus
}
#endif

#endif

#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f4xx_conf.h"
#include "sys.h"

#define USART_REC_LEN 200
#define EN_USART1_RX 1 //ʹ�ܣ�1��/��ֹ��0������1����

extern u8 USART1_RX_BUF[USART_REC_LEN];
extern u16 USART1_RX_STA;

void USART1_Init(u32 bound);
void USART3_Init(u32 bound);
void UART4_Init(u32 bound);

#endif

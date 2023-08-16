#ifndef __USART5_H
#define __USART5_H
#include "stdio.h"	
#include "system.h" 

#define UART5_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_UART5_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  UART5_RX_BUF[UART5_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 UART5_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void UART5_SendString(u8 *DAT,u8 len);
void UART5_SendData(u8 data);
void UART5_Init(u32 bound);
#endif



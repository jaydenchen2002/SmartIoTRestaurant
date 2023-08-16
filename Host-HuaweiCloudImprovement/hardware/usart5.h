#ifndef __USART5_H
#define __USART5_H
#include "stdio.h"	
#include "system.h" 

#define UART5_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_UART5_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
	  	
extern u8  UART5_RX_BUF[UART5_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 UART5_RX_STA;         		//����״̬���	
//����봮���жϽ��գ��벻Ҫע�����º궨��
void UART5_SendString(u8 *DAT,u8 len);
void UART5_SendData(u8 data);
void UART5_Init(u32 bound);
#endif



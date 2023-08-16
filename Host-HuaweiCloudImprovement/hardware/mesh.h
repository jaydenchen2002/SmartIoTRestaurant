#ifndef __mesh_H
#define __mesh_H
#include "stm32f10x.h"


void Usart3_Init(unsigned int baud);
void Mesh_Init(void);
void Mesh_SendData(char *address,char *data);
void Send_data(USART_TypeDef * USARTx,u8 *s);


#endif

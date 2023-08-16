#ifndef __usart_H
#define __usart_H

#include "stm32f10x.h" 


void UART4_Init(u32 bound);

void Order_history_1(char desk_num);
void Order_history_2(int order_num,char food);

extern int num2;
void clear_systle(void);
void clear_food(void);
void Compute_Total_price(char food);

void Food_delivery(unsigned char num);
void Take_meal(unsigned char num);
void Display_tablenum(unsigned char num);

void Data_analysis_recommendations_1(char * food1,char * food2,char * food3,char * food4,int num2);
void Data_analysis_recommendations_2(void);


void HUAWEI_Public(int temp, int humi);
void HUAWEI_Order_Public(void);

#endif



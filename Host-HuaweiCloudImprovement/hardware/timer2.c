
/*-------------------------------------------------*/
/*                                                 */
/*          		��ʱ��2                       */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h" 			 //������Ҫ��ͷ�ļ�
#include "dht11.h"         //������Ҫ��ͷ�ļ�                  
#include "usart.h"        //������Ҫ��ͷ�ļ�
#include "timer2.h"

//�����豸
#include "esp8266.h"

//Э���ļ�
#include "onenet.h"
#include "mqttkit.h"

#include "mesh.h"
#include "hmi.h"

//C��
#include <string.h>
#include <stdio.h>

	int DATA_falg=0;

/*-------------------------------------------------*/
/*����������ʱ��2ʹ��10s��ʱ                        */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM2_ENABLE_5S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;             //����һ�����ö�ʱ���ı���
	NVIC_InitTypeDef NVIC_InitStructure;                           //����һ�������жϵı���	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                //�����ж��������飺��2�� �������ȼ���0 1 2 3 �����ȼ���0 1 2 3		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);            //ʹ��TIM2ʱ��	
	TIM_DeInit(TIM2);                                              //��ʱ��2�Ĵ����ָ�Ĭ��ֵ	
	TIM_TimeBaseInitStructure.TIM_Period = 7000-1; 	           //�����Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36000-1;             //���ö�ʱ��Ԥ��Ƶ��
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //1��Ƶ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);            //����TIM2
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);                    //�������жϱ�־λ
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                     //ʹ��TIM2����ж�    
	TIM_Cmd(TIM2, ENABLE);                                         //��TIM2                          
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;                //����TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;      //��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;             //�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //�ж�ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);                                //�����ж�
}


/*-------------------------------------------------*/
/*����������ʱ��2�жϷ�����                      */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TIM2_IRQHandler(void)
{
	char humidity;				//����һ������������ʪ��ֵ
	char temperature;			//����һ�������������¶�ֵ	
					
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)	
	{ 
		DHT11_Read_Data(&temperature,&humidity);//��ȡ��ʪ��ֵ
//		UsartPrintf(USART_DEBUG, "Temperature:%d,humidity:%d",temperature,humidity);
		
		  if(DATA_falg==0)
			 {
				HUAWEI_Public((int)temperature,(int)humidity);
			 	ESP8266_Clear();
			 }
			else if(DATA_falg==1)
			 {
					HUAWEI_Order_Public(); // ��Ϊ���ϴ��˵�
					ESP8266_Clear();
				  DATA_falg=0;
			 }
				
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   	
	}
}

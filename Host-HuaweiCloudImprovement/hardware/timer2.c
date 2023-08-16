
/*-------------------------------------------------*/
/*                                                 */
/*          		定时器2                       */
/*                                                 */
/*-------------------------------------------------*/

#include "stm32f10x.h" 			 //包含需要的头文件
#include "dht11.h"         //包含需要的头文件                  
#include "usart.h"        //包含需要的头文件
#include "timer2.h"

//网络设备
#include "esp8266.h"

//协议文件
#include "onenet.h"
#include "mqttkit.h"

#include "mesh.h"
#include "hmi.h"

//C库
#include <string.h>
#include <stdio.h>

	int DATA_falg=0;

/*-------------------------------------------------*/
/*函数名：定时器2使能10s定时                        */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM2_ENABLE_5S(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;             //定义一个设置定时器的变量
	NVIC_InitTypeDef NVIC_InitStructure;                           //定义一个设置中断的变量	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);                //设置中断向量分组：第2组 抢先优先级：0 1 2 3 子优先级：0 1 2 3		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);            //使能TIM2时钟	
	TIM_DeInit(TIM2);                                              //定时器2寄存器恢复默认值	
	TIM_TimeBaseInitStructure.TIM_Period = 7000-1; 	           //设置自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 36000-1;             //设置定时器预分频数
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;    //1分频
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);            //设置TIM2
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);                    //清除溢出中断标志位
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                     //使能TIM2溢出中断    
	TIM_Cmd(TIM2, ENABLE);                                         //开TIM2                          
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;                //设置TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;      //抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;             //子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                //中断通道使能
	NVIC_Init(&NVIC_InitStructure);                                //设置中断
}


/*-------------------------------------------------*/
/*函数名：定时器2中断服务函数                      */
/*参  数：无                                       */
/*返回值：无                                       */
/*-------------------------------------------------*/
void TIM2_IRQHandler(void)
{
	char humidity;				//定义一个变量，保存湿度值
	char temperature;			//定义一个变量，保存温度值	
					
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)	
	{ 
		DHT11_Read_Data(&temperature,&humidity);//读取温湿度值
//		UsartPrintf(USART_DEBUG, "Temperature:%d,humidity:%d",temperature,humidity);
		
		  if(DATA_falg==0)
			 {
				HUAWEI_Public((int)temperature,(int)humidity);
			 	ESP8266_Clear();
			 }
			else if(DATA_falg==1)
			 {
					HUAWEI_Order_Public(); // 向华为云上传菜单
					ESP8266_Clear();
				  DATA_falg=0;
			 }
				
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   	
	}
}

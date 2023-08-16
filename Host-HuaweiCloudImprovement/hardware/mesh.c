#include "mesh.h"
#include "usart.h"
#include "delay.h"
#include "hmi.h"
#include "beep.h"
#include "syn6288.h"
#include "timer2.h"



//C库
#include <string.h>
#include <stdio.h>

#define Max_BUFF_Len 50
unsigned char Uart3_Buffer[Max_BUFF_Len];
unsigned int Uart3_Rx=0;
unsigned char MESH_Buffer[Max_BUFF_Len];
unsigned int MESH_Data=0;
unsigned int timeout =0;
unsigned int len=0;

extern int total_price;

// 设备短地址
//char host_address[3] = {0x11,0x11};  // 主机
//char slave_address[3] = {0x11,0x21}; // 从机
//char robot_address[3] = {0x11,0x31}; // 机器人


/*
************************************************************
*	函数名称：	Usart3_Init
*
*	函数功能：	串口3初始化
*
*	入口参数：	baud：设定的波特率
*
*	返回参数：	无
*
*	说明：		TX-PB10		RX-PB11
************************************************************
*/
void Usart3_Init(unsigned int baud)
{

	GPIO_InitTypeDef gpioInitStruct;
	USART_InitTypeDef usartInitStruct;
	NVIC_InitTypeDef nvicInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	//PB10	TXD
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_10;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioInitStruct);
	
	//PB11	RXD
	gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpioInitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &gpioInitStruct);
	
	usartInitStruct.USART_BaudRate = baud;
	usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流控
	usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						//接收和发送
	usartInitStruct.USART_Parity = USART_Parity_No;									//无校验
	usartInitStruct.USART_StopBits = USART_StopBits_1;								//1位停止位
	usartInitStruct.USART_WordLength = USART_WordLength_8b;							//8位数据位
	USART_Init(USART3, &usartInitStruct);
	
	USART_Cmd(USART3, ENABLE);														//使能串口
	USART_ClearFlag(USART3, USART_FLAG_TC);	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);									//使能接收中断
	
	nvicInitStruct.NVIC_IRQChannel = USART3_IRQn;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&nvicInitStruct);

}
/*
************************************************************
*	函数名称：	Send_data
*
*	函数功能：	串口发送数据
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		u8为unsigned char
************************************************************
*/
void Send_data(USART_TypeDef * USARTx,u8 *s)
{
	while(*s != '\0')
	{ 
		USART_SendData(USARTx,*s);
		s++;
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TC)==RESET);	
	}
}
/*
************************************************************
*	函数名称：	Mesh_Init
*
*	函数功能：	初始化组网模块
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		主机短地址：1111
						从机短地址：1121
					机器人短地址：1131
************************************************************
*/
void Mesh_Init(void)
{
	Usart3_Init(9600);
	
	//设置广播名
	Send_data(USART3, (unsigned char *)"AT+NAMEJDY-24M\r\n");		
	DelayXms(500);
	//设置NETID,组网内统一
	Send_data(USART3, (unsigned char *)"AT+NETID6968\r\n");		
	DelayXms(500);
	//设置组网短地址,每个设备不一样
	Send_data(USART3, (unsigned char *)"AT+MADDR1111\r\n");		
	DelayXms(500);
	//reset
	Send_data(USART3, (unsigned char *)"AT+RESET\r\n");		
	DelayXms(1000);
}
/*
************************************************************
*	函数名称：	Mesh_SendData
*
*	函数功能：	Mesh发送数据
*
*	入口参数：	*address: 发送目标地址
							*data：		发送数据内容
*
*	返回参数：	无
*
*	说明：		广播目标地址为：0xFF,0XFF
************************************************************
*/
void Mesh_SendData(char *address,char *data)
{
char MESH_send_data[50]={0x41,0x54,0x2B,0x4D,0x45,0x53,0x48,0x01};//帧头"AT+MESH"+0x01表示有ACK应答
//char MESH_send_data[50]="AT+MESH";//帧头"AT+MESH"
char MESH_end[4]={0xFF,0x0D,0X0A};//0xFF作为接收结束判断位，帧尾：0x0D,0x0A

strcat(MESH_send_data,address);//连接帧头与发送目标地址
strcat(MESH_send_data,data);//连接发送数据
strcat(MESH_send_data,MESH_end);//连接帧尾

Send_data(USART3, (unsigned char *)MESH_send_data);
}

/*
************************************************************
*	函数名称：	USART3_IRQHandler
*
*	函数功能：	串口3收发中断
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		数据接收格式：0xF1,0xDD + 剩余数据长度 + 发送方地址（2字节） + 接收方短地址（2字节） + 数据内容
************************************************************
*/
void USART3_IRQHandler(void)
{
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //接收中断
	{
		
		Uart3_Buffer[Uart3_Rx] = USART_ReceiveData(USART3);
		Uart3_Rx++; 
		
		if(Uart3_Buffer[Uart3_Rx-1] == 0xFF || Uart3_Rx == Max_BUFF_Len) // 大于缓冲区最大容量则重新接收
		{
			
			if(Uart3_Buffer[0] == 0xF1 && Uart3_Buffer[1] == 0xDD) //帧头格式正确
			{
				
				while(Uart3_Buffer[MESH_Data+7] != 0xFF) //自定义发送端结束符0xFF
				{
					MESH_Buffer[MESH_Data] = Uart3_Buffer[MESH_Data+7];  // 接收数据存放在MESH_Buffer中,长度为MESH_Data
					USART_SendData(USART_DEBUG,MESH_Buffer[MESH_Data+7]); // 接收数据打印到串口1
					while(USART_GetFlagStatus(USART_DEBUG,USART_FLAG_TC)==RESET);
					
					MESH_Data++;
					
												// 超过500次跳出死循环
												timeout++;
												if(timeout == 500)
												{
													timeout = 0;
													Send_data(USART1,(unsigned char *)"死循环！");
													break;		
												}
					
				}
				
				if(MESH_Buffer[0] == 'Y')//情景1：呼叫人工服务
				{												 //在主机显示呼叫信息
					Voice_Announcements(3);		//语音播报
					Display_tablenum(MESH_Buffer[1]);
//					beep=0;
//					DelayXms(250);
//					beep=1;
				}
				
				else if(MESH_Buffer[0] == 'X') //情景2：呼叫收餐-X
				{															 //在屏幕显示呼叫信息，主机主动呼叫机器人
					Voice_Announcements(5);	//语音播报
					Take_meal(MESH_Buffer[1]);
//					beep=0;
//					DelayXms(250);
//					beep=1;
				}
				
				else if(MESH_Buffer[0] == 'V') //情景3：呼叫送餐-V
				{															 //在屏幕显示呼叫信息，主机主动呼叫机器人
					Food_delivery(MESH_Buffer[1]);
//					beep=0;
//					DelayXms(250);
//					beep=1;
				}

				
				else if(MESH_Buffer[0] == '1' ||
								MESH_Buffer[0] == '2' ||
								MESH_Buffer[0] == '3' ||
							  MESH_Buffer[0] == '4' ||
							  MESH_Buffer[0] == '5' ||
							  MESH_Buffer[0] == '6' )
				{
					u8 k;
					Voice_Announcements(4);	//语音播报
					total_price=0;
					clear_food();
					clear_systle();
					for(k=1;k<11;k++)
					{
						Compute_Total_price(MESH_Buffer[k]);
					}			
					Order_history_1(MESH_Buffer[0]);
					for(k=1;k<=num2;k++)
					{
						Order_history_2(k,MESH_Buffer[k]);
					}
					num2=0;
					
					DATA_falg=1;				// 向云端上传数据标志位
					
//				beep=0;
//				DelayXms(250);
//				beep=1;
				}
				
//				len = MESH_Data;
				MESH_Data = 0;
				memset(MESH_Buffer, 0, Max_BUFF_Len);
			}
			
			Uart3_Rx=0;
			memset(Uart3_Buffer, 0, Max_BUFF_Len);
		}
		
		if(Uart3_Buffer[0] != 0xF1) // 第一个字节不对重新接收
		{
			Uart3_Rx=0;
			memset(Uart3_Buffer, 0, Max_BUFF_Len);
		}
		
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}
		
}

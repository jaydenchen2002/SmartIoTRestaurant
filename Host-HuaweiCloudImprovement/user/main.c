/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	main.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-08
	*
	*	版本： 		V1.0
	*
	*	说明： 		接入onenet，上传数据和命令控制
	*
	*	修改记录：	
	************************************************************
	************************************************************
	************************************************************
**/

//单片机头文件
#include "stm32f10x.h"

//网络协议层
#include "onenet.h"

//网络设备
#include "esp8266.h"

//硬件驱动
#include "delay.h"
//#include "beep.h"
#include "usart.h"

//C库
#include <string.h>

//温湿度传感器
#include "timer2.h"
#include "dht11.h"       

//串口屏
#include "hmi.h"

//mesh组网
#include "mesh.h"

//语音模块
#include "stdio.h"
#include "usart5.h"
#include "syn6288.h"


//订阅与发布的主题
//#define SET_TOPIC  "$oc/devices/product_device/sys/messages/down"  		 // 订阅: 平台下发消息给设备
//#define POST_TOPIC "$oc/devices/product_device/sys/properties/report"  // 发布：设备上报数据
//#define message_TOPIC	"$oc/devices/product_device/sys/messages/up" 		 // 消息上行

	u16 i;
	
/*
************************************************************
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置

	Delay_Init();									//systick初始化
	
	Usart1_Init(9600);							//串口1，打印信息用
	
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	
	UART4_Init(9600);				//串口屏初始化
	
	Mesh_Init();						//初始化Mesh模块
	
//	BEEP_Init();									//蜂鸣器初始化
	
	ESP8266_Init();					//初始化ESP8266
	
	while(OneNet_DevLink())			//接入OneNET
		DelayXms(500);
	
		//温湿度传感器上报初始化	
	DHT11_Init();
	TIM2_ENABLE_5S();
	
	UART5_Init(9600);				//语音模块初始化
	DelayXms(10);
	
	UsartPrintf(USART_DEBUG, " Hardware init OK\r\n");
	
}

/*
************************************************************
*	函数名称：	main
*
*	函数功能：	
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
int main(void)
{
	
//	const char *topics[] = {"kylinBoard"};
	
//	unsigned short timeCount = 0;	//发送间隔变量

//	unsigned char *dataPtr = NULL;
	
	Hardware_Init();				//初始化外围硬件
	
	Voice_Announcements(1);	//语音播报
	
//	beep=0;
//	DelayXms(250);
//	beep=1;
	
//	OneNet_Subscribe(topics, 1);
		

	while(1)
	{
//		dataPtr = ESP8266_GetIPD(0);
//		if(dataPtr != NULL)
//			OneNet_RevPro(dataPtr);
		
//		DelayXms(10);
	
	}

}

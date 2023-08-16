#include "hmi.h"
#include "mesh.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include "esp8266.h"
#include "onenet.h"
#include "mqttkit.h"
#include "syn6288.h"



#define Max_BUFF 100
unsigned char Uart4_Buffer[Max_BUFF];
unsigned int Uart4_Rx=0;


char mqtt_message[200]; // 上传数据缓冲区


//结尾符
char Order_end[5]={0x22,0xff,0xff,0xff};


int total_price;//总价
int style_num[8];//七个菜系数量


/*******************************************************************************
* 函 数 名         : UART4_Init
* 函数功能		   : USART5初始化函数
* 输    入         : bound:波特率
* 输    出         : 无
*******************************************************************************/ 
void UART4_Init(u32 bound)
{
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
 
	
	/*  配置GPIO的模式和IO口 */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//TX			  
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;	    //复用推挽输出
	GPIO_Init(GPIOC,&GPIO_InitStructure);  /* 初始化串口输入IO */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;//RX			 //串口输入
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;		  //模拟输入
	GPIO_Init(GPIOC,&GPIO_InitStructure); /* 初始化GPIO */
	

   //UART4 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(UART4, &USART_InitStructure); //初始化串口1
	
	USART_Cmd(UART4, ENABLE);  //使能串口1 
	
	USART_ClearFlag(UART4, USART_FLAG_TC);
		
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启相关中断

	//Uart4 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;//串口4中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、	
}


/*
************************************************************
*	函数名称：	Clear_history_order
*
*	函数功能：	历史订单
*
*	入口参数：	桌号，总价


*
*	返回参数：	无
*
*	说明：		收到菜单后分析完一并发送至主机屏幕
						W1~6
************************************************************
*/

char clear_order_1[20] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x36,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_2[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x39,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_3[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x32,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_4[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x35,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_5[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x38,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_6[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x31,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_7[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x34,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_8[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x37,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_9[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x33,0x30,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_10[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x38,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_11[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x37,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_12[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x30,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_13[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x33,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_14[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x36,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_15[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x39,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_16[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x32,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_17[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x35,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_18[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x38,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_19[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x33,0x31,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};
char clear_order_20[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x31,0x2E,0x74,0x78,0x74,0x3D,0x22,0x20,0x22,0xff,0xff,0xff};



void Clear_history_order(void)
{
	Send_data(UART4,(unsigned char *)clear_order_1);
	Send_data(UART4,(unsigned char *)clear_order_2);
	Send_data(UART4,(unsigned char *)clear_order_3);
	Send_data(UART4,(unsigned char *)clear_order_4);
	Send_data(UART4,(unsigned char *)clear_order_5);
	Send_data(UART4,(unsigned char *)clear_order_6);
	Send_data(UART4,(unsigned char *)clear_order_7);
	Send_data(UART4,(unsigned char *)clear_order_8);
	Send_data(UART4,(unsigned char *)clear_order_9);
	Send_data(UART4,(unsigned char *)clear_order_10);
	Send_data(UART4,(unsigned char *)clear_order_11);
	Send_data(UART4,(unsigned char *)clear_order_12);
	Send_data(UART4,(unsigned char *)clear_order_13);
	Send_data(UART4,(unsigned char *)clear_order_14);
	Send_data(UART4,(unsigned char *)clear_order_15);
	Send_data(UART4,(unsigned char *)clear_order_16);
	Send_data(UART4,(unsigned char *)clear_order_17);
	Send_data(UART4,(unsigned char *)clear_order_18);
	Send_data(UART4,(unsigned char *)clear_order_19);
	Send_data(UART4,(unsigned char *)clear_order_20);
}




/*
************************************************************
*	函数名称：	Order_history_1
*
*	函数功能：	历史订单
*
*	入口参数：	桌号，总价


*
*	返回参数：	无
*
*	说明：		收到菜单后分析完一并发送至主机屏幕
						W1~6
************************************************************
*/
//收到订单显示
unsigned char order_1[12]={0x76,0x69,0x73,0x20,0x74,0x35,0x2C,0x31,0xff,0xff,0xff};
unsigned char order_2[13]={0x76,0x69,0x73,0x20,0x62,0x31,0x31,0x2C,0x31,0xff,0xff,0xff};


//桌号
char Desk_num[19]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x2E,0x74,0x78,0x74,0x3D,0x22,0x31,0x22,0xff,0xff,0xff};
//总价
char Total_price_2[10];

void Order_history_1(char desk_num)
{
	//总价
	char Total_price_1[30] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x33,0x2E,0x74,0x78,0x74,0x3D,0x22};
	//桌号
	Desk_num[13]=desk_num;
	
	Clear_history_order();
	
	//总价
	memset(Total_price_2, 0, 10);
	sprintf(Total_price_2,"%d",total_price);
	strcat(Total_price_1,Total_price_2);
	strcat(Total_price_1,Order_end);

	
	Send_data(UART4,(unsigned char *)order_1);
	Send_data(UART4,(unsigned char *)order_2);
	Send_data(UART4,(unsigned char *)Desk_num);
	Send_data(UART4,(unsigned char *)Total_price_1);
	
}
/*
************************************************************
*	函数名称：	Order_history_2
*
*	函数功能：	历史订单
*
*	入口参数：	order_num--订单的第几行(int)
							food--哪个菜品(char)
							第一道菜+菜的编号是什么

*
*	返回参数：	无
*
*	说明：		收到菜单后分析完一并发送至主机屏幕
						W1~6
						需要用到循环，几个菜品调用几次Order_history_2
						
************************************************************
*/
//菜单，单价
char food_a[]="煎鸡胸肉";				int price_a=26;
char food_b[]="荞麦面";					int price_b=20;
char food_c[]="蔬菜沙拉";				int price_c=15;
char food_d[]="糖醋排骨";				int price_d=47;
char food_e[]="松鼠桂鱼";				int price_e=56;
char food_f[]="宫保鸡丁";				int price_f=35;
char food_g[]="凉拌黄瓜";				int price_g=12;
char food_h[]="干拌海蜇";				int price_h=17;
char food_i[]="口水鸡";					int price_i=30;
char food_j[]="肉夹馍";					int price_j=8;
char food_k[]="拌面";						int price_k=5;
char food_l[]="水饺";						int price_l=10;
char food_m[]="排骨玉米汤";			int price_m=40;
char food_n[]="花蛤豆腐汤";			int price_n=32;
char food_o[]="紫菜蛋花汤";			int price_o=22;
char food_p[]="可乐";						int price_p=3;
char food_q[]="啤酒";						int price_q=4;
char food_r[]="橙汁";						int price_r=5;
char food_s[]="炒米粉";					int price_s=14;
char food_t[]="炒面";						int price_t=16;
char food_u[]="米饭";						int price_u=6;

//菜品
char Order_food_a[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x36,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_b[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x39,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_c[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x32,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_d[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x35,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_e[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x38,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_f[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x31,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_g[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x34,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_h[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x37,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_i[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x33,0x30,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_food_j[60] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x38,0x2E,0x74,0x78,0x74,0x3D,0x22};

//单价
char Order_price_a[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x37,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_b[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x30,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_c[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x33,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_d[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x36,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_e[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x39,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_f[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x32,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_g[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x35,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_h[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x38,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_i[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x33,0x31,0x2E,0x74,0x78,0x74,0x3D,0x22};
char Order_price_j[60]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x31,0x31,0x2E,0x74,0x78,0x74,0x3D,0x22};



//菜品
char Order_food[100];		//主体
char Order_food_2[100];	//拼接部分
//单价
char Order_price[100];	//主体
char Order_price_2[10];	//拼接部分
int price;


void Order_history_2(int order_num,char food)
{
	//清空菜单缓存
	memset(Order_food, 0, 100);
	memset(Order_food_2, 0, 100);
	
	switch(order_num)//根据订单不同序号复制菜品和价格在屏幕中的空格位置
	{
		case 1 :
					strcpy(Order_food,Order_food_a);
					strcpy(Order_price,Order_price_a);
					break;
		case 2 :
					strcpy(Order_food,Order_food_b);
					strcpy(Order_price,Order_price_b);
					break;	
		case 3 :
					strcpy(Order_food,Order_food_c);
					strcpy(Order_price,Order_price_c);
					break;
		case 4 :
					strcpy(Order_food,Order_food_d);
					strcpy(Order_price,Order_price_d);
					break;
		case 5 :
					strcpy(Order_food,Order_food_e);
					strcpy(Order_price,Order_price_e);
					break;
		case 6 :
					strcpy(Order_food,Order_food_f);
					strcpy(Order_price,Order_price_f);
					break;
		case 7 :
					strcpy(Order_food,Order_food_g);
					strcpy(Order_price,Order_price_g);
					break;		
		case 8 :
					strcpy(Order_food,Order_food_h);
					strcpy(Order_price,Order_price_h);
					break;		
		case 9 :
					strcpy(Order_food,Order_food_i);
					strcpy(Order_price,Order_price_i);
					break;		
		case 10 :
					strcpy(Order_food,Order_food_j);
					strcpy(Order_price,Order_price_j);
					break;		
		
		default  : break;
	}
	
	switch(food)//根据不同菜品赋值中文和价格
	{
		case 'a' : 
					strcpy(Order_food_2,food_a);
					price=price_a;
					break;
		case 'b' : 
					strcpy(Order_food_2,food_b);
					price=price_b;
					break;
		case 'c' : 
					strcpy(Order_food_2,food_c);
					price=price_c;
					break;
		case 'd' : 
					strcpy(Order_food_2,food_d);
					price=price_d;
					break;
		case 'e' : 
					strcpy(Order_food_2,food_e);
					price=price_e;
					break;
		case 'f' : 
					strcpy(Order_food_2,food_f);
					price=price_f;
					break;
		case 'g' : 
					strcpy(Order_food_2,food_g);
					price=price_g;
					break;
		case 'h' : 
					strcpy(Order_food_2,food_h);
					price=price_h;
					break;
		case 'i' : 
					strcpy(Order_food_2,food_i);
					price=price_i;
					break;
		case 'j' : 
					strcpy(Order_food_2,food_j);
					price=price_j;
					break;
		case 'k' : 
					strcpy(Order_food_2,food_k);
					price=price_k;
					break;
		case 'l' : 
					strcpy(Order_food_2,food_l);
					price=price_l;
					break;
		case 'm' : 
					strcpy(Order_food_2,food_m);
					price=price_m;
					break;
		case 'n' : 
					strcpy(Order_food_2,food_n);
					price=price_n;
					break;
		case 'o' : 
					strcpy(Order_food_2,food_o);
					price=price_o;
					break;
		case 'p' : 
					strcpy(Order_food_2,food_p);
					price=price_p;
					break;
		case 'q' : 
					strcpy(Order_food_2,food_q);
					price=price_q;
					break;
		case 'r' : 
					strcpy(Order_food_2,food_r);
					price=price_r;
					break;
		case 's' : 
					strcpy(Order_food_2,food_s);
					price=price_s;
					break;
		case 't' : 
					strcpy(Order_food_2,food_t);
					price=price_t;
					break;
		case 'u' : 
					strcpy(Order_food_2,food_u);
					price=price_u;
					break;
	
		default  : break;
		
	}
		
		//菜品
		strcat(Order_food,Order_food_2);
		strcat(Order_food,Order_end);
	
		//单价
		memset(Order_price_2, 0, 10);
		sprintf(Order_price_2,"%d",price);
		strcat(Order_price,Order_price_2);
		strcat(Order_price,Order_end);
		//发送数据
		Send_data(UART4,(unsigned char *)Order_food);
		Send_data(UART4,(unsigned char *)Order_price);

}

/*
************************************************************
*	函数名称：	Compute_Total_price
*
*	函数功能：	计算各个菜数量与总价
*
*	入口参数：	桌号，总价


*
*	返回参数：	无
*
*	说明：		收到菜单后分析完一并发送至主机屏幕
						W1~6
************************************************************
*/
int food_num[21];//食物数量

int num2;

void clear_systle(void)
{
	memset(style_num,0,8);
}

void clear_food(void)
{
	memset(food_num,0,21);
}

void Compute_Total_price(char food)
{
	
		switch(food)
		{
			case 'a' : total_price += price_a;  num2++;		style_num[0]++;		food_num[0]++;	break;
			case 'b' : total_price += price_b;  num2++;		style_num[0]++;		food_num[1]++;	break;
			case 'c' : total_price += price_c;  num2++;		style_num[0]++;		food_num[2]++;	break;
			case 'd' : total_price += price_d;  num2++;		style_num[1]++;		food_num[3]++;	break;
			case 'e' : total_price += price_e;  num2++;		style_num[1]++;		food_num[4]++;	break;
			case 'f' : total_price += price_f;  num2++;		style_num[1]++;		food_num[5]++;	break;
			case 'g' : total_price += price_g;  num2++;		style_num[2]++;		food_num[6]++;	break;
			case 'h' : total_price += price_h;  num2++;		style_num[2]++;		food_num[7]++;	break;
			case 'i' : total_price += price_i;  num2++;		style_num[2]++;		food_num[8]++;	break;
			case 'j' : total_price += price_j;  num2++;		style_num[3]++;		food_num[9]++;	break;
			case 'k' : total_price += price_k;  num2++;		style_num[3]++;		food_num[10]++;	break;
			case 'l' : total_price += price_l;  num2++;		style_num[3]++;		food_num[11]++;	break;
			case 'm' : total_price += price_m;  num2++;		style_num[4]++;		food_num[12]++;	break;
			case 'n' : total_price += price_n;  num2++;		style_num[4]++;		food_num[13]++;	break;
			case 'o' : total_price += price_o;  num2++;		style_num[4]++;		food_num[14]++;	break;
			case 'p' : total_price += price_p;  num2++;		style_num[5]++;		food_num[15]++;	break;
			case 'q' : total_price += price_q;  num2++;		style_num[5]++;		food_num[16]++;	break;
			case 'r' : total_price += price_r;  num2++;		style_num[5]++;		food_num[17]++;	break;
			case 's' : total_price += price_s;  num2++;		style_num[6]++;		food_num[18]++;	break;
			case 't' : total_price += price_t;  num2++;		style_num[6]++;		food_num[19]++;	break;
			case 'u' : total_price += price_u;  num2++;		style_num[6]++;		food_num[20]++;	break;
			
			default : break;
		}
}
/*
************************************************************
*	函数名称：	Data_analysis_recommendations_1
*
*	函数功能：	数据分析建议
*
*	入口参数：	9个入口参数
							A菜系，B菜系
							C菜系，D菜系
							预计销售额1元，实际销售额2元
							偏差3元，差异比4%，预计明日销售额5元
*
*	返回参数：	无
*
*	说明：		收到菜单后分析完一并发送至主机屏幕
						W1~6
************************************************************
*/
//销售额
char Analysis_num1_2[10];
char Analysis_num2_2[10];
char Analysis_num3_2[10];
//char Analysis_num4_2[10];
char Analysis_num5_2[10];
	

void Data_analysis_recommendations_1(char * food1,char * food2,char * food3,char * food4,int num2)
{
	int num1,num3,num5;
//	int num4;
	
	//销量分析
	char Analysis_num1_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x31,0X33,0x2E,0x74,0x78,0x74,0x3D,0x22};	
	char Analysis_num2_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x31,0X36,0x2E,0x74,0x78,0x74,0x3D,0x22};	
	char Analysis_num3_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x31,0X38,0x2E,0x74,0x78,0x74,0x3D,0x22};	
	char Analysis_num4_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x32,0X32,0x2E,0x74,0x78,0x74,0x3D,0x22,0x34,0x22,0xff,0xff,0xff};	
	char Analysis_num5_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x32,0X33,0x2E,0x74,0x78,0x74,0x3D,0x22};	
	//菜系
	char Analysis_food1[30]={0x73,0X6A,0X66,0X78,0X2E,0x74,0x31,0x2E,0x74,0x78,0x74,0x3D,0x22};
	char Analysis_food2[30]={0x73,0X6A,0X66,0X78,0X2E,0x74,0x33,0x2E,0x74,0x78,0x74,0x3D,0x22};
	char Analysis_food3[30]={0x73,0X6A,0X66,0X78,0X2E,0x74,0x36,0x2E,0x74,0x78,0x74,0x3D,0x22};
	char Analysis_food4[30]={0x73,0X6A,0X66,0X78,0X2E,0x74,0x38,0x2E,0x74,0x78,0x74,0x3D,0x22};	
		
	//拼接菜系
	strcat(Analysis_food1,food1);
	strcat(Analysis_food1,Order_end);
	
	strcat(Analysis_food2,food2);
	strcat(Analysis_food2,Order_end);

	strcat(Analysis_food3,food3);
	strcat(Analysis_food3,Order_end);

	strcat(Analysis_food4,food4);
	strcat(Analysis_food4,Order_end);

	
	num1=(int)num2*0.96;
	num3=num2-num1;
	num5=(int)num2*1.04;

	
	//拼接销售额
	memset(Analysis_num1_2, 0, 10);
	sprintf(Analysis_num1_2,"%d",num1);
	strcat(Analysis_num1_1,Analysis_num1_2);
	strcat(Analysis_num1_1,Order_end);
	
	memset(Analysis_num2_2, 0, 10);
	sprintf(Analysis_num2_2,"%d",num2);
	strcat(Analysis_num2_1,Analysis_num2_2);
	strcat(Analysis_num2_1,Order_end);

	memset(Analysis_num3_2, 0, 10);
	sprintf(Analysis_num3_2,"%d",num3);
	strcat(Analysis_num3_1,Analysis_num3_2);
	strcat(Analysis_num3_1,Order_end);

//	memset(Analysis_num4_2, 0, 10);
//	sprintf(Analysis_num4_2,"%d",num4);
//	strcat(Analysis_num4_1,Analysis_num4_2);
//	strcat(Analysis_num4_1,Order_end);

	memset(Analysis_num5_2, 0, 10);
	sprintf(Analysis_num5_2,"%d",num5);
	strcat(Analysis_num5_1,Analysis_num5_2);
	strcat(Analysis_num5_1,Order_end);
	
	
	//发送数据
	Send_data(UART4,(unsigned char *)Analysis_food1);
	Send_data(UART4,(unsigned char *)Analysis_food2);
	Send_data(UART4,(unsigned char *)Analysis_food3);
	Send_data(UART4,(unsigned char *)Analysis_food4);
	
	Send_data(UART4,(unsigned char *)Analysis_num1_1);
	Send_data(UART4,(unsigned char *)Analysis_num2_1);
	Send_data(UART4,(unsigned char *)Analysis_num3_1);
	Send_data(UART4,(unsigned char *)Analysis_num4_1);
	Send_data(UART4,(unsigned char *)Analysis_num5_1);

}

/*
************************************************************
*	函数名称：	Compute_food
*
*	函数功能：	计算八个菜系上传数量排名
*
*	入口参数：	
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/
//void Compute_food(char food)
//{
//  if()
//}



/*
************************************************************
*	函数名称：	Compute_style
*
*	函数功能：	计算八个菜系上传数量排名
*
*	入口参数：	
*
*	返回参数：	无
*
*	说明：		
************************************************************
*/

int max1,m1,max2,m2,min1,n1,min2,n2;

void Compute_style(void)
{
	int i;
	max1=min1=style_num[0];
	max2=min2=style_num[1];
	m1=m2=n1=n2=0;
	//选出最大
  for(i=1;i<7;i++)
	{
		if(style_num[i]>=max1)
		{
			max2=max1;//次大
			m2=m1;//次大序号
			
			max1=style_num[i];//最大
			m1=i;//最大的序号		
		}
				
		if(style_num[i]<=min1)
		{
			min2=min1;//次小
			n2=n1;//次小序号
			
			min1=style_num[i];//最小
			n1=i;//最小序号	
		}
	}
	
		for(i=0;i<7;i++)//再次遍历选出次值
		{
			if(style_num[i]>=max2 && style_num[i]!=max1)//大于次大且不等于最大
			{																						//排除最大值不更换则次大值不更换的现象
				max2=style_num[i];//次大
				m2=i;//次大序号
			}
			
			if(style_num[i]<=min2 && style_num[i]!=min1)//同上选出次小值
			{
				min2=style_num[i];//次小
				n2=i;//次小序号
			}				
		}
	
}
/*
************************************************************
*	函数名称：	Data_analysis_recommendations_2
*
*	函数功能：	数据分析建议计算
*
*	入口参数：	9个入口参数
							A菜系，B菜系
							C菜系，D菜系
							预计销售额1元，实际销售额2元
							偏差3元，差异比4%，预计明日销售额5元
*
*	返回参数：	无
*
*	说明：		收到菜单后分析完一并发送至主机屏幕
						W1~6
************************************************************
*/
char style_A[]="减脂餐";
char style_B[]="家常菜";
char style_C[]="凉菜";
char style_D[]="速食";
char style_E[]="汤类";
char style_F[]="酒水";
char style_G[]="主食";

char style_1[100];
char style_2[100];
char style_3[100];
char style_4[100];


void Data_analysis_recommendations_2(void)
{
	Compute_style();
	
	switch(m1)
	{
		case 0 : strcpy(style_1,style_A);		break;
		case 1 : strcpy(style_1,style_B);		break;
		case 2 : strcpy(style_1,style_C);		break;
		case 3 : strcpy(style_1,style_D);		break;
		case 4 : strcpy(style_1,style_E);		break;
		case 5 : strcpy(style_1,style_F);		break;
		case 6 : strcpy(style_1,style_G);		break;
		
		default : break;
	}

	switch(m2)
	{
		case 0 : strcpy(style_2,style_A);		break;
		case 1 : strcpy(style_2,style_B);		break;
		case 2 : strcpy(style_2,style_C);		break;
		case 3 : strcpy(style_2,style_D);		break;
		case 4 : strcpy(style_2,style_E);		break;
		case 5 : strcpy(style_2,style_F);		break;
		case 6 : strcpy(style_2,style_G);		break;
		
		default : break;
	}

	switch(n1)
	{
		case 0 : strcpy(style_3,style_A);		break;
		case 1 : strcpy(style_3,style_B);		break;
		case 2 : strcpy(style_3,style_C);		break;
		case 3 : strcpy(style_3,style_D);		break;
		case 4 : strcpy(style_3,style_E);		break;
		case 5 : strcpy(style_3,style_F);		break;
		case 6 : strcpy(style_3,style_G);		break;
		
		default : break;
	}

	switch(n2)
	{
		case 0 : strcpy(style_4,style_A);		break;
		case 1 : strcpy(style_4,style_B);		break;
		case 2 : strcpy(style_4,style_C);		break;
		case 3 : strcpy(style_4,style_D);		break;
		case 4 : strcpy(style_4,style_E);		break;
		case 5 : strcpy(style_4,style_F);		break;
		case 6 : strcpy(style_4,style_G);		break;
		
		default : break;
	}

	Data_analysis_recommendations_1(style_1,style_2,style_3,style_4,total_price);
	
//	memset(style_num,0,8);
}
/*
************************************************************
*	函数名称：	Food_delivery
*
*	函数功能：	发送至屏幕显示送餐信号
*
*	入口参数：	餐桌号
*
*	返回参数：	无
*
*	说明：		收到人工呼叫后发送至主机屏幕
						V1~6
************************************************************
*/

//送餐显示餐桌号
unsigned char Table_num_1[20]={0x6D,0X61,0X69,0X6E,0X2E,0x74,0x31,0x30,0x2E,0x74,0x78,0x74,0x3D,0x22,0x31,0x22,0xff,0xff,0xff};
unsigned char Table_num_2[12]={0x76,0x69,0x73,0x20,0x74,0x38,0x2C,0x31,0xff,0xff,0xff};
unsigned char Table_num_3[13]={0x76,0x69,0x73,0x20,0x74,0x31,0x30,0x2C,0x31,0xff,0xff,0xff};
unsigned char Table_num_4[13]={0x76,0x69,0x73,0x20,0x74,0x31,0x31,0x2C,0x31,0xff,0xff,0xff};	
unsigned char Table_num_5[13]={0x76,0x69,0x73,0x20,0x62,0x31,0x32,0x2C,0x31,0xff,0xff,0xff};	


void Food_delivery(unsigned char num)
{
	
	Table_num_1[14]=num;
	
	Send_data(UART4,Table_num_1);
	Send_data(UART4,Table_num_2);
	Send_data(UART4,Table_num_3);
	Send_data(UART4,Table_num_4);
	Send_data(UART4,Table_num_5);
}



/*
************************************************************
*	函数名称：	Take_meal
*
*	函数功能：	发送至屏幕显示收餐信号
*
*	入口参数：	餐桌号
*
*	返回参数：	无
*
*	说明：		收到人工呼叫后发送至主机屏幕
						X1~6
************************************************************
*/

//收餐显示餐桌号
unsigned char TableNum_1[20]={0x6D,0X61,0X69,0X6E,0X2E,0x74,0x31,0x32,0x2E,0x74,0x78,0x74,0x3D,0x22,0x31,0x22,0xff,0xff,0xff};
unsigned char TableNum_2[12]={0x76,0x69,0x73,0x20,0x74,0x39,0x2C,0x31,0xff,0xff,0xff};
unsigned char TableNum_3[13]={0x76,0x69,0x73,0x20,0x74,0x31,0x32,0x2C,0x31,0xff,0xff,0xff};
unsigned char TableNum_4[13]={0x76,0x69,0x73,0x20,0x74,0x31,0x33,0x2C,0x31,0xff,0xff,0xff};	
unsigned char TableNum_5[13]={0x76,0x69,0x73,0x20,0x62,0x31,0x33,0x2C,0x31,0xff,0xff,0xff};	


void Take_meal(unsigned char num)
{
	
	TableNum_1[14]=num;
	
	Send_data(UART4,TableNum_1);
	Send_data(UART4,TableNum_2);
	Send_data(UART4,TableNum_3);
	Send_data(UART4,TableNum_4);
	Send_data(UART4,TableNum_5);
}


/*
************************************************************
*	函数名称：	Display_table_number
*
*	函数功能：	发送至屏幕显示桌号呼叫
*
*	入口参数：	餐桌号
*
*	返回参数：	无
*
*	说明：		收到人工呼叫后发送至主机屏幕
						Y1~6
************************************************************
*/

//人工工呼叫显示餐桌号
unsigned char Table_Num_1[19]={0x6D,0X61,0X69,0X6E,0X2E,0x74,0x32,0x2E,0x74,0x78,0x74,0x3D,0x22,0x31,0x22,0xff,0xff,0xff};
unsigned char Table_Num_2[12]={0x76,0x69,0x73,0x20,0x74,0x32,0x2C,0x31,0xff,0xff,0xff};
unsigned char Table_Num_3[12]={0x76,0x69,0x73,0x20,0x74,0x30,0x2C,0x31,0xff,0xff,0xff};
unsigned char Table_Num_4[13]={0x76,0x69,0x73,0x20,0x62,0x31,0x30,0x2C,0x31,0xff,0xff,0xff};	

void Display_tablenum(unsigned char num)
{
	
	Table_Num_1[13]=num;
	
	Send_data(UART4,Table_Num_1);
	Send_data(UART4,Table_Num_2);
	Send_data(UART4,Table_Num_3);
	Send_data(UART4,Table_Num_4);
}


/*******************************************************************************
* 函 数 名         : UART4_IRQHandler
* 函数功能		   : USART4中断函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/ 
void UART4_IRQHandler(void)                	//串口4中断服务程序
{
	
	// 设备短地址
	char robot_address[3] = {0x11,0x31}; // 机器人
//	char slave_address[3] = {0x11,0x21}; // 从机
	
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收中断
	{
		Uart4_Buffer[Uart4_Rx] = USART_ReceiveData(UART4);
		Uart4_Rx++; 
		
			//情景1：命令机器人送餐--V
			//情景2：命令机器人收餐--X
			//发送至机器人
			//情景3：数据分析指令--Z
			//返回数据分析信息
		if(Uart4_Buffer[Uart4_Rx-1] == 'z' || Uart4_Rx == Max_BUFF) 
		{
			
			if(Uart4_Buffer[0] == 'X' || Uart4_Buffer[0] == 'V')
			{
//				Voice_Announcements(2);		//语音播报
				Uart4_Buffer[Uart4_Rx-1]=0x00;
				Mesh_SendData(robot_address,(char *)Uart4_Buffer);
//				Send_data(UART4,(unsigned char *)Uart4_Buffer);
				
			}
			
			else if(Uart4_Buffer[0] == 'Z')
			{
				Uart4_Buffer[Uart4_Rx-1]=0x00;
				Data_analysis_recommendations_2();
			}

			
			Uart4_Rx=0;
			memset(Uart4_Buffer, 0, Max_BUFF);

		}
			
			//帧头不对则重新接收
		 if(Uart4_Buffer[0]!='V' &&
				Uart4_Buffer[0]!='X' &&
				Uart4_Buffer[0]!='Z') 
		 {
			 
			 Uart4_Rx=0;
			 memset(Uart4_Buffer, 0, Max_BUFF);
			 
		 }

		USART_ClearFlag(UART4,USART_FLAG_TC);
	} 
} 	


//==========================================================
//	函数名称：	HUAWEI_Public
//
//	函数功能：	华为云发布数据格式
//
//	入口参数：	
//
//	返回参数：	
//
//	说明：		
//==========================================================
void HUAWEI_Public(int temp, int humi)
{
	sprintf(mqtt_message,
	"{\"services\": [{\"service_id\": \"service2\",\"properties\":{\"temperature\":%d,\"humidity\": %d}}]}",
		temp,humi);
  OneNet_Publish(POST_TOPIC,mqtt_message);
}

void HUAWEI_Order_Public(void)
{
	sprintf(mqtt_message,
	"{\"services\": [{\"service_id\": \"service\",\"properties\":{\"TotalPrice\":%d,\"a\": %d,\"b\": %d,\"c\": %d,\"d\": %d,\"e\": %d,\"f\": %d,\"g\": %d,\"h\": %d,\"i\": %d,\"j\": %d,\"k\": %d,\"l\": %d,\"m\": %d,\"n\": %d,\"o\": %d,\"p\": %d,\"q\": %d,\"r\": %d,\"s\": %d,\"t\": %d,\"u\": %d}}]}",
		total_price,food_num[0],food_num[1],food_num[2],food_num[3],food_num[4],food_num[5],food_num[6],food_num[7],food_num[8],food_num[9],food_num[10],food_num[11],food_num[12],food_num[13],food_num[14],food_num[15],food_num[16],food_num[17],food_num[18],food_num[19],food_num[20]);
  OneNet_Publish(POST_TOPIC,mqtt_message);
}


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


char mqtt_message[200]; // �ϴ����ݻ�����


//��β��
char Order_end[5]={0x22,0xff,0xff,0xff};


int total_price;//�ܼ�
int style_num[8];//�߸���ϵ����


/*******************************************************************************
* �� �� ��         : UART4_Init
* ��������		   : USART5��ʼ������
* ��    ��         : bound:������
* ��    ��         : ��
*******************************************************************************/ 
void UART4_Init(u32 bound)
{
   //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);
 
	
	/*  ����GPIO��ģʽ��IO�� */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//TX			  
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;	    //�����������
	GPIO_Init(GPIOC,&GPIO_InitStructure);  /* ��ʼ����������IO */
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;//RX			 //��������
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;		  //ģ������
	GPIO_Init(GPIOC,&GPIO_InitStructure); /* ��ʼ��GPIO */
	

   //UART4 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(UART4, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(UART4, ENABLE);  //ʹ�ܴ���1 
	
	USART_ClearFlag(UART4, USART_FLAG_TC);
		
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//��������ж�

	//Uart4 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;//����4�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ�����	
}


/*
************************************************************
*	�������ƣ�	Clear_history_order
*
*	�������ܣ�	��ʷ����
*
*	��ڲ�����	���ţ��ܼ�


*
*	���ز�����	��
*
*	˵����		�յ��˵��������һ��������������Ļ
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
*	�������ƣ�	Order_history_1
*
*	�������ܣ�	��ʷ����
*
*	��ڲ�����	���ţ��ܼ�


*
*	���ز�����	��
*
*	˵����		�յ��˵��������һ��������������Ļ
						W1~6
************************************************************
*/
//�յ�������ʾ
unsigned char order_1[12]={0x76,0x69,0x73,0x20,0x74,0x35,0x2C,0x31,0xff,0xff,0xff};
unsigned char order_2[13]={0x76,0x69,0x73,0x20,0x62,0x31,0x31,0x2C,0x31,0xff,0xff,0xff};


//����
char Desk_num[19]={0x6C,0x73,0x64,0x64,0x2E,0x74,0x32,0x2E,0x74,0x78,0x74,0x3D,0x22,0x31,0x22,0xff,0xff,0xff};
//�ܼ�
char Total_price_2[10];

void Order_history_1(char desk_num)
{
	//�ܼ�
	char Total_price_1[30] ={0x6C,0x73,0x64,0x64,0x2E,0x74,0x33,0x2E,0x74,0x78,0x74,0x3D,0x22};
	//����
	Desk_num[13]=desk_num;
	
	Clear_history_order();
	
	//�ܼ�
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
*	�������ƣ�	Order_history_2
*
*	�������ܣ�	��ʷ����
*
*	��ڲ�����	order_num--�����ĵڼ���(int)
							food--�ĸ���Ʒ(char)
							��һ����+�˵ı����ʲô

*
*	���ز�����	��
*
*	˵����		�յ��˵��������һ��������������Ļ
						W1~6
						��Ҫ�õ�ѭ����������Ʒ���ü���Order_history_2
						
************************************************************
*/
//�˵�������
char food_a[]="�弦����";				int price_a=26;
char food_b[]="������";					int price_b=20;
char food_c[]="�߲�ɳ��";				int price_c=15;
char food_d[]="�Ǵ��Ź�";				int price_d=47;
char food_e[]="�������";				int price_e=56;
char food_f[]="��������";				int price_f=35;
char food_g[]="����ƹ�";				int price_g=12;
char food_h[]="�ɰ躣��";				int price_h=17;
char food_i[]="��ˮ��";					int price_i=30;
char food_j[]="�����";					int price_j=8;
char food_k[]="����";						int price_k=5;
char food_l[]="ˮ��";						int price_l=10;
char food_m[]="�Ź�������";			int price_m=40;
char food_n[]="���򶹸���";			int price_n=32;
char food_o[]="�ϲ˵�����";			int price_o=22;
char food_p[]="����";						int price_p=3;
char food_q[]="ơ��";						int price_q=4;
char food_r[]="��֭";						int price_r=5;
char food_s[]="���׷�";					int price_s=14;
char food_t[]="����";						int price_t=16;
char food_u[]="�׷�";						int price_u=6;

//��Ʒ
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

//����
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



//��Ʒ
char Order_food[100];		//����
char Order_food_2[100];	//ƴ�Ӳ���
//����
char Order_price[100];	//����
char Order_price_2[10];	//ƴ�Ӳ���
int price;


void Order_history_2(int order_num,char food)
{
	//��ղ˵�����
	memset(Order_food, 0, 100);
	memset(Order_food_2, 0, 100);
	
	switch(order_num)//���ݶ�����ͬ��Ÿ��Ʋ�Ʒ�ͼ۸�����Ļ�еĿո�λ��
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
	
	switch(food)//���ݲ�ͬ��Ʒ��ֵ���ĺͼ۸�
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
		
		//��Ʒ
		strcat(Order_food,Order_food_2);
		strcat(Order_food,Order_end);
	
		//����
		memset(Order_price_2, 0, 10);
		sprintf(Order_price_2,"%d",price);
		strcat(Order_price,Order_price_2);
		strcat(Order_price,Order_end);
		//��������
		Send_data(UART4,(unsigned char *)Order_food);
		Send_data(UART4,(unsigned char *)Order_price);

}

/*
************************************************************
*	�������ƣ�	Compute_Total_price
*
*	�������ܣ�	����������������ܼ�
*
*	��ڲ�����	���ţ��ܼ�


*
*	���ز�����	��
*
*	˵����		�յ��˵��������һ��������������Ļ
						W1~6
************************************************************
*/
int food_num[21];//ʳ������

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
*	�������ƣ�	Data_analysis_recommendations_1
*
*	�������ܣ�	���ݷ�������
*
*	��ڲ�����	9����ڲ���
							A��ϵ��B��ϵ
							C��ϵ��D��ϵ
							Ԥ�����۶�1Ԫ��ʵ�����۶�2Ԫ
							ƫ��3Ԫ�������4%��Ԥ���������۶�5Ԫ
*
*	���ز�����	��
*
*	˵����		�յ��˵��������һ��������������Ļ
						W1~6
************************************************************
*/
//���۶�
char Analysis_num1_2[10];
char Analysis_num2_2[10];
char Analysis_num3_2[10];
//char Analysis_num4_2[10];
char Analysis_num5_2[10];
	

void Data_analysis_recommendations_1(char * food1,char * food2,char * food3,char * food4,int num2)
{
	int num1,num3,num5;
//	int num4;
	
	//��������
	char Analysis_num1_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x31,0X33,0x2E,0x74,0x78,0x74,0x3D,0x22};	
	char Analysis_num2_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x31,0X36,0x2E,0x74,0x78,0x74,0x3D,0x22};	
	char Analysis_num3_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x31,0X38,0x2E,0x74,0x78,0x74,0x3D,0x22};	
	char Analysis_num4_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x32,0X32,0x2E,0x74,0x78,0x74,0x3D,0x22,0x34,0x22,0xff,0xff,0xff};	
	char Analysis_num5_1[30] = {0x73,0X6A,0X66,0X78,0X2E,0x74,0x32,0X33,0x2E,0x74,0x78,0x74,0x3D,0x22};	
	//��ϵ
	char Analysis_food1[30]={0x73,0X6A,0X66,0X78,0X2E,0x74,0x31,0x2E,0x74,0x78,0x74,0x3D,0x22};
	char Analysis_food2[30]={0x73,0X6A,0X66,0X78,0X2E,0x74,0x33,0x2E,0x74,0x78,0x74,0x3D,0x22};
	char Analysis_food3[30]={0x73,0X6A,0X66,0X78,0X2E,0x74,0x36,0x2E,0x74,0x78,0x74,0x3D,0x22};
	char Analysis_food4[30]={0x73,0X6A,0X66,0X78,0X2E,0x74,0x38,0x2E,0x74,0x78,0x74,0x3D,0x22};	
		
	//ƴ�Ӳ�ϵ
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

	
	//ƴ�����۶�
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
	
	
	//��������
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
*	�������ƣ�	Compute_food
*
*	�������ܣ�	����˸���ϵ�ϴ���������
*
*	��ڲ�����	
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
//void Compute_food(char food)
//{
//  if()
//}



/*
************************************************************
*	�������ƣ�	Compute_style
*
*	�������ܣ�	����˸���ϵ�ϴ���������
*
*	��ڲ�����	
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/

int max1,m1,max2,m2,min1,n1,min2,n2;

void Compute_style(void)
{
	int i;
	max1=min1=style_num[0];
	max2=min2=style_num[1];
	m1=m2=n1=n2=0;
	//ѡ�����
  for(i=1;i<7;i++)
	{
		if(style_num[i]>=max1)
		{
			max2=max1;//�δ�
			m2=m1;//�δ����
			
			max1=style_num[i];//���
			m1=i;//�������		
		}
				
		if(style_num[i]<=min1)
		{
			min2=min1;//��С
			n2=n1;//��С���
			
			min1=style_num[i];//��С
			n1=i;//��С���	
		}
	}
	
		for(i=0;i<7;i++)//�ٴα���ѡ����ֵ
		{
			if(style_num[i]>=max2 && style_num[i]!=max1)//���ڴδ��Ҳ��������
			{																						//�ų����ֵ��������δ�ֵ������������
				max2=style_num[i];//�δ�
				m2=i;//�δ����
			}
			
			if(style_num[i]<=min2 && style_num[i]!=min1)//ͬ��ѡ����Сֵ
			{
				min2=style_num[i];//��С
				n2=i;//��С���
			}				
		}
	
}
/*
************************************************************
*	�������ƣ�	Data_analysis_recommendations_2
*
*	�������ܣ�	���ݷ����������
*
*	��ڲ�����	9����ڲ���
							A��ϵ��B��ϵ
							C��ϵ��D��ϵ
							Ԥ�����۶�1Ԫ��ʵ�����۶�2Ԫ
							ƫ��3Ԫ�������4%��Ԥ���������۶�5Ԫ
*
*	���ز�����	��
*
*	˵����		�յ��˵��������һ��������������Ļ
						W1~6
************************************************************
*/
char style_A[]="��֬��";
char style_B[]="�ҳ���";
char style_C[]="����";
char style_D[]="��ʳ";
char style_E[]="����";
char style_F[]="��ˮ";
char style_G[]="��ʳ";

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
*	�������ƣ�	Food_delivery
*
*	�������ܣ�	��������Ļ��ʾ�Ͳ��ź�
*
*	��ڲ�����	������
*
*	���ز�����	��
*
*	˵����		�յ��˹����к�����������Ļ
						V1~6
************************************************************
*/

//�Ͳ���ʾ������
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
*	�������ƣ�	Take_meal
*
*	�������ܣ�	��������Ļ��ʾ�ղ��ź�
*
*	��ڲ�����	������
*
*	���ز�����	��
*
*	˵����		�յ��˹����к�����������Ļ
						X1~6
************************************************************
*/

//�ղ���ʾ������
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
*	�������ƣ�	Display_table_number
*
*	�������ܣ�	��������Ļ��ʾ���ź���
*
*	��ڲ�����	������
*
*	���ز�����	��
*
*	˵����		�յ��˹����к�����������Ļ
						Y1~6
************************************************************
*/

//�˹���������ʾ������
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
* �� �� ��         : UART4_IRQHandler
* ��������		   : USART4�жϺ���
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/ 
void UART4_IRQHandler(void)                	//����4�жϷ������
{
	
	// �豸�̵�ַ
	char robot_address[3] = {0x11,0x31}; // ������
//	char slave_address[3] = {0x11,0x21}; // �ӻ�
	
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //�����ж�
	{
		Uart4_Buffer[Uart4_Rx] = USART_ReceiveData(UART4);
		Uart4_Rx++; 
		
			//�龰1������������Ͳ�--V
			//�龰2������������ղ�--X
			//������������
			//�龰3�����ݷ���ָ��--Z
			//�������ݷ�����Ϣ
		if(Uart4_Buffer[Uart4_Rx-1] == 'z' || Uart4_Rx == Max_BUFF) 
		{
			
			if(Uart4_Buffer[0] == 'X' || Uart4_Buffer[0] == 'V')
			{
//				Voice_Announcements(2);		//��������
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
			
			//֡ͷ���������½���
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
//	�������ƣ�	HUAWEI_Public
//
//	�������ܣ�	��Ϊ�Ʒ������ݸ�ʽ
//
//	��ڲ�����	
//
//	���ز�����	
//
//	˵����		
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


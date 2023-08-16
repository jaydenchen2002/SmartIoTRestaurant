#include "mesh.h"
#include "usart.h"
#include "delay.h"
#include "hmi.h"
#include "beep.h"
#include "syn6288.h"
#include "timer2.h"



//C��
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

// �豸�̵�ַ
//char host_address[3] = {0x11,0x11};  // ����
//char slave_address[3] = {0x11,0x21}; // �ӻ�
//char robot_address[3] = {0x11,0x31}; // ������


/*
************************************************************
*	�������ƣ�	Usart3_Init
*
*	�������ܣ�	����3��ʼ��
*
*	��ڲ�����	baud���趨�Ĳ�����
*
*	���ز�����	��
*
*	˵����		TX-PB10		RX-PB11
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
	usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//��Ӳ������
	usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						//���պͷ���
	usartInitStruct.USART_Parity = USART_Parity_No;									//��У��
	usartInitStruct.USART_StopBits = USART_StopBits_1;								//1λֹͣλ
	usartInitStruct.USART_WordLength = USART_WordLength_8b;							//8λ����λ
	USART_Init(USART3, &usartInitStruct);
	
	USART_Cmd(USART3, ENABLE);														//ʹ�ܴ���
	USART_ClearFlag(USART3, USART_FLAG_TC);	
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);									//ʹ�ܽ����ж�
	
	nvicInitStruct.NVIC_IRQChannel = USART3_IRQn;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&nvicInitStruct);

}
/*
************************************************************
*	�������ƣ�	Send_data
*
*	�������ܣ�	���ڷ�������
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		u8Ϊunsigned char
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
*	�������ƣ�	Mesh_Init
*
*	�������ܣ�	��ʼ������ģ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		�����̵�ַ��1111
						�ӻ��̵�ַ��1121
					�����˶̵�ַ��1131
************************************************************
*/
void Mesh_Init(void)
{
	Usart3_Init(9600);
	
	//���ù㲥��
	Send_data(USART3, (unsigned char *)"AT+NAMEJDY-24M\r\n");		
	DelayXms(500);
	//����NETID,������ͳһ
	Send_data(USART3, (unsigned char *)"AT+NETID6968\r\n");		
	DelayXms(500);
	//���������̵�ַ,ÿ���豸��һ��
	Send_data(USART3, (unsigned char *)"AT+MADDR1111\r\n");		
	DelayXms(500);
	//reset
	Send_data(USART3, (unsigned char *)"AT+RESET\r\n");		
	DelayXms(1000);
}
/*
************************************************************
*	�������ƣ�	Mesh_SendData
*
*	�������ܣ�	Mesh��������
*
*	��ڲ�����	*address: ����Ŀ���ַ
							*data��		������������
*
*	���ز�����	��
*
*	˵����		�㲥Ŀ���ַΪ��0xFF,0XFF
************************************************************
*/
void Mesh_SendData(char *address,char *data)
{
char MESH_send_data[50]={0x41,0x54,0x2B,0x4D,0x45,0x53,0x48,0x01};//֡ͷ"AT+MESH"+0x01��ʾ��ACKӦ��
//char MESH_send_data[50]="AT+MESH";//֡ͷ"AT+MESH"
char MESH_end[4]={0xFF,0x0D,0X0A};//0xFF��Ϊ���ս����ж�λ��֡β��0x0D,0x0A

strcat(MESH_send_data,address);//����֡ͷ�뷢��Ŀ���ַ
strcat(MESH_send_data,data);//���ӷ�������
strcat(MESH_send_data,MESH_end);//����֡β

Send_data(USART3, (unsigned char *)MESH_send_data);
}

/*
************************************************************
*	�������ƣ�	USART3_IRQHandler
*
*	�������ܣ�	����3�շ��ж�
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		���ݽ��ո�ʽ��0xF1,0xDD + ʣ�����ݳ��� + ���ͷ���ַ��2�ֽڣ� + ���շ��̵�ַ��2�ֽڣ� + ��������
************************************************************
*/
void USART3_IRQHandler(void)
{
	
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //�����ж�
	{
		
		Uart3_Buffer[Uart3_Rx] = USART_ReceiveData(USART3);
		Uart3_Rx++; 
		
		if(Uart3_Buffer[Uart3_Rx-1] == 0xFF || Uart3_Rx == Max_BUFF_Len) // ���ڻ�����������������½���
		{
			
			if(Uart3_Buffer[0] == 0xF1 && Uart3_Buffer[1] == 0xDD) //֡ͷ��ʽ��ȷ
			{
				
				while(Uart3_Buffer[MESH_Data+7] != 0xFF) //�Զ��巢�Ͷ˽�����0xFF
				{
					MESH_Buffer[MESH_Data] = Uart3_Buffer[MESH_Data+7];  // �������ݴ����MESH_Buffer��,����ΪMESH_Data
					USART_SendData(USART_DEBUG,MESH_Buffer[MESH_Data+7]); // �������ݴ�ӡ������1
					while(USART_GetFlagStatus(USART_DEBUG,USART_FLAG_TC)==RESET);
					
					MESH_Data++;
					
												// ����500��������ѭ��
												timeout++;
												if(timeout == 500)
												{
													timeout = 0;
													Send_data(USART1,(unsigned char *)"��ѭ����");
													break;		
												}
					
				}
				
				if(MESH_Buffer[0] == 'Y')//�龰1�������˹�����
				{												 //��������ʾ������Ϣ
					Voice_Announcements(3);		//��������
					Display_tablenum(MESH_Buffer[1]);
//					beep=0;
//					DelayXms(250);
//					beep=1;
				}
				
				else if(MESH_Buffer[0] == 'X') //�龰2�������ղ�-X
				{															 //����Ļ��ʾ������Ϣ�������������л�����
					Voice_Announcements(5);	//��������
					Take_meal(MESH_Buffer[1]);
//					beep=0;
//					DelayXms(250);
//					beep=1;
				}
				
				else if(MESH_Buffer[0] == 'V') //�龰3�������Ͳ�-V
				{															 //����Ļ��ʾ������Ϣ�������������л�����
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
					Voice_Announcements(4);	//��������
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
					
					DATA_falg=1;				// ���ƶ��ϴ����ݱ�־λ
					
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
		
		if(Uart3_Buffer[0] != 0xF1) // ��һ���ֽڲ������½���
		{
			Uart3_Rx=0;
			memset(Uart3_Buffer, 0, Max_BUFF_Len);
		}
		
		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}
		
}

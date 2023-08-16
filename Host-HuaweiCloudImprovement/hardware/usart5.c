#include "system.h"
#include "usart5.h"
#include "timer3.h"

#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;

};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
	x = x;
}
//�ض���fputc����
int fputc(int ch, FILE *f)
{
	while((UART5->SR & 0X40) == 0); //ѭ������,ֱ���������
	UART5->DR = (u8) ch;
	return ch;
}
#endif

void UART5_SendData(u8 data)
{
	while((UART5->SR & 0X40) == 0);
	UART5->DR = data;
}

void UART5_SendString(u8 *DAT, u8 len)
{
	u8 i;
	for(i = 0; i < len; i++)
	{
		UART5_SendData(*DAT++);
	}
}

#if EN_UART5_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 UART5_RX_BUF[UART5_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
u16 UART5_RX_STA = 0;     //����״̬���

void UART5_Init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);	//ʹ��USRT4��GPIOCʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	//USRT4_TX   GPIOC.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO

	//USRT4_RX	  GPIOC.11��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIO

	//USRT4 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ; //��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(UART5, &USART_InitStructure); //��ʼ������1
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ���1

	TIM3_Int_Init(99, 7199);		//10ms�ж�
	UART5_RX_STA = 0;		//����
	TIM_Cmd(TIM3, DISABLE);			//�رն�ʱ��7
}

void UART5_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
	{
		Res = USART_ReceiveData(UART5);	//��ȡ���յ�������
		if((UART5_RX_STA & 0x8000) == 0) //����δ���
		{
			if(UART5_RX_STA < UART5_REC_LEN)	//�����Խ�������
			{
				TIM_SetCounter(TIM3, 0); //���������          				//���������
				if(UART5_RX_STA == 0) 				//ʹ�ܶ�ʱ��7���ж�
				{
					TIM_Cmd(TIM3, ENABLE); //ʹ�ܶ�ʱ��7
				}
				UART5_RX_BUF[UART5_RX_STA++] = Res;	//��¼���յ���ֵ
			}
			else
			{
				UART5_RX_STA |= 1 << 15;				//ǿ�Ʊ�ǽ������
			}
		}
	}
}
#endif


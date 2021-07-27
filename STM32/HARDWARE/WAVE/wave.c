/************************************************
Ӳ����ALIENTEK��ӢSTM32�����塢OpenMV��ATK-ESP8266��HC-SR04��
���ӣ�
--------------------------	
|  HC-SR04  |   STM32    |
| :-------: | :--------: |
| 	 TRIG   |    PE5     |
| 	 ECHO   |    PE56    |
|    VCC    |    +5V     |
|    GND    |    GND     |
--------------------------
���򣺳�����������
��汾��ST3.5.0
 
���ߣ�л���� @kelecn
���ڣ�2021��03��25��
************************************************/

#include "wave.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
//#include "esp8266.h"

#define Trig GPIO_Pin_5

#define Echo GPIO_Pin_6

float Distance;

void Wave_SRD_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitSture;
	EXTI_InitTypeDef  EXTI_InitSture;
	NVIC_InitTypeDef  NVIC_InitSture;
	//����ⲿ�жϵĻ���һ��ʹ��AFIO���ù���
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOE,ENABLE);
	
	
	//����IO�˿�
	GPIO_InitSture.GPIO_Mode=GPIO_Mode_Out_PP;   //�������ģʽ
	GPIO_InitSture.GPIO_Pin=Trig;                //��PE4��Trig����
	GPIO_InitSture.GPIO_Speed=GPIO_Speed_50MHz;  
	GPIO_Init(GPIOE,&GPIO_InitSture);
	
	GPIO_InitSture.GPIO_Mode=GPIO_Mode_IPD;      //������ģʽ
	GPIO_InitSture.GPIO_Pin=Echo;                //��PE6��Echo����
	GPIO_InitSture.GPIO_Speed=GPIO_Speed_50MHz;  
	GPIO_Init(GPIOE,&GPIO_InitSture);
	
	//�жϺ�6�˿�ӳ��һ��
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource6);
	
	//�ⲿ�ж�����
	EXTI_InitSture.EXTI_Line=EXTI_Line6;
	EXTI_InitSture.EXTI_LineCmd=ENABLE;
	EXTI_InitSture.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitSture.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitSture);
	
	
	//�ж����ȼ�����
	NVIC_InitSture.NVIC_IRQChannel=EXTI9_5_IRQn;
	NVIC_InitSture.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitSture.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitSture.NVIC_IRQChannelSubPriority=2;
	NVIC_Init(&NVIC_InitSture);
}

void EXTI9_5_IRQHandler(void)
{
	delay_us(10);
	
	
	if(EXTI_GetITStatus(EXTI_Line6)!=RESET)
	{
		TIM_SetCounter(TIM4,0);
		TIM_Cmd(TIM4,ENABLE);
		
		while(GPIO_ReadInputDataBit(GPIOE,Echo));  //�ȴ��͵�ƽ
		
		TIM_Cmd(TIM4,DISABLE);
		
		Distance=TIM_GetCounter(TIM4)*340/200.0;
		
		if(Distance>0)
		{
		//printf("Distance:%f cm\r\n",Distance);
		//	u3_printf("{\"z\":%d}",(int)Distance);
		}
			
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
}

void Wave_SRD_Strat(void)
{
	GPIO_SetBits(GPIOE,Trig);   //��Trig����Ϊ�ߵ�ƽ
	delay_us(20);               //��������10us����������������ģ�鹤��
	GPIO_ResetBits(GPIOE,Trig); 
	
}












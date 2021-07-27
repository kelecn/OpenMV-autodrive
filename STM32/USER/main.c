/************************************************
Ӳ����ALIENTEK��ӢSTM32�����塢OpenMV��ATK-ESP8266��HC-SR04��		
���򣺡�����OpenMV�����˼�ʻ����С��ģ��ϵͳ�����س���
��汾��ST3.5.0
���ߣ�л���� @kelecn
���ڣ�2021��03��25��
************************************************/

#include "led.h"
#include "delay.h"
#include "sys.h"
#include "beep.h"
#include "timer.h"
#include "wave.h"
#include "sys.h"
#include "pwm.h"
#include "usart.h"
#include "openmv.h"
#include "esp8266.h"
#include "stdio.h"
#include <stdlib.h>

u16 rlen=0,aaa=0;                                 //����WIFI�������鳤��
u8 aw[10]={48,49,50,51,52,53,54,55,56,57};        //ASCII��0123456789
u8 flag=0;                                        //��������WIFIָ��
int Position_PID(int a, int b);                   //Ԥ����PID������ 
void autodrive (int a, int b,int PID);            //Ԥ�����Զ���ʻģʽ        

 int main(void)
 {		
	 int z =0;                                      //��װ����X��Y��Distance
	 int d=0;                                       //Distance ȡ��
   int a,b=0;                                     //PWM���Ƴ�ʼ��ֵ
	 float PID=0;                                   //PID���������PWMֵ
	 
	delay_init();	    	 														//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	  												  	//����1��ʼ��Ϊ9600
 	LED_Init();		  		 														//��ʼ����LED���ӵ�Ӳ���ӿ�
	BEEP_Init();         	 													//��ʼ���������˿�
	usart2_init(115200);  												  //OpenMV��ʼ��Ϊ115200
	usart3_init(115200);  												  //ESP8266��ʼ��Ϊ115200
	Timer_SRD_Init(5000,7199);											//���ڳ������Ķ�ʱ��4��ʼ��
	Wave_SRD_Init();                                //HC-SR04������ģ���ʼ��
	while(1)
	{	 		
		Wave_SRD_Strat();                             //�����������ź�
		d=Distance;                                   //Distance ȡ��
		z= 10000*0+100*0+d;                           //·�����ݴ��
		a=880;b=870;                                  //�����ٶ� ���ҵ��ת�ٲ�һ�������1000
		u3_printf("{\"z\":%d}",z);                    //����·������
		PID=Position_PID(50,Y);                       //PID������
		BEEP=0;
		LED0=1;
		switch(flag){
			case 0:Go_stop ();break;						//ͣ��
			case 1:Go_forward(a,b);break;				//ǰ��
			case 2:Go_right(a-100,b-100);break; //��ת
		  case 3:Go_left(a-100,b-100);break;  //��ת
			case 4:Go_back(a,b);break;          //����
			case 5:autodrive(a,b,PID);break;    //�Զ�
		}
//WIFI�������ݺ���
		 if(USART3_RX_STA&(1<<15))	//���յ�����
			                          //[15]:0,û�н��յ�����;1,���յ���һ������.
                                //[14:0]:���յ������ݳ���
	   {	
      rlen=USART3_RX_STA&0X7FFF;//�õ����ν��յ������ݳ���
			USART3_RX_BUF[rlen]=0;		//��ӽ����� 
			if(USART3_RX_BUF[rlen-1]==aw[0])     //����0  ֹͣ ASCII��48 
			{
				flag=0;
			}
			else if(USART3_RX_BUF[rlen-1]==aw[1])//����1 ǰ��
			{
				flag=1;
			}
			else if(USART3_RX_BUF[rlen-1]==aw[2])//����2 ��ת
			{
				flag=2;
			}
			else if(USART3_RX_BUF[rlen-1]==aw[3])//����3 ��ת
			{
				flag=3;
			}
      else if(USART3_RX_BUF[rlen-1]==aw[4])//����4 ����
			{
				flag=4;
			}
      else if(USART3_RX_BUF[rlen-1]==aw[5])//����5 �Զ�ģʽ 
     {
        flag=5;
      }
	    //�����־λ
			delay_ms(100);		
			USART3_RX_STA=0;
	   }
	}
}
//�Զ���ʻģʽ ���̵��Լ�����ͬʱʶ�� PID  ���ڳɱ�����,û����������ͷ����,��ͨ��־ʶ���ܵ������Լ���.
void autodrive (int a, int b, int PID)
	{
    if(Distance<20)          //�򵥱���
	 {
		      BEEP=1;
					Go_stop();
		}else if (Distance>20)
		{
			    BEEP=0;
		if(X==1)                //���ͣ��
		{
			    LED0=0;
					Go_stop();
		}else if (X==0||X==2)   //�޺��̵ƻ����̵Ƽ�����ʻ ��������
		{
			   LED0=1;
		 if(Y>50)               //С��ƫ��  
		 {
		  Go_forward(PID,b);
		 }else if(Y<50)         //С��ƫ��  
			{
			Go_forward(a,PID);
			}
		}
	}
}

//λ��ʽPID������
int Position_PID (int Encoder,int Target) //Ŀ��ֵ50  ����ֵY�����Ƕ�
  { 	
	 float Position_KP=160,Position_KI=0.01,Position_KD=90;
   static float Bias,Pwm,Integral_bias,Last_Bias;
   Bias=Encoder-Target; //����ƫ��
   Integral_bias+=Bias; //���ƫ��Ļ���	     
   Pwm=Position_KP*Bias+Position_KI*Integral_bias+Position_KD*(Bias-Last_Bias);       
   Last_Bias=Bias;       //������һ��ƫ�� 
   return Pwm;           //���
  }


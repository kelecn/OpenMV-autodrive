/************************************************
Ӳ����ALIENTEK��ӢSTM32�����塢OpenMV��ATK-ESP8266��HC-SR04��
���ӣ�
--------------------------	
|  OpenMV   |   STM32    |
| :-------: | :--------: |
| P4(TXD)   | PA3(RXD)   |
| P5(RXD)   | PA2(TXD)   |
|    VCC    |    +5V     |
|    GND    |    GND     |
|    GND    |    GND     |
--------------------------
����OpenMVͨѶ����
��汾��ST3.5.0
 
���ߣ�л���� @kelecn
���ڣ�2021��03��25��
************************************************/

#include "sys.h"
#include "openmv.h"
#include "led.h"
#include "delay.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif 
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout2;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit2(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputcc(int ch, FILE *f)
{      
	while((USART2->SR&0X40)==0);//ѭ������,ֱ���������   
    USART2->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART2_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF2[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA2=0;       //����״̬���	  
  
void usart2_init(u32 bound){
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
     
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);    //ʹ��USART2,GPIOAʱ��//USART2_TX   GPIOA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    //�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.2
   
    //USART2_RX      GPIOA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.3  

    //USART2 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        //�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);    //����ָ���Ĳ�����ʼ��NVIC�Ĵ���//USART ��ʼ������

    USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;    //�շ�ģʽ

    USART_Init(USART2, &USART_InitStructure); //��ʼ������2
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2
}
  
extern int ov_frame;
    
u8 X,Y;    //�������� X->0������ 1��� 2�̵�;Y->�������50(�м�)
   
void Data_Processing(u8 *data_buf,u8 num)
    
{
     
 int theta_org,rho_org;
    
    theta_org = (int)(*(data_buf+1)<<0) | (int)(*(data_buf+2)<<8) | (int)(*(data_buf+3)<<16) | (int)(*(data_buf+4)<<24) ;
     
    X = theta_org;
    
     
     
    rho_org = (int)(*(data_buf+5)<<0) | (int)(*(data_buf+6)<<8) | (int)(*(data_buf+7)<<16) | (int)(*(data_buf+8)<<24) ;
     
    Y = rho_org;
      
}
   
     
void Receive_Prepare(u8 data)
     
{
    
    static u8 RxBuffer[10];
    
    static u8  _data_cnt = 0;
    
    static u8 state = 0;
     
    if(state==0 && data==0xAA)
    
    {
          //LED0 =0;
			    //LED1 =1;
          state=1;
		      //delay_ms(1000); 
			   
     
    }
     
    else if(state==1 && data==0xAE)
     
    {
        //LED0 =1;
			  // LED1 =0;
        state=2;
			  // delay_ms(1000); 
      
        _data_cnt = 0;
     
    }
     
    else if(state==2)
      
    {
     
        RxBuffer[++_data_cnt]=data;
     
        if(_data_cnt>=8)
    
        { 
     
            state = 0;
     
            Data_Processing(RxBuffer,_data_cnt);
     
        }
      
    }
     
    else
     
        state = 0;
       
}
     
      
      
void USART2_IRQHandler(void)    
       
{
     
 u8 temp;
      
 if( USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET )
     
 {
     
  USART_ClearITPendingBit(USART2,USART_IT_RXNE);
     
  temp = USART_ReceiveData(USART2);
    
  Receive_Prepare(temp);
         
 }
       
}
#endif

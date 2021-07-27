#ifndef __ESP8266_H
#define __ESP8266_H	 
#include "sys.h"  
/************************************************
Ӳ����ALIENTEK��ӢSTM32�����塢OpenMV��ATK-ESP8266��HC-SR04��
���򣺴���3����
���ߣ�л���� @kelecn
���ڣ�2021��03��25��
************************************************/  

#define USART3_MAX_RECV_LEN		600					//�����ջ����ֽ���
#define USART3_MAX_SEND_LEN		600					//����ͻ����ֽ���
#define USART3_RX_EN 			1					//0,������;1,����.

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern vu16 USART3_RX_STA;   						//��������״̬

void usart3_init(u32 bound);				//����3��ʼ�� 
void u3_printf(char* fmt,...);
#endif














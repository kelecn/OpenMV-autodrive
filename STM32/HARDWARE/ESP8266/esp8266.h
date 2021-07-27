#ifndef __ESP8266_H
#define __ESP8266_H	 
#include "sys.h"  
/************************************************
硬件：ALIENTEK精英STM32开发板、OpenMV、ATK-ESP8266、HC-SR04等
程序：串口3定义
作者：谢青桦 @kelecn
日期：2021年03月25日
************************************************/  

#define USART3_MAX_RECV_LEN		600					//最大接收缓存字节数
#define USART3_MAX_SEND_LEN		600					//最大发送缓存字节数
#define USART3_RX_EN 			1					//0,不接收;1,接收.

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern vu16 USART3_RX_STA;   						//接收数据状态

void usart3_init(u32 bound);				//串口3初始化 
void u3_printf(char* fmt,...);
#endif














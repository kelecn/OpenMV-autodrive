#ifndef __PWM_H
#define	__PWM_H

#include "stm32f10x.h"

void TIM3_PWM_Init(u16 CCR1_Val,u16 CCR2_Val,u16 CCR3_Val,u16 CCR4_Val);//四路PWM输出
void Go_forward(u16 a,u16 b);//前进
void Go_back(u16 a,u16 b);	 //后退
void Go_right(u16 a,u16 b);	 //右转	
void Go_left(u16 a,u16 b);	 //左转
void Go_stop(void);	 //停止

#endif /* __PWM_H */


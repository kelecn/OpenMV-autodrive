#ifndef __PWM_H
#define	__PWM_H

#include "stm32f10x.h"

void TIM3_PWM_Init(u16 CCR1_Val,u16 CCR2_Val,u16 CCR3_Val,u16 CCR4_Val);//��·PWM���
void Go_forward(u16 a,u16 b);//ǰ��
void Go_back(u16 a,u16 b);	 //����
void Go_right(u16 a,u16 b);	 //��ת	
void Go_left(u16 a,u16 b);	 //��ת
void Go_stop(void);	 //ֹͣ

#endif /* __PWM_H */


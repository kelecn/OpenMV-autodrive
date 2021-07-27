#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

void TIM4_Int_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
void Timer_SRD_Init(u16 arr,u16 psc);
/////////////////////////////////////
void TIM7_Int_Init(u16 arr,u16 psc);
#endif

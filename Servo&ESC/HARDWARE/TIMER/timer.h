#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

extern TIM_HandleTypeDef TIM4_Handler;      //��ʱ����� 

void TIM4_Init(u16 arr,u16 psc);
#endif


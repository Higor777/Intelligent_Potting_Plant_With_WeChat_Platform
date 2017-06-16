#ifndef __TIMER_H
#define __TIMER_H	 
#include "sys.h"

		 	
void TIMER_PWM_Init(u16 arr,u16 psc);      


void Motor_PWM(int32_t PWM0,int32_t PWM1,int32_t PWM2);
    
#endif

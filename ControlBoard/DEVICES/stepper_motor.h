#ifndef _STEPPER_MOTOR_H_
#define _STEPPER_MOTOR_H_

#include"stm32f10x.h"


void lightAndWatering(void);
void Light_On_Off(uint8_t Switch);
void watering(uint32_t step_t);

 void Stepper_motor(void);


//步进电机
#define ln1(a)	if (a)	\
					GPIO_SetBits(GPIOB,GPIO_Pin_8);\
					else		\
					GPIO_ResetBits(GPIOB,GPIO_Pin_8)

#define ln2(a)	if (a)	\
					GPIO_SetBits(GPIOC,GPIO_Pin_13);\
					else		\
					GPIO_ResetBits(GPIOC,GPIO_Pin_13)

#endif
                    
                    


#include "stm32f10x.h"
#include "delay.h"
#include "stepper_motor.h"
#include "os.h"
void lightAndWatering(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);					
    GPIO_ResetBits(GPIOC,GPIO_Pin_5);		
    GPIO_ResetBits(GPIOC,GPIO_Pin_7);	    
}
 

void Light_On_Off(uint8_t Switch)
{
    if (Switch==0)
    {
         GPIO_ResetBits(GPIOC,GPIO_Pin_5);
    }else if(Switch==1)
    {
         GPIO_SetBits(GPIOC,GPIO_Pin_5);
    }
}


void TIM2_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能定时器2时钟
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
    
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); //使能指定的TIM2中断,允许更新中断

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM2, DISABLE);  //使能TIMx外设
							 
}

void TIM8_PWM_Init(u16 arr,u16 psc)
{ 
    
    
    GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE );
	   
 

    GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_6 ;		
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;		   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
 
   //初始化TIM8
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period =arr;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;	
	
    TIM_OC1Init(TIM8, &TIM_OCInitStructure);	
	
	
	TIM_CtrlPWMOutputs(TIM8, ENABLE);
	
	TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable); 
	TIM_SetCompare1(TIM8,arr/2);
}



volatile int step;                       //步进数
volatile int flag=0,hold=0;     


void TIM2_IRQHandler(void)   //TIM3中断
{  
    static int count=0;
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
            TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
			count++;
            
			if (count>step)
			{
                count=0;
                step=0;
                flag=1;
                TIM_Cmd(TIM8, DISABLE);
                TIM_Cmd(TIM2, DISABLE);
                GPIO_ResetBits(GPIOC,GPIO_Pin_7);
            }
		}
}



void direction(int a)        //正为正转，负为反转
{	 

    if(a>0)
	 {
			ln1(1);             //使能
			ln2(1);
	 }
	if(a<0)
	 {

		 ln1(1);
		 ln2(0);
	 }
}



 void Stepper_motor(void)
{
    TIM_Cmd(TIM2, ENABLE);  //使能TIM2
    TIM_Cmd(TIM8, ENABLE);
}



void watering(uint32_t step_t)              		
{
    GPIO_SetBits(GPIOC,GPIO_Pin_7);   
    hold=0;
    //direction(1); 
    step=step_t;
    TIM2_Int_Init(999,30);
    TIM8_PWM_Init(999,30);   
    Stepper_motor();

}



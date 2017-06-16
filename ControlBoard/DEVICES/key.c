#include "key.h"
#include "delay.h"

volatile uint8_t KEY_Scan_STA=0;
//KEY_Scan_STA含义：
//Bit		  7		  6	  	5		  4		  3		  2	  	1		  0
//Key		更新标识	NC		NC		KEY5	KEY4	KEY3	KEY2	KEY1
//1表示有效，0表示无效

volatile uint8_t M_Encoder_STA=0;
//M_Encoder_STA含义：
//Bit	   	7		  6		  5	  	4	  	3		  2	  	1	   	0
//Key		更新标识	NC		NC		NC		NC		NC		CCW		CW
//1表示有效，0表示无效


/*********************************************
* 按键IO扫描
*********************************************/
void Key_Scan(void)
{
	static uint8_t keysta_old=0;
	uint8_t keysta_new=0x00;
    keysta_new = (((~GPIO_ReadInputData(GPIOF))>>1)&0x0F);

	
#if USE_PIT_KEYSCAN==0
	
	keysta_old=keysta_new;
	keysta_new=0x00;
	
	DelayMs(10);	//消除抖动
	

    keysta_new = (((~GPIO_ReadInputData(GPIOF))>>1)&0x0F);
	
#endif
	

	KEY_Scan_STA=keysta_new&keysta_old;
	
	KEY_Scan_STA|=KEY_Scan_STA_Update_MASK;		//添加标识位
	
	keysta_old=keysta_new;
}


/*********************************************
* 按键IO初始化
*********************************************/
void Key_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);	                    //使能PF端口时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;  //KEY1234端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	 //浮空输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOF, &GPIO_InitStructure);					 //根据设定参数初始化GPIOF

	Key_Scan();	//初始化KeyUp_old,KeyDown_old
	
#if USE_PIT_KEYSCAN!=0
	delay_ms(10);
	Key_Scan();
#endif
}




/******************************************************
*按键状态
*这个函数用于实现多按键连按输出
******************************************************/
uint8_t Key_STA(void)
{
	static uint8_t keysta_mask=0xFF;
	static uint8_t keysta_mask_old=0xFF;
	static uint8_t keysta_cnt=0;
	
	uint8_t keysta;
//	uint8_t keysta_out;
	
#if USE_PIT_KEYSCAN==0
	
	Key_Scan();
	
#endif
	
	if(KEY_Scan_STA&0x80)
	{
		keysta=KEY_Scan_STA&0x3F;
		KEY_Scan_STA=0;
	}
	else 
		return 0;
	
	
	//解除MASK
	keysta_mask|=~keysta;
	keysta_mask_old|=~keysta;
	
	//cnt清零
 	if(keysta_mask&keysta)
	{
		keysta_cnt=0;
		keysta_mask_old=keysta_mask;
	}
	
	//输出keysta
	if(keysta_cnt>=Key_KeepTime)
	{
		keysta&=keysta_mask_old;
		keysta_cnt-=Key_KeepStep;
	}
	else
	{
		keysta&=keysta_mask;
		keysta_cnt++;
	}
	
	//设定MASK
	keysta_mask&=~keysta;
	
	return keysta;
}

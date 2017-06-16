#include "stm32f10x.h"
#include "iwatchdog.h"

void IWDG_Configuration(void)
{
    //写入0x5555,用于允许看门狗寄存器写入功能
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    //看门狗时钟分频,40K/4=10K HZ(0.1ms)
    IWDG_SetPrescaler(IWDG_Prescaler_4);

    //喂狗时间 50ms/0.1MS=500 .注意不能大于0xfff
    IWDG_SetReload(500);

    //喂狗
    IWDG_ReloadCounter();

    //使能看门狗
    IWDG_Enable();
}


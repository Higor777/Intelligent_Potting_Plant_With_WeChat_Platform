#include "led.h"
#include "delay.h"
#include "sys.h"
#include "includes.h"

//UCOSIII中以下优先级用户程序不能使用，ALIENTEK
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		512
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
__align(8) CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);



//任务优先级
#define Move_TASK_PRIO		4
//任务堆栈大小
#define Move_STK_SIZE		512
//任务控制块
OS_TCB	MoveTaskTCB;
//任务堆栈
__align(8) CPU_STK	Move_TASK_STK[Move_STK_SIZE];
//任务函数
void Move_task(void *p_arg);

//任务优先级
#define RC_TASK_PRIO		4
//任务堆栈大小
#define RC_STK_SIZE		512
//任务控制块
OS_TCB	RCTaskTCB;
//任务堆栈
__align(8) CPU_STK	RC_TASK_STK[RC_STK_SIZE];
//任务函数
void RC_task(void *p_arg);


//任务优先级
#define Light_TASK_PRIO		4
//任务堆栈大小
#define Light_STK_SIZE		128
//任务控制块
OS_TCB	LightTaskTCB;
//任务堆栈
__align(8) CPU_STK	Light_TASK_STK[Light_STK_SIZE];
//任务函数
void Light_task(void *p_arg);


//任务优先级
#define Metal_Sensor_TASK_PRIO		4
//任务堆栈大小
#define Metal_Sensor_STK_SIZE		512
//任务控制块
OS_TCB	Metal_SensorTaskTCB;
//任务堆栈
__align(8) CPU_STK	Metal_Sensor_TASK_STK[Metal_Sensor_STK_SIZE];
//任务函数
void Metal_Sensor_task(void *p_arg);


//任务优先级
#define UI_TASK_PRIO		4
//任务堆栈大小
#define UI_STK_SIZE		256
//任务控制块
OS_TCB	UITaskTCB;
//任务堆栈
__align(8) CPU_STK	UI_TASK_STK[UI_STK_SIZE];
//任务函数
void UI_task(void *p_arg);


float system_time_100ms = 0.0;

//用于测试按键调参
float test_para1 = 1;
float test_para2 = 1;
uint16_t device_id_1=0;
uint16_t device_id_2=0;
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
    
	delay_init();       //延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    USART_Module_Init();
    LCD_Init();
    ParameterList_Init();
    Key_Init();
    
    TIMER_PWM_Init(1000,6);
    temperature_soil_moisture_init();
    TSL2561_Init(TIMING_402MS);
    lightAndWatering();
    
    
    //mfrc522_Init();

    /*
    while(device_id_1 == 0){
        device_id_1 = LDC1614_1_Init(); 
    }
    while(device_id_2 == 0){
        device_id_2 = LDC1614_2_Init();    
    }
	*/
    
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*2=2ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//进入临界区

			 
	//创建Move任务
	OSTaskCreate((OS_TCB 	* )&MoveTaskTCB,		
				 (CPU_CHAR	* )"Move task", 		
                 (OS_TASK_PTR )Move_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )Move_TASK_PRIO,     	
                 (CPU_STK   * )&Move_TASK_STK[0],	
                 (CPU_STK_SIZE)Move_STK_SIZE/10,	
                 (CPU_STK_SIZE)Move_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )1,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);		
	//创建RC任务
	OSTaskCreate((OS_TCB 	* )&RCTaskTCB,		
				 (CPU_CHAR	* )"RC task", 		
                 (OS_TASK_PTR )RC_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )RC_TASK_PRIO,     	
                 (CPU_STK   * )&RC_TASK_STK[0],	
                 (CPU_STK_SIZE)RC_STK_SIZE/10,	
                 (CPU_STK_SIZE)RC_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )1,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
	//创建Metal_Sensor任务
                 /*
	OSTaskCreate((OS_TCB 	* )&Metal_SensorTaskTCB,		
				 (CPU_CHAR	* )"Metal_Sensor task", 		
                 (OS_TASK_PTR )Metal_Sensor_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )Metal_Sensor_TASK_PRIO,     	
                 (CPU_STK   * )&Metal_Sensor_TASK_STK[0],	
                 (CPU_STK_SIZE)Metal_Sensor_STK_SIZE/10,	
                 (CPU_STK_SIZE)Metal_Sensor_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )1,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);	
 */  
	//创建Light任务
	OSTaskCreate((OS_TCB 	* )&LightTaskTCB,		
				 (CPU_CHAR	* )"Light task", 		
                 (OS_TASK_PTR )Light_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )Light_TASK_PRIO,     	
                 (CPU_STK   * )&Light_TASK_STK[0],	
                 (CPU_STK_SIZE)Light_STK_SIZE/10,	
                 (CPU_STK_SIZE)Light_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )1,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
              

	//创建UI任务
	OSTaskCreate((OS_TCB 	* )&UITaskTCB,		
				 (CPU_CHAR	* )"UI task", 		
                 (OS_TASK_PTR )UI_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )UI_TASK_PRIO,     	
                 (CPU_STK   * )&UI_TASK_STK[0],	
                 (CPU_STK_SIZE)UI_STK_SIZE/10,	
                 (CPU_STK_SIZE)UI_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )1,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
    //IWDG_Configuration();
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//进入临界区
}


//Move任务
void Move_task(void *p_arg)
{
    OS_ERR err;
	while(1)
	{
        
        move_base_control();
        OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err);//延时5ms,延时会进行任务切换
        //OSSchedRoundRobinYield(&err);
	}
}

//RC任务
void RC_task(void *p_arg)
{
    OS_ERR err;
	while(1)
	{
        //IWDG_ReloadCounter();
        system_time_100ms = OSTimeGet(&err)*0.02;//100ms
        //在时间片内处理消息队列，如果消息队列为空，则让出空闲时间
        while(RCMsg_Hander())
        {
            ;
        }
        OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err);//延时5ms,延时会进行任务切换
		//OSSchedRoundRobinYield(&err);
	}
}

extern uint32_t L1,L2,L3,L4,R1,R2,R3,R4;
extern int16_t error;
//Metal_Sensor任务
void Metal_Sensor_task(void *p_arg)
{
    OS_ERR err;
    uint8_t flag = 0;
    unsigned char str[100];
	while(1)
	{
        flag ++;
        getline_L(&flag);
        sprintf((char*)str,"L: %d %d %d %d",L1,L2,L3,L4);
        LCD_ShowString(0,150,350,24,24,str);
        sprintf((char*)str,"R: %d %d %d %d",R1,R2,R3,R4);
        LCD_ShowString(0,180,350,24,24,str);
        sprintf((char*)str,"Error: %-9d",error);
        LCD_ShowString(0,210,350,24,24,str);
        
        
        OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err);//延时5ms,延时会进行任务切换
		//OSSchedRoundRobinYield(&err);
	}
}


//Light任务
void Light_task(void *p_arg)
{
    OS_ERR err;
    uint16_t illumination_data= 0;
    float temperature_data= 0;
    uint16_t soil_moisture_data= 0;
    unsigned char str[100];
	while(1)
	{
        
		illumination_data= get_illumination();
        sprintf((char*)str,"illumination: %d lux",illumination_data);
        LCD_ShowString(0,240,300,24,24,str);
        temperature_data = get_temperature();
        sprintf((char*)str,"temperature: %3.1f ",temperature_data);
        LCD_ShowString(0,270,300,24,24,str);
        soil_moisture_data = get_soil_moisture();
        sprintf((char*)str,"soil_moisture: %d %%",soil_moisture_data);
        LCD_ShowString(0,300,300,24,24,str);
        USART_printf(USART1,"|i:%d lux|t:%2.1f |s:%d%%|",illumination_data,temperature_data,soil_moisture_data);
        
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err);//延时500ms,延时会进行任务切换
        
		//OSSchedRoundRobinYield(&err);
	}
}


//UI任务
void UI_task(void *p_arg)
{
    OS_ERR err;
	float system_time_s = 0;
    unsigned char str[100];
    //uint8_t status;
    //uint8_t g_ucTempbuf[20];  
	while(1)
	{
		

        system_time_s = system_time_100ms*0.1;
        sprintf((char*)str,"Time : %d h %d m %d s.",(int)system_time_s/3600,(int)system_time_s/60,(int)system_time_s%60);
		POINT_COLOR=RED;	  
        LCD_ShowString(0,0,300,24,24,str); 
        /*
        LCD_ShowString(30,70,200,16,16,"TFTLCD UI");
        LCD_ShowString(30,90,200,16,16,"zz");
        sprintf((char*)str,"UI1: %f    UI2: %f",test_para1,test_para2);
        LCD_ShowString(30,110,200,16,16,str);		//显示LCD ID
        LCD_ShowString(30,130,200,12,12,"2017/4/9");
        */
        
        Key_Scan();
        Parameter_Change();

        
        
        //status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡
        //sprintf((char*)str,"Status1:%d",status);
        //LCD_ShowString(0,30,300,24,24,str); 
        //sprintf((char*)str,"Card type:%x %x",g_ucTempbuf[0],g_ucTempbuf[1]);
        //LCD_ShowString(0,60,300,24,24,str); 
        //status = PcdAnticoll(g_ucTempbuf);//防冲撞
        //sprintf((char*)str,"Card value:%x %x %x %x",g_ucTempbuf[0],g_ucTempbuf[1],g_ucTempbuf[2],g_ucTempbuf[3]);
        //LCD_ShowString(0,90,300,24,24,str); 
        //sprintf((char*)str,"Status2:%d",status);
        //LCD_ShowString(0,120,300,24,24,str);
    

                    
                    
		OSTimeDlyHMSM(0,0,0,1,OS_OPT_TIME_HMSM_STRICT,&err); //延时5ms,延时会进行任务切换
	}
}



 


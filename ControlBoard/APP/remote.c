#include <math.h>
#include "remote.h"
#include "queue.h"
#include "usart.h"
#include "transform.h"
#include "stepper_motor.h"
#include "os.h"
#define RC_MsgMAX_LEN 50

#define SPEED   0x10
#define CMD     0x20
extern float system_time_100ms;
extern DMA_USART_Ch DMA_USART_Ch1;
#define RC_Msg_Rx_Queue DMA_USART_Ch1.rx_queue

extern robot Omnidirectional_Robot;
float Speed_out_time = 0;
float out_time = 10;
//速度缩放比率
#define SPEED_RATIO 300.0
void dataHander(uint8_t * data)
{
    OS_ERR err;
    system_time_100ms = OSTimeGet(&err)*0.02;//100ms
    if(((*data)&SPEED) != 0)
    {
        Speed_out_time = system_time_100ms+out_time;
        data++;
        Omnidirectional_Robot.robot_expected_speed.x = *((float*)data)*SPEED_RATIO;
        Omnidirectional_Robot.robot_expected_speed.y = *((float*)(data+4))*SPEED_RATIO;
        Omnidirectional_Robot.robot_expected_speed.z = *((float*)(data+8))*SPEED_RATIO;
    }else if(((*data)&CMD) != 0)
    {
        data++;
        switch (*data) {
            case 0x01:
                Speed_out_time = system_time_100ms+out_time;
                Omnidirectional_Robot.robot_expected_speed.x = SPEED_RATIO;
                Omnidirectional_Robot.robot_expected_speed.y = 0;
                Omnidirectional_Robot.robot_expected_speed.z = 0;
                break;
            case 0x02:
                Speed_out_time = system_time_100ms+out_time;
                Omnidirectional_Robot.robot_expected_speed.x = -SPEED_RATIO;
                Omnidirectional_Robot.robot_expected_speed.y = 0;
                Omnidirectional_Robot.robot_expected_speed.z = 0;
                break;
            case 0x03:
                Speed_out_time = system_time_100ms+out_time;
                Omnidirectional_Robot.robot_expected_speed.x = 0;
                Omnidirectional_Robot.robot_expected_speed.y = -SPEED_RATIO;
                Omnidirectional_Robot.robot_expected_speed.z = 0;
                break;
            case 0x04:
                Speed_out_time = system_time_100ms+out_time;
                Omnidirectional_Robot.robot_expected_speed.x = 0;
                Omnidirectional_Robot.robot_expected_speed.y = SPEED_RATIO;
                Omnidirectional_Robot.robot_expected_speed.z = 0;
                break;
            case 0x05:
                Speed_out_time = system_time_100ms+out_time;
                Omnidirectional_Robot.robot_expected_speed.x = 0;
                Omnidirectional_Robot.robot_expected_speed.y = 0;
                Omnidirectional_Robot.robot_expected_speed.z = 0;
                break;
            case 0x06:
                Speed_out_time = system_time_100ms+out_time;
                Omnidirectional_Robot.robot_expected_speed.x = 0;
                Omnidirectional_Robot.robot_expected_speed.y = 0;
                Omnidirectional_Robot.robot_expected_speed.z = SPEED_RATIO;
                break;
            case 0x07:
                Speed_out_time = system_time_100ms+out_time;
                Omnidirectional_Robot.robot_expected_speed.x = 0;
                Omnidirectional_Robot.robot_expected_speed.y = 0;
                Omnidirectional_Robot.robot_expected_speed.z = -SPEED_RATIO;
                break;
            case 0x08:
                Light_On_Off(1);
                break;
            case 0x09:
                Light_On_Off(0);
                break;
            case 0x0A:
                watering(17777);
                break;
            default :
                break;
        }
    }
}
void timeoutHander(void)
{
    if(Speed_out_time < system_time_100ms)
    {
        Omnidirectional_Robot.robot_expected_speed.x = 0;
        Omnidirectional_Robot.robot_expected_speed.y = 0;
        Omnidirectional_Robot.robot_expected_speed.z = 0;
    }
}


enum recstate //状态机 
{
	RECFF1,RECLEN1,RECSEL,RECCHECK
}RC_Recstate = RECFF1;


//返回值为消息队列是否空 1未空  0已空
uint8_t RCMsg_Hander(void)
{
    static uint16_t checksum;
	static uint16_t RCdata_len;
	static uint32_t RCcur=0;
	static uint8_t RCbuffer[RC_MsgMAX_LEN];
	static uint8_t data;

	timeoutHander();
    
	if(queue_empty(&RC_Msg_Rx_Queue))
		return 0;

	data = (uint8_t)queue_get(&RC_Msg_Rx_Queue);
	switch (RC_Recstate) 
	{			
		case RECFF1:
			if (data == 0x55)	  
			{	
				RC_Recstate = RECLEN1;	
				RCdata_len = 0;
				RCcur = 0;	
                checksum=0x55;
			}
			break;
			
		case RECLEN1:	
			if((data & 0x0F) <= 0x0C)
			{		 
                RCbuffer[RCcur++] = data;			//暂存数据
				RCdata_len = (data & 0x0F);
                checksum+=data;
				RC_Recstate = RECSEL;	 
                
			}else
			{
				RC_Recstate = RECFF1;
			}
			break;
			
			
		case RECSEL:
			RCbuffer[RCcur++] = data;			//暂存数据
            checksum+=data;
			if(RCcur >= RCdata_len+1)
			{
				RC_Recstate = RECCHECK;
			}
			break;

		case RECCHECK:
            checksum=checksum%255;
			if(checksum == data)
			{
				dataHander(RCbuffer);
                checksum=0;
                RC_Recstate = RECFF1;
			}else
            {
                RC_Recstate = RECFF1;
            }
			break;
		
		default:
			RC_Recstate = RECFF1;
			break;
	}	
    return 1;
}




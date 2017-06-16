#include <math.h>
#include "transform.h"
#include "timer.h"




/*************************************************
函数  void Robot_To_Motor_tf(float Motor[3] , float *Global ,float R_theta)   
**************************************************/

void Robot_To_Motor_tf(float Robot[3] , float *Motor )   
{
	*Motor = 0 * Robot[0] + 1 * Robot[1] + radius * Robot[2] ;
	*(Motor + 1) =  -0.866025f* Robot[0] - 0.5f * Robot[1] + radius * Robot[2] ;
	*(Motor + 2) =  0.866025f * Robot[0] - 0.5f * Robot[1] + radius * Robot[2] ;

}

/******************
设置机器人坐标系速度函数
******************/
#define ADJUST  0.5
void Set_Speed_Robot_To_Single(robot * bot)
{
    static float Robot[3],Speed[3] ;	
    Robot[0] = bot->robot_expected_speed.x ;
    Robot[1] = bot->robot_expected_speed.y ;
    Robot[2] = -bot->robot_expected_speed.z ;
    
	Robot_To_Motor_tf( Robot , Speed ) ;

	bot->single_expected_speed.M1=Speed[0];
	bot->single_expected_speed.M2=Speed[1];
	bot->single_expected_speed.M3=Speed[2];
}

/***************
底盘控制
**************/

robot Omnidirectional_Robot;
void move_base_control()
{
   
    Set_Speed_Robot_To_Single(&Omnidirectional_Robot);

    //Motor_PWM(400,400,400);
    Motor_PWM((int32_t)Omnidirectional_Robot.single_expected_speed.M1,(int32_t)Omnidirectional_Robot.single_expected_speed.M2,(int32_t)Omnidirectional_Robot.single_expected_speed.M3);
}

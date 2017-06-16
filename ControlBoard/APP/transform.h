#ifndef __TRANSFORM_H
#define __TRANSFORM_H	


#define radius 0.5    //全向轮中心到机器人质心的距离
#define  pi  3.1415926


typedef struct motor{
	float M1;
	float M2;
	float M3;
} motor;


typedef struct vector{
	float x;
	float y;
	float z;
}vector;


typedef struct robot{
	//速度
    motor single_expected_speed;    //每个电机的速度大小
    vector robot_expected_speed;    //底盘的期望速度大小
}robot;


void move_base_control(void);
#endif

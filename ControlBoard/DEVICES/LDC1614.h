/*
   启真 
   西北工业大学——电轨组
   第十一届智能汽车竞赛
   LDC1614测试程序
   2016.7.29
   by.LST
*/
#ifndef __LDC1614_H__
#define __LDC1614_H__

#include "sys.h"
#include "stdint.h"


//连接LDC1614用到的I2C通道
#define LDC1614_1_I2CX    (I2C2)

#define LDC1614_1_I2CX_GPIO_RCC RCC_APB2Periph_GPIOB
#define LDC1614_1_I2CX_RCC      RCC_APB1Periph_I2C2

#define LDC1614_1_I2CX_GPIO GPIOB
//连接LDC1614用到的SCL引脚
#define LDC1614_1_SCLPIN  (GPIO_Pin_10)
//连接LDC1614用到的SDA引脚
#define LDC1614_1_SDAPIN  (GPIO_Pin_11)




//连接LDC1614用到的I2C通道
#define LDC1614_2_I2CX    (I2C0)

//连接LDC1614用到的SCL引脚
#define LDC1614_2_SCLPIN  (PTB2)

//连接LDC1614用到的SDA引脚
#define LDC1614_2_SDAPIN  (PTB3)


/********LDC1614寄存器地址**********/

////分频系数寄存器
#define CLOCK_DIVIDERS_CH0         0x14   //[15:12] CH0_FIN_DIVIDER：1 ---- [9:0] CH0_FREF_DIVIDER ：2   0x100A
#define CLOCK_DIVIDERS_CH1         0x15 
#define CLOCK_DIVIDERS_CH2         0x16 
#define CLOCK_DIVIDERS_CH3         0x17 
////准备时间寄存器
#define SETTLECOUNT_CH0            0x10  //[15:0] :10 0x000A
#define SETTLECOUNT_CH1            0x11
#define SETTLECOUNT_CH2            0x12
#define SETTLECOUNT_CH3            0x13
////
#define RCOUNT_CH0                 0x08  //[15:0] :238  0x00EE
#define RCOUNT_CH1                 0x09
#define RCOUNT_CH2                 0x0A
#define RCOUNT_CH3                 0x0B

#define DRIVE_CURRENT_CH0          0x1E  //[15:11] CH0_IDRIVE : 7     ------CH0_INIT_IDRIVE [10:6] : 0   0x3800
#define DRIVE_CURRENT_CH1          0x1F
#define DRIVE_CURRENT_CH2          0x20
#define DRIVE_CURRENT_CH3          0x21

#define MUX_CONFIG                 0x1B  //[15] AUTOSCAN_EN : 1    [14:13] RR_SEQUENCE : b10  [2:0] DEGLITCH : b100
//0xC20C
#define CONFIG                     0x1A  //[15:14] ACTIVE_CHAN  : b00   [13] SLEEP_MODE_EN:0    [12]RP_OVERRIDE_EN:1 [11] SENSOR_ACTIVATE_SEL:1  [10]AUTO_AMP_DIS:0 [9]REF_CLK_SRC:1
//0x1E01
#define ERROR_CONFIG               0x19

#define DATA_MSB_CH0               0x00 //[27:16]DATA0 
#define DATA_LSB_CH0               0x01 //[15:0]
#define DATA_MSB_CH1               0x02 //[27:16]DATA0 
#define DATA_LSB_CH1               0x03 //[15:0]
#define DATA_MSB_CH2               0x04 //[27:16]DATA0 
#define DATA_LSB_CH2               0x05 //[15:0]
#define DATA_MSB_CH3               0x06 //[27:16]DATA0 
#define DATA_LSB_CH3               0x07 //[15:0]

#define STATUS                     0x18 //[6] DRDY b1: A new conversion result is ready.  [3] CH0_UNREADCONV b1: An unread conversion is present for Channel 0.

#define DEVICE_ID                  0x7F //Device ID = 0x3055

#define RESET_DEV                  0x1C


//=========LDC1614 功能参数==================//
#define LDC1614_1_DEV_ADDR   0x2A //Normally,can range 0x08 to 0xEF
#define LDC1614_1_DEV_WRITE  LDC1614_1_DEV_ADDR<<1 | 0x00
#define LDC1614_1_DEV_READ   LDC1614_1_DEV_ADDR<<1 | 0x01

#define LDC1614_2_DEV_ADDR   0x2B //Normally,can range 0x08 to 0xEF
#define LDC1614_2_DEV_WRITE  LDC1614_2_DEV_ADDR<<1 | 0x00
#define LDC1614_2_DEV_READ   LDC1614_2_DEV_ADDR<<1 | 0x01

#define LDC1614_DEV_ID     0x3055   //LDC1614设备ID

//=========LDC1614 STATUS寄存器 寄存器位==================//
#define STATUS_READY               0x0040
#define STATUS_CH0_UNREADCONV      0x0008
#define STATUS_CH1_UNREADCONV      0x0004
#define STATUS_CH2_UNREADCONV      0x0002
#define STATUS_CH3_UNREADCONV      0x0001


//定义SCL Bus Speed取值，外设总线为50Mhz时的计算结果
#define LDC1614_SCL_50KHZ                   (0x33) 
#define LDC1614_SCL_100KHZ                  (0x2B)  
#define LDC1614_SCL_150KHZ                  (0x28)
#define LDC1614_SCL_200KHZ                  (0x23)
#define LDC1614_SCL_250KHZ                  (0x21)
#define LDC1614_SCL_300KHZ                  (0x20)
#define LDC1614_SCL_400KHZ                  (0x17)  

//函数声明
uint16_t LDC1614_1_Init(void);
void  LDC1614_1_WriteReg(uint8_t, uint8_t, uint8_t);
uint16_t LDC1614_1_ReadReg(uint8_t);
uint32_t LDC1614_1_GetResult(uint8_t);
void LDC1614_1_GetAllResult(uint32_t *result);

uint16_t LDC1614_2_Init(void);
void  LDC1614_2_WriteReg(uint8_t, uint8_t, uint8_t);
uint16_t LDC1614_2_ReadReg(uint8_t);
uint32_t LDC1614_2_GetResult(uint8_t);
void LDC1614_2_GetAllResult(uint32_t *result);

#endif 


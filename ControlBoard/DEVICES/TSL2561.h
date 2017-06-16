#ifndef __TSL2561_H__
#define __TSL2561_H__

#include "sys.h"
#include "stdint.h"


#define TSL2561_ADDR0 0x29 // address with '0' shorted on board
#define TSL2561_ADDR  0x39 // default address 
#define TSL2561_ADDR1 0x49 // address with '1' shorted on board

#define TSL2561_CMD           0x80
#define TSL2561_TIMING        0x81
#define	TSL2561_REG_ID        0x8A
#define	TSL2561_DATA0_LOW     0x8C
#define TSL2561_DATA0_HIGH    0x8D
#define	TSL2561_DATA1_LOW     0x8E
#define TSL2561_DATA1_HIGH    0x8F

#define TSL2561_POWER_ON      0x03
#define TSL2561_POWER_DOWN    0x00

#define  TIMING_13MS         0x00     
#define  TIMING_101MS        0x01     
#define  TIMING_402MS        0x02     
#define  TIMING_GAIN_1X      0x10     
#define  TIMING_GAIN_16X     0x00    



/*
 * Constats for simplified lux calculation
 * according TAOS Inc.
 */
#define LUX_SCALE 14
#define RATIO_SCALE 9

#define CH_SCALE 10
#define CH_SCALE_TINT0 0x7517
#define CH_SCALE_TINT1 0x0FE7



/* 
 * T, FN, and CL Package coefficients
 */
#define TSL2561_K1T 0x0040
#define TSL2561_B1T 0x01F2
#define TSL2561_M1T 0x01BE
#define TSL2561_K2T 0x0080
#define TSL2561_B2T 0x0214
#define TSL2561_M2T 0x02D1
#define TSL2561_K3T 0x00C0
#define TSL2561_B3T 0x023F
#define TSL2561_M3T 0x037B
#define TSL2561_K4T 0x0100
#define TSL2561_B4T 0x0270
#define TSL2561_M4T 0x03FE
#define TSL2561_K5T 0x0138
#define TSL2561_B5T 0x016F
#define TSL2561_M5T 0x01fC
#define TSL2561_K6T 0x019A
#define TSL2561_B6T 0x00D2
#define TSL2561_M6T 0x00FB
#define TSL2561_K7T 0x029A
#define TSL2561_B7T 0x0018
#define TSL2561_M7T 0x0012
#define TSL2561_K8T 0x029A
#define TSL2561_B8T 0x0000
#define TSL2561_M8T 0x0000



/* 
 * CS package coefficients
 */
#define TSL2561_K1C 0x0043
#define TSL2561_B1C 0x0204
#define TSL2561_M1C 0x01AD
#define TSL2561_K2C 0x0085
#define TSL2561_B2C 0x0228
#define TSL2561_M2C 0x02C1
#define TSL2561_K3C 0x00C8
#define TSL2561_B3C 0x0253
#define TSL2561_M3C 0x0363
#define TSL2561_K4C 0x010A
#define TSL2561_B4C 0x0282
#define TSL2561_M4C 0x03DF
#define TSL2561_K5C 0x014D
#define TSL2561_B5C 0x0177
#define TSL2561_M5C 0x01DD
#define TSL2561_K6C 0x019A
#define TSL2561_B6C 0x0101
#define TSL2561_M6C 0x0127
#define TSL2561_K7C 0x029A
#define TSL2561_B7C 0x0037
#define TSL2561_M7C 0x002B
#define TSL2561_K8C 0x029A
#define TSL2561_B8C 0x0000
#define TSL2561_M8C 0x0000



void TSL2561_Write(uint8_t command,uint8_t date);
uint16_t TSL2561_Read(uint8_t command);
void TSL2561_PowerOn(void);
void TSL2561_PowerDown(void);
void TSL2561_TimingSet(uint8_t TIME);
void TSL2561_Init(uint8_t Time_model);
uint16_t TSL2561_Chanel0Read(void);
uint16_t TSL2561_Chanel1Read(void);

unsigned long tsl2561_compute_lux(int ch0, int ch1);

uint16_t get_illumination(void);
    
#endif 



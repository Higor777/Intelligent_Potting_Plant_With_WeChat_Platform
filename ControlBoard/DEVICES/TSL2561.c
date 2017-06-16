#include "myiic.h"
#include "TSL2561.h"

void TSL2561_Write(uint8_t command,uint8_t date)
{
	IIC_Start();
	IIC_Send_Byte(TSL2561_ADDR<<1);
	IIC_Wait_Ack();
	IIC_Send_Byte(command);
	IIC_Wait_Ack();
	IIC_Send_Byte(date);
	IIC_Wait_Ack();
	IIC_Stop();
}


uint16_t TSL2561_Read(uint8_t command)
{
	uint16_t date;
	IIC_Start();
	IIC_Send_Byte(TSL2561_ADDR<<1);
	IIC_Wait_Ack();
	IIC_Send_Byte(command);
	IIC_Wait_Ack();
	
	IIC_Start();
	IIC_Send_Byte((TSL2561_ADDR<<1)+1);
	IIC_Wait_Ack();
	date=IIC_Read_Byte(1);
	date+=(IIC_Read_Byte(1)<<8);
	IIC_Stop();
	return date;
}

void TSL2561_PowerOn(void)
{
	TSL2561_Write(TSL2561_CMD,TSL2561_POWER_ON);
}

void TSL2561_PowerDown(void)
{
	TSL2561_Write(TSL2561_CMD,TSL2561_POWER_DOWN);
}

void TSL2561_TimingSet(uint8_t TIME)
{
	IIC_Start();
	IIC_Send_Byte(TSL2561_ADDR<<1);
	IIC_Wait_Ack();
	IIC_Send_Byte(TSL2561_TIMING);
	IIC_Wait_Ack();
	IIC_Send_Byte(TIME);
	IIC_Wait_Ack();
	IIC_Stop();
}

void TSL2561_Init(uint8_t Time_model)
{
	IIC_Init();
	TSL2561_PowerOn();
	TSL2561_TimingSet(Time_model);
	
}

uint16_t TSL2561_Chanel0Read(void)
{
  uint16_t buff;
	buff=TSL2561_Read(TSL2561_DATA0_LOW);
	return buff;
}

uint16_t TSL2561_Chanel1Read(void)
{
  uint16_t buff;
	buff=TSL2561_Read(TSL2561_DATA1_LOW);
	return buff;
}


uint16_t get_illumination(void)
{
    uint16_t temp0,temp1;
    uint32_t lux;
    temp0 = TSL2561_Chanel0Read();
    temp1 = TSL2561_Chanel1Read();
    lux = tsl2561_compute_lux(temp0,temp1);
    return lux;
}


/*
 * Helper function for computing lux values.
 * It uses a lux equation approximation without floating point calculations.
 *
 */
unsigned long tsl2561_compute_lux(int ch0, int ch1) {

	 long ch_scale, channel0, channel1;
     long ratio = 0, ratio1 = 0;
    int b, m;
     long tmp ;
     long lux;
    
    ch_scale = (1 << CH_SCALE);
    

	// scale the channel values
	channel0 = (ch0 * ch_scale) >> CH_SCALE;
	channel1 = (ch1 * ch_scale) >> CH_SCALE;

	

	// find the ratio of the channel values (Channel1/Channel0)
	// protect against divide by zero
	if(channel0 != 0) 
		ratio1 = (channel1 << (RATIO_SCALE + 1)) / channel0;
	
	// round the ratio value
	ratio = (ratio1 + 1) >> 1;

	

    if((ratio >= 0) && (ratio <= TSL2561_K1T)){ 
        b = TSL2561_B1T; m = TSL2561_M1T;
    } else if(ratio <= TSL2561_K2T) {
        b = TSL2561_B2T; m = TSL2561_M2T;
    } else if(ratio <= TSL2561_K3T) {
        b = TSL2561_B3T; m = TSL2561_M3T;
    } else if(ratio <= TSL2561_K4T) {
        b = TSL2561_B4T; m = TSL2561_M4T;
    } else if (ratio <= TSL2561_K5T) {
        b = TSL2561_B5T; m = TSL2561_M5T;
    } else if(ratio <= TSL2561_K6T) {
        b = TSL2561_B6T; m = TSL2561_M6T;
    } else if(ratio <= TSL2561_K7T) {
        b = TSL2561_B7T; m = TSL2561_M7T;
    } else if(ratio > TSL2561_K8T) {
        b = TSL2561_B8T; m = TSL2561_M8T;
    }

	tmp = (channel0 * b) - (channel1 * m);

	if(tmp < 0) 
		tmp = 0;

	tmp += (1 << (LUX_SCALE-1));
	lux = (tmp >> LUX_SCALE);

	return lux;		
}

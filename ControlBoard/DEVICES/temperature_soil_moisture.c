 #include "temperature_soil_moisture.h"
 #include "delay.h"
 #include "os.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//ADC 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/7
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
   
void temperature_soil_moisture_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    Adc_Init();
    
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE );
    //一线制总线
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOG, &GPIO_InitStructure);					
    GPIO_SetBits(GPIOG,GPIO_Pin_11);	


}
		   
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3																	   
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
 


	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	//PA1 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
    //PC0 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //复位ADC1 

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

}				  
//获得ADC值
//ch:通道值 0~3
u16 Get_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(1);
	}
	return temp_val/times;
} 	 



#define DQ_IN()  {GPIOG->CRH&=0XFFFF0FFF;GPIOG->CRH|=(u32)8<<(4*3);}
#define DQ_OUT() {GPIOG->CRH&=0XFFFF0FFF;GPIOG->CRH|=(u32)3<<(4*3);}

//IO操作函数	 

#define DQ_OUT_DATA    PGout(11) 
#define DQ_READ_DATA   PGin(11)  

#define TIME_OUT  100


void DQ_Delayus(uint32_t us){
  uint32_t n,m;
  for(n=0;n<us;n++) {
    for(m=0;m<10;m++)m=m;
  }
}

//器件存在返回0，不存在返回1
uint8_t DQ_reset(void)
{
    int read_time = 0;
    DQ_OUT();
    DQ_OUT_DATA = 1;
    DQ_Delayus(5);
    DQ_OUT_DATA = 0;
    DQ_Delayus(700);
    DQ_OUT_DATA = 1;
    DQ_Delayus(20);
    DQ_IN();
    //传感器存在会把DQ拉低，超过100没拉低判断传感器未连接
    while(DQ_READ_DATA==1 && read_time<TIME_OUT)read_time++;
    if(read_time>=TIME_OUT)
        return 1;
    DQ_Delayus(250);
    return 0;
    
}


int WRITE1 = 10;
int WRITE2 = 50;

void DQ_write_1(void)
{
    DQ_OUT();
    DQ_OUT_DATA = 1;
    DQ_Delayus(5);
    DQ_OUT_DATA = 0;
    DQ_Delayus(15);
    DQ_OUT_DATA = 1;
    DQ_Delayus(45);
}

void DQ_write_0(void)
{
    DQ_OUT();
    DQ_OUT_DATA = 1;
    DQ_Delayus(5);
    DQ_OUT_DATA = 0;
    DQ_Delayus(60);
    DQ_OUT_DATA = 1;
}
int WRITE11 = 3;
int WRITE22 = 10;
uint8_t DQ_read(void)
{
    uint8_t data;
    DQ_OUT();
    DQ_OUT_DATA = 1;
    DQ_Delayus(5);
    DQ_OUT_DATA = 0;
    DQ_Delayus(20);
    DQ_OUT_DATA = 1;
    DQ_IN();
    data = DQ_READ_DATA;
    DQ_OUT();
    DQ_OUT_DATA = 1;
    return data;

}

void DQ_write_byte(uint8_t data)
{
    uint8_t i=0;
    for(i=0;i<8;i++)
    {
        if(data&0x01)
            DQ_write_1();
        else
            DQ_write_0();
        data = data>>1;
    }
}

uint16_t DQ_read_byte(void)
{
    uint16_t data=0,data_temp=0;
    uint8_t i=0;
    for(i=0;i<8;i++)
    {
        data_temp = data_temp>>1;
        if(DQ_read())
        {
            data_temp |= 0x80;
        }
        
        
        
    }
//    data |= data_temp>>8;
//    data |= data_temp<<8;
    data = data_temp;
    return data;
}

const char CRC8Table[]={
  0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
  157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
  190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
  219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
  101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
  202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
  87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};
 
unsigned char CRC8_Table( char *p, char counter)
{
    unsigned char crc8 = 0;
    for( ; counter > 0; counter--){
        crc8 = CRC8Table[crc8^*p];
        p++;
    }
    return(crc8);
}

float get_temperature(void)
{
    char temp[9],crc;
    float tempdata;
    static float last;
    uint8_t i = 0;
    OS_ERR err;
    CPU_SR_ALLOC();
    OSTimeDlyHMSM(0,0,0,1,OS_OPT_TIME_HMSM_STRICT,&err);//延时1ms,延时会进行任务切换
    OS_CRITICAL_ENTER();//进入临界区
    if(DQ_reset())return 77;
    DQ_write_byte(0xcc);//Skip ROM
    DQ_write_byte(0x44);//启动测温
    OS_CRITICAL_EXIT();	//退出临界区	 
    OSTimeDlyHMSM(0,0,0,750,OS_OPT_TIME_HMSM_STRICT,&err);//延时750ms,延时会进行任务切换
    OS_CRITICAL_ENTER();//进入临界区
    if(DQ_reset())return 77;
    DQ_write_byte(0xcc);//Skip ROM
    DQ_write_byte(0xbe);//开始读取数据
    for(i=0;i<9;i++)
    {
        temp[i] = DQ_read_byte();
    }
    OS_CRITICAL_EXIT();	//退出临界区	
    OSTimeDlyHMSM(0,0,0,1,OS_OPT_TIME_HMSM_STRICT,&err);//延时1ms,延时会进行任务切换    
    crc = CRC8_Table(temp,8);
    if(crc==temp[8])
    {
        tempdata=(float)(temp[0]+((temp[1]&0x0F)*256))/16.0;
        last = tempdata;
    }else
    {
        tempdata=last;
    }
    

    return tempdata;
}

u16 get_soil_moisture(void)
{
    u16 temp = 0;
    temp = 100-Get_Adc_Average(ADC_Channel_1,10)/41;
    return temp;
}



























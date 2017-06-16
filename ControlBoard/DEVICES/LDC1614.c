/*
   启真 
   西北工业大学——电轨组
   第十一届智能汽车竞赛
   LDC1614测试程序
   2016.7.29
   by.LST
*/
#include "LDC1614.h"
#include "myiic.h"

static void LDC1614_Delay(void);



/*
 *   LDC1614_1_Init
 *   该函数用于初始化LDC1614
 *
 *   参数：
 *   RegisterAddress 
 *    |__ void
 *   Data
 *    |__ device_id 
 */

uint16_t LDC1614_1_Init(void)
{
    uint16_t device_id;
    int cnt;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOG, ENABLE );	//使能GPIOG时钟
	   
    //设置地址0x2A
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOG,GPIO_Pin_1); 	//PG1 输出低
    
    IIC_Init();
    
    
    LDC1614_1_WriteReg(RESET_DEV, 0x80, 0x00);

    for(cnt=0;cnt<300;cnt++){
    LDC1614_Delay();
    }
    //读取设备ID
    device_id = LDC1614_1_ReadReg(DEVICE_ID);

    //进行寄存器配置
    if(device_id==0x3055){

    LDC1614_1_WriteReg(RCOUNT_CH0, 0x02, 0x20); 
    LDC1614_1_WriteReg(RCOUNT_CH1, 0x02, 0x20);
    LDC1614_1_WriteReg(RCOUNT_CH2, 0x02, 0x20);
    LDC1614_1_WriteReg(RCOUNT_CH3, 0x02, 0x20);

    LDC1614_1_WriteReg(SETTLECOUNT_CH0, 0x00, 0x1E);
    LDC1614_1_WriteReg(SETTLECOUNT_CH1, 0x00, 0x1E);
    LDC1614_1_WriteReg(SETTLECOUNT_CH2, 0x00, 0x1E);
    LDC1614_1_WriteReg(SETTLECOUNT_CH3, 0x00, 0x1E);

    LDC1614_1_WriteReg(CLOCK_DIVIDERS_CH0, 0x10, 0x03);
    LDC1614_1_WriteReg(CLOCK_DIVIDERS_CH1, 0x10, 0x03); 
    LDC1614_1_WriteReg(CLOCK_DIVIDERS_CH2, 0x10, 0x03);  
    LDC1614_1_WriteReg(CLOCK_DIVIDERS_CH3, 0x10, 0x03); 
     
    LDC1614_1_WriteReg(ERROR_CONFIG, 0x00, 0x01);

    LDC1614_1_WriteReg(DRIVE_CURRENT_CH0, 0x30, 0x00); ///0X70  
    LDC1614_1_WriteReg(DRIVE_CURRENT_CH1, 0x30, 0x00);
    LDC1614_1_WriteReg(DRIVE_CURRENT_CH2, 0x30, 0x00);
    LDC1614_1_WriteReg(DRIVE_CURRENT_CH3, 0x30, 0x00);

    LDC1614_1_WriteReg(MUX_CONFIG, 0xC2, 0x0C);

    //LDC1614_1_WriteReg(CONFIG, 0x16, 0x01); 
    LDC1614_1_WriteReg(CONFIG, 0x14, 0x01); 
    //LDC1614_1_WriteReg(CONFIG, 0x54, 0x01); 
    //LDC1614_1_WriteReg(CONFIG, 0x94, 0x01); 
    //LDC1614_1_WriteReg(CONFIG, 0xD4, 0x01); 

    return device_id;



    }
    else return 0;

}

/*
 *   LDC1614_1_WriteReg
 *   该函数用于配置LDC1614的寄存器
 *
 *   参数：
 *   RegisterAddress 
 *    |__ LDC1614寄存器地址
 *   Data
 *    |__ 具体写入的uint16_t数据 
 */
void LDC1614_1_WriteReg(uint8_t RegisterAddress, uint8_t Data1 , uint8_t Data0)
{
    
    IIC_Start();

    //发送从机地址
    IIC_Send_Byte(LDC1614_1_DEV_WRITE);
    IIC_Wait_Ack();
    

    //写LDC1614寄存器地址
    IIC_Send_Byte(RegisterAddress);
    IIC_Wait_Ack();

    //向寄存器中写具体数据
    IIC_Send_Byte(Data1);
    IIC_Wait_Ack();

    //向寄存器中写具体数据
    IIC_Send_Byte(Data0);
    IIC_Wait_Ack();

    IIC_Stop();
                                 

    LDC1614_Delay();
}

/*
 *   LDC1614_1_ReadReg
 *   该函数用于读取LDC1614的数据
 *
 *   参数：
 *     RegisterAddress 
 *        |__ LDC1614寄存器地址
 *   返回值
          |__ 寄存器的值
 *      
 */

uint16_t LDC1614_1_ReadReg(uint8_t RegisterAddress)
{
    uint16_t result;

  	IIC_Start();
    
    //发送从机地址
    IIC_Send_Byte(LDC1614_1_DEV_WRITE);
    IIC_Wait_Ack();
    

    //写LDC1614寄存器地址
    IIC_Send_Byte(RegisterAddress);
    IIC_Wait_Ack();
    

	//需要读取数据，这个时候需要变换数据传输方向，就要主机重新发送起始位
    //再次产生开始信号
	IIC_Start();

    //发送从机地址和读取位
    IIC_Send_Byte(LDC1614_1_DEV_READ);
    IIC_Wait_Ack();
    result =IIC_Read_Byte(1)<<8;	
   
    result|=IIC_Read_Byte(0);		  

    IIC_Stop();
  
    LDC1614_Delay(); 

    return result;
}

/*
 * 函数功能：读LDC1614单通道输出
 * 参数
 *       Status - 数据寄存器状态
 *       Regs_Addr - 数据寄存器地址
 * 函数返回值：电感值（uint32_t）
 */   
uint32_t LDC1614_1_GetResult(uint8_t Regs_Addr) 
{
  uint32_t result,temp;
     
  result= LDC1614_1_ReadReg( Regs_Addr);
  temp  = LDC1614_1_ReadReg( Regs_Addr-1);
  temp=temp&0x0FFF;
  temp=temp<<16;
  result=result|temp;
  
  return result;
}
/*
 * 函数功能：读LDC1614四通道输出
 * 参数
 *       *result - 结果指针
 *       
 * 函数返回值：无
 */  
void LDC1614_1_GetAllResult(uint32_t *result)
{
  uint16_t ret=0;
  
  ret=LDC1614_1_ReadReg(STATUS);
  
  if((ret&STATUS_CH0_UNREADCONV)!=0){
  result[0] = LDC1614_1_ReadReg( DATA_LSB_CH0);
  }
  if((ret&STATUS_CH1_UNREADCONV)!=0){
  result[1] = LDC1614_1_ReadReg( DATA_LSB_CH1);
  }
  if((ret&STATUS_CH2_UNREADCONV)!=0){
  result[2] = LDC1614_1_ReadReg( DATA_LSB_CH2);
  }
  if((ret&STATUS_CH3_UNREADCONV)!=0){
  result[3] = LDC1614_1_ReadReg( DATA_LSB_CH3);
  }


} 


/*
 *   LDC1614_2_Init
 *   该函数用于初始化LDC1614
 *
 *   参数：
 *   RegisterAddress 
 *    |__ void
 *   Data
 *    |__ device_id 
 */
uint16_t LDC1614_2_Init(void)
{
    uint16_t device_id;
    int cnt;
    GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOG, ENABLE );	//使能GPIOG时钟
    
    //设置地址0x2B
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_SetBits(GPIOG,GPIO_Pin_8); 	//PG8 输出高
    
    IIC_Init();
    
  
    LDC1614_2_WriteReg(RESET_DEV, 0x80, 0x00);
  
    for(cnt =0;cnt<300;cnt++){
        LDC1614_Delay();
    }
  
      //读取设备ID
      device_id = LDC1614_2_ReadReg(DEVICE_ID);

    //进行寄存器配置
    if(device_id==0x3055){

    LDC1614_2_WriteReg(RCOUNT_CH0, 0x02, 0x20);    //040d
    LDC1614_2_WriteReg(RCOUNT_CH1, 0x02, 0x20);
    LDC1614_2_WriteReg(RCOUNT_CH2, 0x02, 0x20);
    LDC1614_2_WriteReg(RCOUNT_CH3, 0x02, 0x20);
                                    
    LDC1614_2_WriteReg(SETTLECOUNT_CH0, 0x00, 0x1E);
    LDC1614_2_WriteReg(SETTLECOUNT_CH1, 0x00, 0x1E);
    LDC1614_2_WriteReg(SETTLECOUNT_CH2, 0x00, 0x1E);
    LDC1614_2_WriteReg(SETTLECOUNT_CH3, 0x00, 0x1E);

    LDC1614_2_WriteReg(CLOCK_DIVIDERS_CH0, 0x10, 0x03);
    LDC1614_2_WriteReg(CLOCK_DIVIDERS_CH1, 0x10, 0x03); 
    LDC1614_2_WriteReg(CLOCK_DIVIDERS_CH2, 0x10, 0x03);  
    LDC1614_2_WriteReg(CLOCK_DIVIDERS_CH3, 0x10, 0x03); 
     
    //LDC1614_2_WriteReg(ERROR_CONFIG, 0x00, 0x01);

    LDC1614_2_WriteReg(DRIVE_CURRENT_CH0, 0x30, 0x00); ///0X70  
    LDC1614_2_WriteReg(DRIVE_CURRENT_CH1, 0x30, 0x00);
    LDC1614_2_WriteReg(DRIVE_CURRENT_CH2, 0x30, 0x00);
    LDC1614_2_WriteReg(DRIVE_CURRENT_CH3, 0x30, 0x00);

    //LDC1614_2_WriteReg(MUX_CONFIG, 0xA2, 0x0F);
    LDC1614_2_WriteReg(MUX_CONFIG, 0xC2, 0x0C);

    //LDC1614_2_WriteReg(CONFIG, 0x16, 0x01); 
    LDC1614_2_WriteReg(CONFIG, 0x14, 0x01); 
    //LDC1614_2_WriteReg(CONFIG, 0x54, 0x01); 
    //LDC1614_2_WriteReg(CONFIG, 0x94, 0x01); 
    //LDC1614_2_WriteReg(CONFIG, 0xD4, 0x01); 

    return device_id;


    }
    else return 0;

}

/*
 *   LDC1614_2_WriteReg
 *   该函数用于配置LDC1614的寄存器
 *
 *   参数：
 *   RegisterAddress 
 *    |__ LDC1614寄存器地址
 *   Data
 *    |__ 具体写入的uint16_t数据 
 */
void LDC1614_2_WriteReg(uint8_t RegisterAddress, uint8_t Data1 , uint8_t Data0)
{
    IIC_Start();

    //发送从机地址
    IIC_Send_Byte(LDC1614_2_DEV_WRITE);
    IIC_Wait_Ack();
    

    //写LDC1614寄存器地址
    IIC_Send_Byte(RegisterAddress);
    IIC_Wait_Ack();

    //向寄存器中写具体数据
    IIC_Send_Byte(Data1);
    IIC_Wait_Ack();

    //向寄存器中写具体数据
    IIC_Send_Byte(Data0);
    IIC_Wait_Ack();

    IIC_Stop();
                                 

    LDC1614_Delay();
}

/*
 *   LDC1614_2_ReadReg
 *   该函数用于读取LDC1614的数据
 *
 *   参数：
 *     RegisterAddress 
 *        |__ LDC1614寄存器地址
 *   返回值
          |__ 寄存器的值
 *      
 */
uint16_t LDC1614_2_ReadReg(uint8_t RegisterAddress)
{
    uint16_t result;

  	IIC_Start();
    
    //发送从机地址
    IIC_Send_Byte(LDC1614_2_DEV_WRITE);
    IIC_Wait_Ack();
    

    //写LDC1614寄存器地址
    IIC_Send_Byte(RegisterAddress);
    IIC_Wait_Ack();
    

	//需要读取数据，这个时候需要变换数据传输方向，就要主机重新发送起始位
    //再次产生开始信号
	IIC_Start();

    //发送从机地址和读取位
    IIC_Send_Byte(LDC1614_2_DEV_READ);
    IIC_Wait_Ack();
    result =IIC_Read_Byte(1)<<8;	
   
    result|=IIC_Read_Byte(0);		  

    IIC_Stop();
  
    LDC1614_Delay(); 

    return result;
}

/*
 * 函数功能：读LDC1614单通道输出
 * 参数
 *       Status - 数据寄存器状态
 *       Regs_Addr - 数据寄存器地址
 * 函数返回值：电感值（uint32_t）
 */   
uint32_t LDC1614_2_GetResult(uint8_t Regs_Addr) 
{
  uint32_t result,temp;
     
  result= LDC1614_2_ReadReg( Regs_Addr);
  temp  = LDC1614_2_ReadReg( Regs_Addr-1);
  temp=temp&0x0FFF;
  temp=temp<<16;
  result=result|temp;
  
  return result;
}
/*
 * 函数功能：读LDC1614四通道输出
 * 参数
 *       *result - 结果指针
 *       
 * 函数返回值：无
 */  
void LDC1614_2_GetAllResult(uint32_t *result)
{
  uint16_t ret=0;
  
  ret=LDC1614_2_ReadReg(STATUS);
  
  if((ret&STATUS_CH0_UNREADCONV)!=0){
  result[0] = LDC1614_2_ReadReg( DATA_LSB_CH0);
  }
  if((ret&STATUS_CH1_UNREADCONV)!=0){
  result[1] = LDC1614_2_ReadReg( DATA_LSB_CH1);
  }
  if((ret&STATUS_CH2_UNREADCONV)!=0){
  result[2] = LDC1614_2_ReadReg( DATA_LSB_CH2);
  }
  if((ret&STATUS_CH3_UNREADCONV)!=0){
  result[3] = LDC1614_2_ReadReg( DATA_LSB_CH3);
  }


} 
/*
 * 延时函数
 */
static void LDC1614_Delay(void){
  int n;
  for(n=1;n<2000;n++) {
    n=n;//asm("nop");
  }
}



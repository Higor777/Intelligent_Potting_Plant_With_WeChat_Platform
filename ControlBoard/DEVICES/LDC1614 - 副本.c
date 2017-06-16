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
	I2C_InitTypeDef    I2C_InitStructure;
    GPIO_InitTypeDef               GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(LDC1614_1_I2CX_GPIO_RCC,ENABLE);            //没有外部中断,没有重映射,所以不需要开启复用功能

	GPIO_InitStructure.GPIO_Pin    =LDC1614_1_SCLPIN|LDC1614_1_SDAPIN;
	GPIO_InitStructure.GPIO_Speed  =GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode   =GPIO_Mode_AF_OD;            //设置管脚为复用功能开漏输出 SDA是输入输出引脚

	GPIO_Init(LDC1614_1_I2CX_GPIO, &GPIO_InitStructure);
    //初始化LDC1614_1_I2CX

	RCC_APB1PeriphClockCmd(LDC1614_1_I2CX_RCC,ENABLE);
	I2C_DeInit(LDC1614_1_I2CX);

	I2C_InitStructure.I2C_ClockSpeed          =200000;   		 	//200KHz I2C时钟频率
	I2C_InitStructure.I2C_Mode                =I2C_Mode_I2C;   	//I2C模式
	I2C_InitStructure.I2C_DutyCycle           =I2C_DutyCycle_2; 	//时钟占空比
	I2C_InitStructure.I2C_OwnAddress1         =0x30;     			//主机地址 可以任意的
	I2C_InitStructure.I2C_Ack                 =I2C_Ack_Enable; 	//开启ACK应答响应
	I2C_InitStructure.I2C_AcknowledgedAddress =I2C_AcknowledgedAddress_7bit;//7位地址模式 非10为地址模式

	I2C_Init(LDC1614_1_I2CX,&I2C_InitStructure);
	I2C_Cmd(LDC1614_1_I2CX,ENABLE);
	I2C_AcknowledgeConfig(LDC1614_1_I2CX,ENABLE);
    
    

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

    LDC1614_1_WriteReg(CONFIG, 0x16, 0x01); 
    //LDC1614_1_WriteReg(CONFIG, 0x14, 0x01); 
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
    
    I2C_GenerateSTART(LDC1614_1_I2CX, ENABLE);
	while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_MODE_SELECT)); 

    //发送从机地址
	I2C_Send7bitAddress(LDC1614_1_I2CX,LDC1614_1_DEV_ADDR, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 

    //写LDC1614寄存器地址
	I2C_SendData(LDC1614_1_I2CX, RegisterAddress);
	while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    //向寄存器中写具体数据
	I2C_SendData(LDC1614_1_I2CX, Data1);
	while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    //向寄存器中写具体数据
	I2C_SendData(LDC1614_1_I2CX, Data0);
	while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
	I2C_GenerateSTOP(LDC1614_1_I2CX, ENABLE);                                        

    
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

  	while(I2C_GetFlagStatus(LDC1614_1_I2CX,I2C_FLAG_BUSY));


	I2C_AcknowledgeConfig(LDC1614_1_I2CX, ENABLE);


	I2C_GenerateSTART(LDC1614_1_I2CX,ENABLE);
	while(!I2C_CheckEvent(LDC1614_1_I2CX,I2C_EVENT_MASTER_MODE_SELECT));                 //检查是不是主模式与起始位已经发送 备注:这样做的目的就是为了清空该事件

    //发送从机地址
	I2C_Send7bitAddress(LDC1614_1_I2CX,LDC1614_1_DEV_ADDR, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));//检查数据是不是已经发送地址完成

    //写LDC1614寄存器地址
	I2C_SendData(LDC1614_1_I2CX, RegisterAddress);
	while (!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_BYTE_TRANSMITTED));         //检查数据是不是已经发送完成

	//需要读取数据，这个时候需要变换数据传输方向，就要主机重新发送起始位
    //再次产生开始信号
	I2C_GenerateSTART(LDC1614_1_I2CX, ENABLE);
	while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_MODE_SELECT));

    //发送从机地址和读取位
	I2C_Send7bitAddress(LDC1614_1_I2CX,LDC1614_1_DEV_READ, I2C_Direction_Receiver);         //再一次发送EEPROM地址
	while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));


    while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_BYTE_RECEIVED));            //检查是不是接受到了数据
    result = I2C_ReceiveData(LDC1614_1_I2CX)<<8;

    I2C_AcknowledgeConfig(LDC1614_1_I2CX, DISABLE);         //关闭应答
    I2C_GenerateSTOP(LDC1614_1_I2CX,ENABLE);           //使能停止功能

    while(!I2C_CheckEvent(LDC1614_1_I2CX, I2C_EVENT_MASTER_BYTE_RECEIVED));            //检查是不是接受到了数据
    
    result|=I2C_ReceiveData(LDC1614_1_I2CX);
        
	I2C_AcknowledgeConfig(LDC1614_1_I2CX, ENABLE);          //允许应答模式
  
  
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
  I2C_InitTypeDef i2c_init_param;
  uint16_t device_id;
    int cnt;
    
  //初始化LDC1614_2_I2CX
//  i2c_init_param.I2C_I2Cx = LDC1614_2_I2CX;       //在LDC1614.h中修改该值
//  i2c_init_param.I2C_IntEnable = FALSE;
//  i2c_init_param.I2C_ICR = LDC1614_SCL_200KHZ;  //可根据实际电路更改SCL频率
//  i2c_init_param.I2C_SclPin = LDC1614_2_SCLPIN;   //在LDC1614.h中修改该值
//  i2c_init_param.I2C_SdaPin = LDC1614_2_SDAPIN;   //在LDC1614.h中修改该值
//  i2c_init_param.I2C_Isr = NULL;
//  
//  LPLD_I2C_Init(i2c_init_param);
  
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

  LDC1614_2_WriteReg(CONFIG, 0x16, 0x01); 
  //LDC1614_2_WriteReg(CONFIG, 0x14, 0x01); 
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
  //发送从机地址
//  LPLD_I2C_StartTrans(LDC1614_2_I2CX, LDC1614_2_DEV_ADDR, I2C_MWSR);
//  LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);
//  
//  //写LDC1614寄存器地址
//  LPLD_I2C_WriteByte(LDC1614_2_I2CX, RegisterAddress);
//  LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);
//  
//  //向寄存器中写具体数据
//  LPLD_I2C_WriteByte(LDC1614_2_I2CX, Data1);
//  LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);
//  
//  //向寄存器中写具体数据
//  LPLD_I2C_WriteByte(LDC1614_2_I2CX, Data0);
//  LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);

//  LPLD_I2C_Stop(LDC1614_2_I2CX);

//  LDC1614_Delay();
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

  //发送从机地址
//  LPLD_I2C_StartTrans(LDC1614_2_I2CX, LDC1614_2_DEV_ADDR, I2C_MWSR);
//  LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);

//  //写LDC1614寄存器地址
//  LPLD_I2C_WriteByte(LDC1614_2_I2CX, RegisterAddress);
//  LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);

//  //再次产生开始信号
//  LPLD_I2C_ReStart(LDC1614_2_I2CX);

//  //发送从机地址和读取位
//  LPLD_I2C_WriteByte(LDC1614_2_I2CX, LDC1614_2_DEV_READ);
//  LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);


//  //转换主机模式为读
// LPLD_I2C_SetMasterWR(LDC1614_2_I2CX, I2C_MRSW);
// result =LPLD_I2C_ReadByte(LDC1614_2_I2CX);
// LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);
//  //关闭应答ACK
// //LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_OFF);//关闭ACK
//// i2c1->C1 &= 0xF7;
//// i2c1->
// result =LPLD_I2C_ReadByte(LDC1614_2_I2CX)<<8;


//  LPLD_I2C_WaitAck(LDC1614_2_I2CX, I2C_ACK_ON);   

//  LPLD_I2C_Stop(LDC1614_2_I2CX);
//  result =result|LPLD_I2C_ReadByte(LDC1614_2_I2CX);
//  
//  LDC1614_Delay(); 

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
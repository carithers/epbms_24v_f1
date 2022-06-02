/* ==================================================================================

 File name:     I2C_BQ769x0.h
 Originator:    BLJ
 Description:   BQ769x0 I2C通信模块，兼容CRC校验，基于STM32F0xx平台

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-31-2016		Version 1.0.0			修正部分bug，测试通过
 12-20-2015		Version 0.9.0			正式版本，CRC校验测试通过，无CRC芯片兼容未测试
-----------------------------------------------------------------------------------*/

#include "target.h"                     // 目标板硬件选择头文件
#include "I2C_BQ769xx.h"                // I2C总线通讯模块头文件
#include "delay.h"                      // 软件延时模块
#include "CRC8.h"						// CRC校验值模块


void IIC_Stop(void);

// ------------------------- I2C EEPROM模块IO口初始化 ---------------------------------
void I2C_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;                // GPIO配置寄存器结构体
    //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);  //开启GPIOB时钟
    
	GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN|I2C_SDA_PIN;    		// PB13,I2C2_SCL,推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
}






// ------------------------------- EEPROM模块硬件初始化 ---------------------------------
void I2C_BQ769xx_DeviceInit(void)
{
	I2C_GPIO_Config();
	
    IIC_Stop();
}




void IIC_Start(void)
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	AFE_I2C_SDA_HIGH();
	AFE_I2C_SCL_HIGH();
	AFE_I2C_Delay();
	AFE_I2C_SDA_LOW();
	AFE_I2C_Delay();
	AFE_I2C_SCL_LOW();
	AFE_I2C_Delay();
}	
void IIC_Stop(void)
{
	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */
	AFE_I2C_SDA_LOW();
	AFE_I2C_SCL_HIGH();
	AFE_I2C_Delay();
	AFE_I2C_SDA_HIGH();
						   	
}
u8 IIC_Wait_Ack(void)
{
	uint8_t re;

	AFE_I2C_SDA_HIGH();	/* CPU释放SDA总线 */
	AFE_I2C_Delay();
	AFE_I2C_SCL_HIGH();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	AFE_I2C_Delay();
	if (AFE_I2C_SDA_READ())	/* CPU读取SDA口线状态 */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	AFE_I2C_SCL_LOW();
	AFE_I2C_Delay();
	return re; 
} 

void IIC_Ack(void)
{
	AFE_I2C_SDA_LOW();	/* CPU驱动SDA = 0 */
	AFE_I2C_Delay();
	AFE_I2C_SCL_HIGH();	/* CPU产生1个时钟 */
	AFE_I2C_Delay();
	AFE_I2C_SCL_LOW();
	AFE_I2C_Delay();
	AFE_I2C_SDA_HIGH();	/* CPU释放SDA总线 */
	
}

void IIC_NAck(void)
{
	AFE_I2C_SDA_HIGH();	/* CPU驱动SDA = 1 */
	AFE_I2C_Delay();
  AFE_I2C_SCL_HIGH();	/* CPU产生1个时钟 */
	AFE_I2C_Delay();
	AFE_I2C_SCL_LOW();
	AFE_I2C_Delay();

}

void IIC_Send_Byte(u8 txd)
{                        
	u8 i;

	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++)
	{		
		if (txd & 0x80)
		{
			AFE_I2C_SDA_HIGH();
		}
		else
		{
			AFE_I2C_SDA_LOW();
		}
		AFE_I2C_Delay();
		AFE_I2C_SCL_HIGH();
		AFE_I2C_Delay();	
		AFE_I2C_SCL_LOW();
		if (i == 7)
		{
			AFE_I2C_SDA_HIGH(); // 释放总线
		}
		txd <<= 1;	/* 左移一个bit */
		AFE_I2C_Delay();
	}	 
} 

u8 IIC_Read_Byte(u8 ack)
{
	uint8_t i;
	uint8_t value;

	/* 读到第1个bit为数据的bit7 */
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		AFE_I2C_SCL_HIGH();
		AFE_I2C_Delay();
		if (AFE_I2C_SDA_READ())
		{
			value++;
		}
		AFE_I2C_SCL_LOW();
		AFE_I2C_Delay();
	}
	
//    if (!ack)
//        IIC_NAck();//发送nACK
//    else
//        IIC_Ack(); //发送ACK   
    return value;
}


//u8 i2c_read_cmd_wait_flg = 0;
//u8 text_CRC = 0;
//u16 i2c_read_cmd_wait_cnt = 0;

// ------------------------- I2C EEPROM模块读取数据过程函数 --------------------------------
//u16 I2C_BQ769xx_Read_Process(I2C_BQ769x0_structDef* v)
//{
// u8 Crc_buf[6] = {0};   
//	v->Variable.DataByteCnt = 0;
//	IIC_Start();//起始信号
//	IIC_Send_Byte(v->Resver_Parameter.ADDR);//从机地址加写标志
//	Crc_buf[0] = v->Resver_Parameter.ADDR;
//	if(IIC_Wait_Ack() == 1){/*v->State.Fault_bits.bit.ByteSendAckHi = 1;*/IIC_Stop(); return 1;}//应答
//	IIC_Send_Byte(v->Resver_Parameter.Register_Address);//寄存器地址
//	Crc_buf[1] = v->Resver_Parameter.Register_Address;
//	if(IIC_Wait_Ack() == 1){/*v->State.Fault_bits.bit.ByteSendAckHi = 1;*/IIC_Stop();  return 1;}//应答
//	IIC_Start();//起始信号 和RS复位信号一致
//	
//	IIC_Send_Byte(v->Resver_Parameter.ADDR+1);//从机地址加读标志
//	Crc_buf[2] = v->Resver_Parameter.ADDR + 1;
//	if(IIC_Wait_Ack() == 1){/*v->State.Fault_bits.bit.ByteSendAckHi = 1;*/ IIC_Stop(); return 1;}//应答

//	
//	while(v->Variable.DataByteCnt < v->Resver_Parameter.DataByteLength)//303
//	{
	
//		switch(v->Variable.DataByteCnt)
//		{
//			case 0://data
//				*v->Resver_Parameter.pBuffer = IIC_Read_Byte(1);//包含应答
//			     
//				Crc_buf[3] = *v->Resver_Parameter.pBuffer;        //
//				break;
//			case 1://data
//				*(v->Resver_Parameter.pBuffer+1) = IIC_Read_Byte(1);
//				Crc_buf[4] = *(v->Resver_Parameter.pBuffer+1);
//			   
//				break;
//			case 2://crc
//				v->Resver_Parameter.CRCValue = IIC_Read_Byte(0);
//			    Crc_buf[5] = v->Resver_Parameter.CRCValue;
//				break;
//			default :
//				break;
//		}
//		
//			v->Variable.DataByteCnt++;
//		
//	}
//	IIC_Stop();
//	v->Variable.DataByteCnt=0;
//	
////	if(v->State.State_bits.bit.CRCCheckEnable==0) 
////		IIC_NAck();
//	
//    if(v->Resver_Parameter.CRCValue == GetCRC8(Crc_buf, 5)||(v->State.State_bits.bit.CRCCheckEnable==0))//核对CRC结果
//	{
//		//可以在传出读取的数据
//		//v->Resver_Parameter.pBuffer;
//		
//		// 读取指令已完成，清除指令
//		v->State.State_bits.bit.Command = 0;
//		v->State.State_bits.bit.CRCCheckFail=0;
//		v->State.State_bits.bit.CommandFinish = 1;          // 指令完成标志位
//	}
//	else
//	{ 
//		*(v->Resver_Parameter.pBuffer) = 0;
//		*(v->Resver_Parameter.pBuffer+1) = 0;//如果校验错误则清除错误值
//		v->State.State_bits.bit.CRCCheckFail = 1;
//	}
//    return 0;
//}






u16 I2C_BQ769xx_Read_Process(I2C_BQ769x0_structDef* v)
{
   u8 Crc_buf[6] = {0};   
	v->Variable.DataByteCnt = 0;
   
   
	IIC_Start();//起始信号
	IIC_Send_Byte(0x36);//从机地址加写标志
	if(IIC_Wait_Ack() == 1){IIC_Stop(); return 1;}//应答
	IIC_Send_Byte(v->Resver_Parameter.Register_Address);//寄存器地址
	if(IIC_Wait_Ack() == 1){IIC_Stop();  return 1;}//应答
	IIC_Start();//起始信号 和RS复位信号一致
	IIC_Send_Byte(0x37);//从机地址加读标志
	if(IIC_Wait_Ack() == 1){ IIC_Stop(); return 1;}//等待应答
	
	*v->Resver_Parameter.pBuffer = IIC_Read_Byte(1);//包含应答 
	 IIC_Ack(); //发送ACK  
	*(v->Resver_Parameter.pBuffer+1) = IIC_Read_Byte(1);
	 IIC_Ack(); //发送ACK  
	 v->Resver_Parameter.CRCValue = IIC_Read_Byte(0);
	 IIC_NAck(); //发送NACK  
	IIC_Stop();
	
	Crc_buf[0]=0x36;
	Crc_buf[1]=v->Resver_Parameter.Register_Address;
	Crc_buf[2] =0x37;
	Crc_buf[3] =  *v->Resver_Parameter.pBuffer;
	Crc_buf[4] =  *(v->Resver_Parameter.pBuffer+1);
	
   if(v->Resver_Parameter.CRCValue == GetCRC8(Crc_buf, 5))//核对CRC结果
	{
		// 读取指令已完成，清除指令
		v->State.State_bits.bit.Command = 0;
		v->State.State_bits.bit.CRCCheckFail=0;
		v->State.State_bits.bit.CommandFinish = 1;          // 指令完成标志位
	}
	else
	{ 
		*(v->Resver_Parameter.pBuffer) = 0;
		*(v->Resver_Parameter.pBuffer+1) = 0;//如果校验错误则清除错误值
		v->State.State_bits.bit.CRCCheckFail = 1;
	}
    return 0;
}



// --------- I2C EEPROM模块连续写入函数，可以写入1byte至1页数据，最多一次性写入一页数据，正常返回0，错误返回1 -----------
u16 I2C_BQ769xx_Write_Process(I2C_BQ769x0_structDef* v)
{   	
u8 Write_CRC[3]={0};	
  
	IIC_Start();
	IIC_Send_Byte(v->Send_Parameter.ADDR);
	if(IIC_Wait_Ack() == 1){/*v->State.Fault_bits.bit.ByteSendAckHi = 1;*/ return 1;}
	Write_CRC[0]=v->Send_Parameter.ADDR;
	IIC_Send_Byte(v->Send_Parameter.Register_Address);
	if(IIC_Wait_Ack() == 1){/*v->State.Fault_bits.bit.ByteSendAckHi = 1;*/ return 1;}
	Write_CRC[1]=v->Send_Parameter.Register_Address;
    //需要发送数据时
    while(v->Variable.DataByteCnt < v->Send_Parameter.DataByteLength)  //DataByteCnt写入前已清零
    {
		switch (v->Variable.DataByteCnt)
		{
			case 0://data
				IIC_Send_Byte(*v->Send_Parameter.pBuffer);//发送一个字节数据
				if(IIC_Wait_Ack() == 1){/*v->State.Fault_bits.bit.ByteSendAckHi = 1;*/return 1;}//等待接收应答信号
				Write_CRC[2]= *v->Send_Parameter.pBuffer;
				break;
			case 1://crc              
				v->Send_Parameter.CRCValue  = GetCRC8(Write_CRC,3);//CRC校验//写操作校验 从机地址+写标志位 寄存器地址 数据
				IIC_Send_Byte (v->Send_Parameter.CRCValue);   //发送CRC校验值
				if(IIC_Wait_Ack()){/*v->State.Fault_bits.bit.ByteSendAckHi = 1;*/ return 1;}//CRC校验失败
			    //IIC_Stop();// 等待设置完成 发送调整信号
			default://防止越界
				break;
		}

		v->Variable.DataByteCnt++;		//循环发送数据
    }
	v->Variable.DataByteCnt = 0;
   IIC_Stop();// 等待设置完成 发送调整信号
    
	v->State.State_bits.bit.Command = 0;      //发送成功读写指令清除
	v->State.State_bits.bit.CommandFinish = 1;//读写指令正常完成,内部置位,需外部清除  

    return 0;
}


// --------------------------I2C模块过程函数，运行在主循环中--根据命令读写数据-----------------------


void I2C_BQ769x0_Process(I2C_BQ769x0_structDef* v)  //g_I2C_SH36730x0.State.State_bits.bit.CommandFinish 
{
	
    switch (v->State.State_bits.bit.Command)            // 判断读或者写
    {
        case I2C_EEPROM_WRITE:
        {
            I2C_BQ769xx_Write_Process(v);
			v->State.State_bits.bit.Command=0;
        }
        break;
        
        case I2C_EEPROM_READ:
        {
			
            I2C_BQ769xx_Read_Process(v);
			v->State.State_bits.bit.Command=0;
        }
        break;
    }
	
	// Take care: 有故障时自动清除命令
	if (v->State.Fault_bits.all > 0)
	{
		v->State.State_bits.bit.Command = 0;
	}
}


// ---------------------- 读取配置---------------------------

u16 I2C_BQ769x0_Read(I2C_BQ769x0_structDef* v, u8 Address,u8* pbuffer, u8 Register_Address, u16 CRCCheckEnable)
{
    if (v->State.State_bits.bit.Command == 0 && v->State.Fault_bits.all == 0)//读取 与 通讯无错误
    {
        v->Variable.CRCValue = 0;       
        v->Variable.DataByteCnt = 0;
        v->State.State_bits.bit.CRCCheckFail = 0;
		
        
        v->Resver_Parameter.ADDR = Address;
		v->Resver_Parameter.pBuffer = pbuffer;//-------------------------指针指向寄存器结构体---------------------------
		v->Resver_Parameter.Register_Address = Register_Address;
        v->State.State_bits.bit.CRCCheckEnable = CRCCheckEnable;  
    		
       		
        if (v->State.State_bits.bit.CRCCheckEnable > 0)
        {

            v->Resver_Parameter.DataByteLength = 3;           
        }
		else
		{
			v->Resver_Parameter.DataByteLength = 2;
		}

        v->State.State_bits.bit.Command = I2C_EEPROM_READ;
        
        return 0;
    }
    else
    {

        return 1;
    }
}


// ---------------------- I2C SH36730模块写入数据指令,返回值0：正常；1：错误 ---------------------------

//                                           从机地址       寄存器地址            写入数据          CRC校验使能
u16 I2C_BQ769x0_Write(I2C_BQ769x0_structDef* v,u8 Address, u8 Register_Address, u8* Register_Data, u16 CRCCheckEnable )
{
    if (v->State.State_bits.bit.Command == 0 && v->State.Fault_bits.all == 0)//判断是否有命令在执行中或者错误
    {		

        v->Variable.CRCValue = 0;
        v->Variable.DataByteCnt = 0;
        v->State.State_bits.bit.CRCCheckFail = 0;
		
		v->Send_Parameter.ADDR = Address;                        // 从机地址
        v->Send_Parameter.Register_Address = Register_Address;   // AFE寄存器地址
		v->Send_Parameter.pBuffer = Register_Data;            // 需要写入的数据
        
        v->State.State_bits.bit.CRCCheckEnable = CRCCheckEnable; // CRC校验使能    
		
        if (v->State.State_bits.bit.CRCCheckEnable > 0) 
        {
            v->Send_Parameter.DataByteLength = 2;                //写入数据加CRC校验值         
        }
		else
		{
			v->Send_Parameter.DataByteLength = 1;                //写入数据
		}
        v->State.State_bits.bit.Command = I2C_EEPROM_WRITE;
        
        return 0;
    }
    else
    {
        return 1;
    }
}







// End of BQ769x0.c





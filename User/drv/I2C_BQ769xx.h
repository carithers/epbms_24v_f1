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


#include "stm32f0xx.h"                            // STM32器件寄存器定义头文件


// ---------------------------------- 模块宏定义 -------------------------------------
// For edit

// 产品地址由具体产品决定
// Take care: 不同型号后缀的BQ769系列新品，从机地址不相同
#define I2C_SLAVE_DEVICE_ADDRESS        0x08        // 从设备地址，7bit，BQ7693003默认地址0x08

#define I2C_SPEED                       100000      // I2C时钟频率100kHz

// 配置I2C总线通信频率，具体配置方式请看STM32F0xx系列使用手册
//#define I2C_TIMING          			0x10420F13		// 8MHzI2C模块主频下，总线频率100KHz
//#define I2C_TIMING          			0x10425F63		// 8MHzI2C模块主频下，总线频率20KHz
#define I2C_TIMING          			0x1042C3C7		// 8MHzI2C模块主频下，总线频率10KHz

// Edit end

// I2C write/read command
#define I2C_EEPROM_WRITE                1
#define I2C_EEPROM_READ                 2

#define I2C_BUFFER_LENGTH               512

#define AFE_I2C_SCL_HIGH()                GPIO_SetBits(GPIOB,GPIO_Pin_10)           
#define AFE_I2C_SCL_LOW()                 GPIO_ResetBits(GPIOB,GPIO_Pin_10)        
#define AFE_I2C_SDA_HIGH()                GPIO_SetBits(GPIOB,GPIO_Pin_11)           
#define AFE_I2C_SDA_LOW()                 GPIO_ResetBits(GPIOB,GPIO_Pin_11)          
#define AFE_I2C_SDA_READ()                GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)


// ---------------------------- I2C总线模块结构体 ------------------------------

struct I2C_EEPROM_State_bits {
    u16 Command:2;                          // I2C 读写指令
    u16 CommandFinish:1;                    // 读写指令正常完成，内部置位，需外部清除
    u16 CRCCheckEnable:1;                   // 使能CRC校验
    u16 CRCCheckFail:1;                     // CRC校验错误
    u16 rsvd:11;
};

union I2C_EEPROM_State_union {
    u16                             all;
    struct I2C_EEPROM_State_bits    bit;
};

struct I2C_EEPROM_Fault_bits {
    u16 OverTimeFlag1:1;                        // 超时
    u16 OverTimeFlag2:1;                        // 超时
    u16 OverTimeFlag3:1;                        // 超时
    u16 OverTimeFlag4:1;                        // 超时
    u16 OverTimeFlag5:1;                        // 超时
    u16 OverTimeFlag6:1;                        // 超时
    u16 OverTimeFlag7:1;                        // 超时
    u16 OverTimeFlag8:1;                        // 超时
    u16 OverTimeFlag9:1;                        // 超时
    u16 OverTimeFlag10:1;                       // 超时
    u16 START_SDA_LOW_BUSY:1;                   // 发送START前，SDA低电平，总线忙
    u16 ByteSendAckHi:1;                        // 字节发送完成，从机返回ACK高电平，未成功接收
    u16 BufferOverFlow:1;                       // 缓存溢出
    u16 SeriesModeError:1;                      // 串联模式选择错误。SeriesMode可设置为0或1，0: I2C总线对应低位AFE，1：I2C总线对应高位AFE，两个AFE数据独立，IO口不同！！！
    u16 rsvd:2;
};

union I2C_EEPROM_Fault_union {
    u16                             all;
    struct I2C_EEPROM_Fault_bits    bit;
};

struct I2C_EEPROM_State {
    union I2C_EEPROM_State_union    State_bits;
    union I2C_EEPROM_Fault_union    Fault_bits;
};

struct I2C_EEPROM_Parameter {
    u8      ADDR;                            // 从机地址
	u8		Register_Address;				 // 寄存器地址，单位为字节Byte
    u8*     pBuffer;                         // 外部数据起始地址
	u8		CRCValue;                        // CRC校验值
	u16     DataByteLength;                  // 读取写入数据长度
};

struct I2C_EEPROM_Variable {    
    u16     ClearStep;                          // 清除busy状态
    u16     WriteDataStep;                      // 写入数据步骤
    u16     ReadDataStep;                       // 读取数据步骤
    u16     StartStep;                          // start步骤
    u16     ByteStep;                           // 字节操作步骤
    u16     DataByteCnt;                        // 读取写入数据计数
    u16     WaitCnt;                            // 等待计数器
    u8      ACKState;                           // ACK状态,0:低电平，1：高电平
    u8      ACKCmd;                             // ACK发送命令，发送高，则停止读取
    u8      ByteToSend;                         // 待发送字节
    u8      ReceiveByte;                        // 接收字节
    u8      CRCValue;                           // CRC校验值
    u8      wait_crc8_flg;
    u8      wait_crc8_come_back_flg;
    u8      wait_crc8_v;
};

// I2C总线模块结构体
typedef struct {    
    struct  I2C_EEPROM_State        State;
    struct  I2C_EEPROM_Parameter    Send_Parameter;
	struct  I2C_EEPROM_Parameter    Resver_Parameter;
    struct  I2C_EEPROM_Variable     Variable; 
	//u8		ByteBuffer[I2C_BUFFER_LENGTH];						// 数据缓存字节，用于读写缓存及CRC校验
} I2C_BQ769x0_structDef;


// ---------------------------- 外部可调用函数 ---------------------------------------
void I2C_BQ769xx_DeviceInit(void);                      	// I2C模块硬件初始化
void I2C_BQ769x0_Process(I2C_BQ769x0_structDef* v);       	// I2C模块过程函数，运行在主循环中

uint8_t AFE_EEPROM_ReadBytes(uint8_t *ReadBuf, uint16_t Address, uint16_t Size);
u16 I2C_BQ769x0_Read(I2C_BQ769x0_structDef* v, u8 Address,u8* pbuffer, u8 Register_Address, u16 CRCCheckEnable);      // 读取指令,返回值0：正常；1：错误

u16 I2C_BQ769x0_Write(I2C_BQ769x0_structDef* v,u8 Address, u8 Register_Address, u8* Register_Data, u16 CRCCheckEnable );


// End of I2C_BQ769x0.h


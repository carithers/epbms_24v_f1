/* ==================================================================================

 File name:     USART.c
 Originator:    BLJ
 Description:   串口通信模块，兼容RS485，强制工作在半双工

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-20-2016		Version 1.0.0			增加写入数据完成标志位更新，正式版本
 01-17-2015     Version 0.0.1           测试功能
 03-26-2015     Version 0.1.0           MODBUS，多寄存器读写功能实现，待长期测试及改进，
 										可使用串口模块1，3，暂时未添加2
-----------------------------------------------------------------------------------*/


#include "stm32f0xx.h"

// 模块相关宏定义

// for edit： 修改此宏定义改变从机节点ID号，后期考虑ID存储在EEPROM中，此宏定义仅作默认ID号使用
#define	MODBUS_ID_UMBER					1		// MODBUS节点ID号
#define MODBUS_COMMUNICAT               0x45    //简易版BMS通讯协议的帧头

#define	MODBUS_BLOCK_DEBUGREF			0		// MODBUS通信，区块编号：调试给定值
#define	MODBUS_BLOCK_PARAMETER			1		// MODBUS通信，区块编号：参数区块


// For Edit: 模块最大允许外部数据区块，需与模块设置相对应
// 模块初始化前，需定义数据读写区块地址及长度
// 默认总区块为16个，所有区块允许读取，前4个允许写入。所有操作最小单位为双字节
#define	MODBUS_DATA_BLOCK_MAX			16		// 最多16个数据区块
#define	MODBUS_DATA_WRITEBLOCK_MAX		4		// 最多4个数据区块有写入权限，即区块0,1,2,3

#define	BYTE_BUFFER_LENGTH				300		// 数据缓存长度，需保证能接收发送125×16bits数据内容

// MODBUS相关宏定义
#define	MODBUS_DATA_LENGTH_LIMIT		125		// MODBUS数据帧最多允许携带数据，单位16bits

#define MODBUS_BYTE_TIMEOUT				10		// 接收时，字节间超时默认为10ms，设为1-10ms较为合适
#define	MODBUS_FRAME_TIMEOUT			100		// 发送时，数据帧超时为100ms，即一个数据帧需在1s内全部接收完成
#define MODBUS_FAULT_RESPONSE_DELAY		5		// 故障帧发送延时，无新数据被接收超过该时间后，开始发送故障回复帧，此值必须小于 MODBUS_BYTE_TIMEOUT

// MODBUS从机处理状态
#define	MODBUS_IDLE						0		// MODBUS状态：MODBUS空闲状态
#define	MODBUS_CHECK_ID					8		// MODBUS状态：检查ID是否匹配	
#define	MODBUS_CHECK_FUNCTION_LENGTH	1		// MODBUS状态：检查功能码及数据总长度
#define MODBUS_WAITRECEIVE				2		// MODBUS状态：等待数据接收完成
#define	MODBUS_PROCESSREAD				3		// MODBUS状态：多寄存器读取处理
#define	MODBUS_PROCESSWRITE				4		// MODBUS状态：多寄存器写入处理
#define MODBUS_RESPONSE					5		// MODBUS状态：发送反馈数据
#define	MODBUS_ERROR					6		// MODBUS状态：数据错误
#define	MODBUS_NOTMINE					7		// MODBUS状态：其它目标ID

#define COMM_RECIVE_FUNCTION            9       // 串口与仪表通讯功能   

// ----------------------------- MODBUS帧结构 --------------------------------
#define	MODBUS_ID						0			// BYTE0: ID，8位
#define	MODBUS_FUNCTION					1			// BYTE1: 功能码，8位
#define	MODBUS_ADDRESSHI				2			// BYTE2: 地址高8位
#define	MODBUS_ADDRESSLOW				3			// BYTE3: 地址低8位
#define	MODBUS_NUMBERHI					4			// BYTE4: 数据长度高8位
#define	MODBUS_NUMBERLOW				5			// BYTE5: 数据长度低8位
#define	MODBUS_MULTI_WRITE_BYTE_NUMBER	6			// BYTE6: 多寄存器写字节长度，8位
#define	MODBUS_MULTI_READ_BYTE_NUMBER	2			// BYTE2: 多寄存器读，回复帧，字节长度，8位
#define	MODBUS_FAULTINF					2			// BYTE2: 异常信息存储位置
#define	MODBUS_FAULT_CRCLOW				3			// BYTE3: 错误码CRC低8位存放位置
#define	MODBUS_FAULT_CRCHI				4			// BYTE4: 错误码CRC高8位存放位置

// MODBUS功能码定义
#define MODBUS_FUNCTION_MULTI_READ		0x0003		// 功能码：多寄存器读
#define MODBUS_FUNCTION_MULTI_WRITE		0x0010		// 功能码：多寄存器写
//通讯部分功能码

#define COMM_FUNCTION_HEAD_INFOR        0x50        // 功能码：实际为简易版帧头 

#define COMM_FUNCTION_BASIC_INFOR       0x16        // 功能码：仪表读取基本信息
#define COMM_FUNCTION_MORE_INFOR        0x21        // 功能码：仪表读取其余基本信息

// MODBUS故障信息定义
#define	MODBUS_FAULT_WRONGFUNCTION		1			// MODBUS错误：功能码错误，不支持
#define	MODBUS_FAULT_WRONGADDRESS		2			// MODBUS错误：数据地址错误
#define	MODBUS_FAULT_DATALENGTH			3			// MODBUS错误：数据长度超限
#define	MODBUS_FAULT_LENGTH				254			// MODBUS错误：多字节写入帧长度出错，写入数据数量与字节数量不是2倍关系
#define	MODBUS_FAULT_CRCFAILURE			255			// MODBUS错误：CRC校验值错误


// ------------------------------ 模块结构体 ---------------------------------
struct USART_State_Bits {
	u16	MODBUSState:4;							// MODBUS模块状态
	u16 rsvd:12;
};

union USART_State_Union {
	u16						all;
	struct USART_State_Bits	bit;
};

struct USART_State {
	union USART_State_Union		State_bits;

};

// Parameter 模块参数
struct MODBUS_DATA {
	u16* pBuffer;						// 参数存储指针
	u16	BlockLength;					// 参数块数据长度
	u16	BlockStartAddress;				// 参数块在MODBUS中起始地址
};

struct USART_PARAMETER {
	struct MODBUS_DATA	DataBlock[MODBUS_DATA_BLOCK_MAX];
};

struct USART_Variable {
	u16	TimeOutCount;							// 超时计数，单位1ms
	u16	SendByteLength;							// 发送字节长度
	u16	ReceiveByteLength;						// 接收字节长度
	u16	ReceiveByteNumber;						// 已接收字节数目	
	u16	ReceiveByteNumberOld;					// 已接收字节数目，旧的
	u16	MODBUSFaultInf;							// MODBUS错误信息
	u16	MODBUSCRCValue;							// ModbusCRC校验值	
	u16	MODBUSDataAddress;						// MODBUS 数据起始地址
	u16	MODBUSDataLength;						// MODBUS 数据长度
	u16	DataBlock;								// 数据读写块,0-15
};

struct USART_Output {
	u16	ReceiveBufferUpdated[16];				// 接收缓存数据更新标志，内部++，外部清零
};

// 统一架构考虑，缓存区设置为环形缓存，发送缓存，接收缓存三组。现在半双工模式下仅需一组缓存即可。
typedef struct {	
	struct USART_State		State;
	struct USART_PARAMETER	Parameter;
	struct USART_Variable	Variable;
	struct USART_Output		Output;
	u8	SendByteBuffer[BYTE_BUFFER_LENGTH];			// 发送字节缓存
	u8	ReceiveByteBuffer[BYTE_BUFFER_LENGTH];		// 接收字节缓存
} USART_MODBUS_structDef;


// -------------------------------------- 外部可调用函数 ------------------------------------------
void USART_MODBUS_DeviceInit(USART_MODBUS_structDef* v);			// 串口模块硬件初始化函数
void USART_MODBUS_Process(USART_MODBUS_structDef* v);				// 串口模块过程函数，运行在主循环中
void USART_MODBUS_TimeTick(USART_MODBUS_structDef* v);				// 串口模块定时函数，运行在1ms时基中


// End of USART_MODBUS.h


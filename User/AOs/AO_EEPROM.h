/* ==================================================================================

 File name:     AO_EEPROM.h
 Originator:    BLJ
 Description:   EEPROM状态机，读取写入相关信息，内附一个1s定时器，处理运行记录

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 06-30-2016     Version 0.9.1       长期测试版本
 04-12-2016     Version 0.9.0       测试功能通过
 01-02-2015     Version 0.0.1       测试功能通过
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                             // STM32器件寄存器定义头文件
#include "qpn_port.h"                               // 状态机头文件

// ------------------------------- 全局宏定义 ---------------------------------------

// flash模拟EEPROM,注意单位为半字，每组数据长度为16(32Bytes)，循环64组,总共2Kbytes
#define RECORD_CIRCLE_LENGTH            16          // 运行记录每个循环长度，单位半字
#define RECORD_CIRCLE_NUMBER            FLASH_EEPROM_BLOCK_LENGTH / RECORD_CIRCLE_LENGTH / 2            // 行车记录循环缓存个数，


#define EEPROM_WRITE_COMMAND_LENGTH     10           // EEPROM模块允许写入指令队列缓存长度


#define EEPROM_BLOCK_RECORD             0           // 运行记录
#define EEPROM_BLOCK_FAULT_HISTORY      1           // 故障历史记录
#define EEPROM_BLOCK_FAULT_COUNT        2           // 故障次数记录
#define EEPROM_BLOCK_CHECK              3           // 校验区
#define EEPROM_BLOCK_PARAMETER          4           // 设备信息及参数
#define EEPROM_BLOCK_ALL                5           // 全局擦除

// -------------------------- 显示模块活动对象结构体 ---------------------------------
struct AO_EEPROM_State {
    u16 RecordWriteAsk;                     // 运行记录写入请求
};

struct AO_EEPROM_Variable_bits {
    u32 Block:4;                            // Variable: EEPROM has five blocks for information, parameter, record ......
    u32 BlockStack1:4;                      // Variable: EEPROM block in stack
    u32 BlockStack2:4;                      // Variable: EEPROM block in stack
    u32 StackCount:4;                       // Variable: block numbers in stack
    u32 SystemRecordPageFoundFlag:1;        // 系统运行记录所在页被找到
    u32 SystemRecordPage:2;                 // 系统运行记录所在页
    u32 SystemRecordFoundFlag:1;            // Variable: The last vehicle record has been found flag
    u32 LastRecordLRCFail:1;                // 最后一次运行记录LRC校验错误标志位
    u32 rsvd:11;
};

struct AO_EEPROM_Variable_union {
    u32                             all;
    struct AO_EEPROM_Variable_bits  bit;
};

struct AO_EEPROM_Variable {
    struct  AO_EEPROM_Variable_union    Variable_bits;
    u32 TotalTimeRecord;                    // Variable: use for total time record in first read
    u32 mileageOld;                         // Variable: Old mileage， use for record as mileage 100m once
    u16 EEPROMFaultInf;                     // Variable: EEPROM fault information
    u16 RecordWriteDelay;                   // Variable: 运行记录写入EEPROM延时计数，单位s
    u16 EEPROMWriteBlock;                   // Variable: EEPROM写入指令区块
    u16 EEPROMWriteBlockWait[EEPROM_WRITE_COMMAND_LENGTH];          // 等待写入区块
    u16 EEPROMWriteBlockPointer;            // 等待写入区块个数
    u16 SystemRecordCircleNumber;           // 系统运行记录数组编号，0-63  *2
    u16 SystemRecordRetryCount;             // 系统运行记录读取重试次数，若读取的运行记录LRC校验出错，则读取上一次记录，此变量++，限制重试次数
    u16 RecordLRCFail;                      // for test: 运行记录有错误
};

typedef struct AO_EEPROMTag {                      
    QActive super;                              // derive from QActive
    struct  AO_EEPROM_State     State;          // 模块状态
    struct  AO_EEPROM_Variable  Variable;       // 模块临时变量
} AO_EEPROM;

// ------------------------------ 声明清除函数 ---------------------------------
void AO_EEPROM_ctor(AO_EEPROM * const me);

// ------------------------------ 声明活动状态 ---------------------------------
static QState AO_EEPROM_initial(AO_EEPROM * const me);                          // 初始状态
    static QState AO_EEPROM_StartWait(AO_EEPROM * const me);                    // 1.启动等待状态
    static QState AO_EEPROM_FirstRead(AO_EEPROM * const me);                    // 2.上电读取EEPROM信息，默认跳转至AO_EEPROM_ReadCheckInf
        static QState AO_EEPROM_ReadCheckInf(AO_EEPROM * const me);             // 3.读取校验区信息
        static QState AO_EEPROM_SearchRecord(AO_EEPROM * const me);             // 4.寻找最新运行记录
        static QState AO_EEPROM_ReadRecord(AO_EEPROM * const me);               // 5.读取运行记录
        static QState AO_EEPROM_FixRecord(AO_EEPROM * const me);                // 51.最后一次运行记录错误，修复之
        static QState AO_EEPROM_ReadParameter(AO_EEPROM * const me);            // 6.读取参数   
        static QState AO_EEPROM_FixParameter(AO_EEPROM * const me);             // 61.修复参数  
		static QState AO_EEPROM_UpdateInformation(AO_EEPROM * const me);        // 63.软件版本更新,重写参数EEPROM，清除故障记录
        static QState AO_EEPROM_ReadFaultHistory(AO_EEPROM * const me);         // 62.读取故障历史    
    static QState AO_EEPROM_Normal(AO_EEPROM * const me);                       // 7.EEPROM正常处理状态，默认跳转至AO_EEPROM_Idle
        static QState AO_EEPROM_Idle(AO_EEPROM * const me);                     // 8.EEPROM空闲状态，可接收写入读取指令
        static QState AO_EEPROM_Write(AO_EEPROM * const me);                    // 9.EEPROM写入状态，可接收写入读取指令
    static QState AO_EEPROM_Fault(AO_EEPROM * const me);                        // 100.EEPROM出错，停止工作，仅可处理紧急事件，此时可能无法写入数据
    static QState AO_EEPROM_Initial(AO_EEPROM * const me);                      // 20.EEPROM初始化，擦除并写入默认数据
        static QState AO_EEPROM_Erase(AO_EEPROM * const me);                    // 21.擦除EEPROM
        static QState AO_EEPROM_InitialRecord(AO_EEPROM * const me);            // 22.初始化运行记录
        static QState AO_EEPROM_InitialFaultHistory(AO_EEPROM * const me);      // 23.初始化故障历史记录
//      static QState AO_EEPROM_InitialFaultCount(AO_EEPROM * const me);        // 24.初始化故障次数记录
        static QState AO_EEPROM_InitialParameter(AO_EEPROM * const me);         // 25.初始化系统参数


// End of AO_EEPROM.h


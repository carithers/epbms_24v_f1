/* ==================================================================================

 File name:     flash_EEPROM.h
 Originator:    BLJ
 Description:   片上flash模拟EEPROM，实现flash页擦除，连续半字写入，读取功能。
 Take care:     flash写入前必须先擦除并确认数据全部为0xFFFF。擦除或写入会短时间停止运行，
                严禁在电机运行或三相全桥驱动使能时进行flash操作！！！

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-22-2016		Version 1.1.1			修正读写指令函数LRC校验配置错误
 04-19-2016     Version 1.1.0           修正bug，功能升级，完善代码，长期验证测试版本
 09-15-2015     Version 1.0.0           第一版
 08-24-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/


#include "stm32f0xx.h"                            // STM32器件寄存器定义头文件


// ---------------------------- 模块内部宏定义 -----------------------------------------
// flash write/read command
#define FLASH_EEPROM_WRITE                1         // 写指令步骤/状态
#define FLASH_EEPROM_READ                 2         // 读指令步骤/状态


// ---------------------------- flash模拟EEPROM模块结构体 ------------------------------
struct flash_EEPROM_State_bits {
    u16 Command:2;                          // flash EEPROM 读写指令
    u16 CommandFinish:1;                    // 读写指令正常完成，内部置位，需外部清除。此标志位作为发送EEPROM_FINISH_SIG事件的条件之一
    u16 LRCCheckEnable:1;                   // 使能末尾LRC校验
    u16 LRCCheckFail:1;                     // LRC校验错误
    u16 rsvd:11;
};

union flash_EEPROM_State_union {
    u16                             all;
    struct flash_EEPROM_State_bits  bit;
};

struct flash_EEPROM_Fault_bits {
    u16 FlashEraseFail:1;                       // flash擦除失败
    u16 FlashEraseAddressWrong:1;               // flash擦除地址错误
    u16 FlashWriteFail:1;                       // flash写入失败
    u16 FlashWriteForbid:1;                     // flash写入禁止，flash内数据不为0xFFFF
    u16 FlashWriteCheckFail:1;                  // flash写入数据检查不一致
    u16 rsvd:11;
};

union flash_EEPROM_Fault_union {
    u16                             all;
    struct flash_EEPROM_Fault_bits  bit;
};

struct flash_EEPROM_State {
    union flash_EEPROM_State_union    State_bits;
    union flash_EEPROM_Fault_union    Fault_bits;
};

struct flash_EEPROM_Parameter {
    u32     Address;                            // EEPROM数据地址，单位为字节Byte
    u16     DataLength;                         // 读取写入数据个数长度，单位半字
    u16*    pBuffer;                            // 外部数据RAM起始地址指针
};

struct flash_EEPROM_Variable {    
    u16     DataCnt;                            // 读取写入数据个数计数，单位半字
    u16     Data;                               // 读取写入数据，半字
    u16     LRCValue;                           // LRC校验值
};

// flash模拟EEPROM模块结构体
typedef struct {    
    struct flash_EEPROM_State       State;
    struct flash_EEPROM_Parameter   Parameter;
    struct flash_EEPROM_Variable    Variable;
} flash_EEPROM_structDef;


// ---------------------------- 外部可调用函数 ---------------------------------------
void flash_EEPROM_DeviceInit(void);                         // flash模拟EEPROM模块硬件初始化，运行在硬件初始化中
void flash_EEPROM_Process(flash_EEPROM_structDef* v);       // flash模拟EEPROM模块进程函数，运行在主循环中
    
// I2C EEPROM读取指令函数,返回值0：正常；1：错误
// pBuffer：读取数据在RAM中存放起始地址
// Address：数据在flash中存放起始地址
// DataNumToRead：待读取数据个数，单位半字
// LRCCheckEnable：LRC校验使能
u16 flash_EEPROM_Read(flash_EEPROM_structDef* v, u16* pBuffer, u32 Address, u16 DataNumToRead, u16 LRCCheckEnable);  

// I2C EEPROM写入指令函数,返回值0：正常；1：错误
// pBuffer：写入数据在RAM中存放起始地址
// Address：数据在flash中存放起始地址
// DataNumToRead：待写入数据个数，单位半字
// LRCCheckEnable：LRC校验值写入使能，写在数据区域最后
u16 flash_EEPROM_Write(flash_EEPROM_structDef* v, u16* pBuffer, u32 Address, u16 DataNumToWrite, u16 LRCCheckEnable);   

// flash模拟EEPROM擦除函数，即使函数，会造成约20msCPU无响应   
// Address：数据在flash中存放起始地址，此值需为芯片flash页起始地址
u16 flash_EEPROM_Erase(flash_EEPROM_structDef* v, u32 Address);             


// End of flash_EEPROM.h


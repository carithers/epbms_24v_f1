/* ==================================================================================

 File name:     flash_EEPROM.c
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


#include "target.h"                     // 目标板硬件选择头文件
#include "flash_EEPROM.h"               // flash模拟EEPROM模块头文件


// --------------------- flash模拟EEPROM模块硬件初始化,运行在硬件初始化中 -------------------------
void flash_EEPROM_DeviceInit(void)
{
    FLASH_Unlock();                     // Flash unlock   解锁flash
      
    // Clear all FLASH flags
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_FLAG_PGERR | FLASH_FLAG_BSY);     
}


// --------------------- flash模拟EEPROM模块数据读取函数，内部调用 -----------------------
void flash_EEPROM_Read_Process(flash_EEPROM_structDef* v)
{   
    // 数据未全部读取
    if (v->Variable.DataCnt < v->Parameter.DataLength)
    {
        // 读取flash数据,每次读取一个半字，注意:读取地址单位为字节，必须是偶数，已由读取指令函数保证
        v->Variable.Data = *(__IO uint16_t*)(v->Parameter.Address + v->Variable.DataCnt * 2);
        
        // 校验数据，LRC校验，16bit
        if (v->State.State_bits.bit.LRCCheckEnable > 0)
        {
            v->Variable.LRCValue += v->Variable.Data;
        }
        
        // 数据存储到ram
        *(v->Parameter.pBuffer + v->Variable.DataCnt) = v->Variable.Data;
        
        v->Variable.DataCnt++;          // 已读取数据个数++
        
        // 数据全部读取完成
        if (v->Variable.DataCnt >= v->Parameter.DataLength)
        {
            // 校验和验证
            if (v->State.State_bits.bit.LRCCheckEnable > 0)
            {
                v->Variable.LRCValue = ~v->Variable.LRCValue;// LRC校验和累加后取反
                
                // 校验和存储在数据区末尾，注意此处DataCnt已再次+1
                if (v->Variable.LRCValue != *(__IO uint16_t*)(v->Parameter.Address + v->Variable.DataCnt * 2))
                {
                    v->State.State_bits.bit.LRCCheckFail = 1;       // LRC校验不通过
                }
            }
            
            // 数据已全部读取完成
            v->State.State_bits.bit.Command = 0;                // 清除指令，允许新的指令下达
            v->State.State_bits.bit.CommandFinish = 1;          // 置位完成标志位，外部触发状态机事件//主循环中有函数可以根据标志位进行信号发送触发状态机
        }
    }
}


// --------------------- flash模拟EEPROM模块数据写入函数，内部调用 ------------------------
void flash_EEPROM_Write_Process(flash_EEPROM_structDef* v)
{
    // 写入数据前先读取flash，检查是否为0xFFFF，否则禁止写入
    if (*(__IO uint16_t*)(v->Parameter.Address + v->Variable.DataCnt * 2) == 0xFFFF)  //Datacnt 读取写入数据个数计数，单位半字
    {
        // 数据未全部写入
        if (v->Variable.DataCnt < v->Parameter.DataLength)
        {
            v->Variable.Data = *(v->Parameter.pBuffer + v->Variable.DataCnt);
            
            // 写入数据至flash，每次写入一个半字，注意写入地址必须为偶数
            // Take care: FLASH_ProgramHalfWord函数会造成CPU短时无响应，注意影响
            if (FLASH_ProgramHalfWord(v->Parameter.Address + v->Variable.DataCnt * 2, v->Variable.Data) == FLASH_COMPLETE)
            {
                // 写入成功，读取数据进行检查，二次确认写入是否成功
                if (*(__IO uint16_t*)(v->Parameter.Address + v->Variable.DataCnt * 2) == v->Variable.Data)
                {
                    // 确认数据写入成功
                    
                    // 校验数据，LRC校验，16bit
                    if (v->State.State_bits.bit.LRCCheckEnable > 0)
                    {
                        v->Variable.LRCValue += v->Variable.Data;
                    }   

                    // 数据全部写入完成
                    v->Variable.DataCnt++;      
                    
                    // 数据全部写入完成
                    if (v->Variable.DataCnt >= v->Parameter.DataLength) 
                    {
                        // 校验和验证
                        if (v->State.State_bits.bit.LRCCheckEnable == 0)
                        {
                            // 数据已全部写入完成
                            v->State.State_bits.bit.Command = 0;
                            v->State.State_bits.bit.CommandFinish = 1;  
                        }
                        else
                        {
                            v->Variable.LRCValue = ~v->Variable.LRCValue;
                            
                            // 写入校验值至flash
                            if (FLASH_ProgramHalfWord(v->Parameter.Address + v->Variable.DataCnt * 2, v->Variable.LRCValue) == FLASH_COMPLETE)
                            {   
                                // 写入成功，读取数据进行检查，二次确认写入是否成功
                                if (*(__IO uint16_t*)(v->Parameter.Address + v->Variable.DataCnt * 2) == v->Variable.LRCValue)
                                {                               
                                
                                    // 数据已全部写入完成
                                    v->State.State_bits.bit.Command = 0;
                                    v->State.State_bits.bit.CommandFinish = 1;          
                                }
                                else
                                {
                                    // 写入数据检查不一致                              
                                    v->State.Fault_bits.bit.FlashWriteCheckFail = 1;        // flash写入数据检查不一致
                                    
                                    v->State.State_bits.bit.Command = 0;                    // 指令清除，允许接收新指令                       
                                }
                            }   
                            else
                            {
                                // 写入失败
                                v->State.Fault_bits.bit.FlashWriteFail = 1;     // flash写入失败标志位，可能造成后续写入操作都无法执行
                                v->State.State_bits.bit.Command = 0;            
                            }                       
                        }
                    }                   
                }
                else
                {
                    // 写入数据检查不一致                  
                    v->State.Fault_bits.bit.FlashWriteCheckFail = 1;        // flash写入数据检查不一致
                    
                    v->State.State_bits.bit.Command = 0;                    // 指令清除，允许接收新指令                       
                }
            }
            else
            {
                // 写入失败            
                v->State.Fault_bits.bit.FlashWriteFail = 1;             // flash写入失败标志位，可能造成后续写入操作都无法执行
                
                v->State.State_bits.bit.Command = 0;                    // 指令清除，允许接收新指令   
            }       
        }
    }
    else                // flash内数据不为0xFFFF，禁止写入
    {       
        v->State.Fault_bits.bit.FlashWriteForbid = 1;           // 故障标志位：flash内数据不为0xFFFF，禁止写入
        
        v->State.State_bits.bit.Command = 0;                    // 指令清除，允许接收新指令   
    }
}


// --------------------- flash模拟EEPROM模块进程函数 ---------------------------
void flash_EEPROM_Process(flash_EEPROM_structDef* v)
{
    switch (v->State.State_bits.bit.Command)            // 根据指令选择调用读取，写入过程函数
    {
        case FLASH_EEPROM_WRITE:
        {
            flash_EEPROM_Write_Process(v);
        }
        break;
        
        case FLASH_EEPROM_READ:
        {
            flash_EEPROM_Read_Process(v);               // 执行EEPROM读取函数
        }
        break;
    }
}

// ------------------------- I2C EEPROM读取指令,返回值0：正常；1：错误 -----------------------------------
// pBuffer:内存区域指针
// Address:存储区(Flash)地址
// DataNumToRead:需要读取的数据数量
// LRCCheckEnable:LRC校验使能
u16 flash_EEPROM_Read(flash_EEPROM_structDef* v, u16* pBuffer, u32 Address, u16 DataNumToRead, u16 LRCCheckEnable)
{
    // 上一条指令已处理完成 && 无错误发生 && 地址大于等于起始地址 && 地址小于等于结束地址 && 读取参数个数大于0 && 读取数据未超出1页范围
    // Take care：仔细测试
    if (v->State.State_bits.bit.Command == 0 
        && v->State.Fault_bits.all == 0
        && Address >= FLASH_EEPROM_START_ADDRESS
        && (Address < (FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH * FLASH_EEPROM_BLOCK_NUMBER))
        && DataNumToRead > 0
        && ((Address % FLASH_EEPROM_BLOCK_LENGTH + DataNumToRead * 2) <= FLASH_EEPROM_BLOCK_LENGTH))
    {
        // Take care：当校验和使能时，数据区末尾会多存储16bit的校验值，所以再次判断超限
        if (LRCCheckEnable > 0)
        {
            if ((Address % FLASH_EEPROM_BLOCK_LENGTH + DataNumToRead * 2) >= FLASH_EEPROM_BLOCK_LENGTH)
            {
                return 1;
            }
        }   
        
        v->State.State_bits.bit.LRCCheckEnable = LRCCheckEnable;        // 设置是否需要LRC校验
        v->Variable.DataCnt = 0;                                        // 已读取数据计数清零
        
        v->State.State_bits.bit.LRCCheckFail = 0;                       // 清除LRC校验失败标志位
        v->Variable.LRCValue = 0;       
        
        // 设置ram区数据地址，EEPROM地址，写入字节
        v->Parameter.pBuffer = pBuffer;
        v->Parameter.Address = Address;
        v->Parameter.DataLength = DataNumToRead;
        
        v->State.State_bits.bit.Command = FLASH_EEPROM_READ;//置位读取命令，在主循环中进行更新后发送事件信号来触发状态机
        
        return 0;
    }
    else
    {
        return 1;
    }   
}

// ----------------------------- flash模拟EEPROM写入指令,返回值0：正常；1：错误 -------------------------------------
// pBuffer:内存区域指针
// Address:存储区(Flash)地址
// DataNumToWrite:需要写入的数据数量
// LRCCheckEnable:LRC校验使能
u16 flash_EEPROM_Write(flash_EEPROM_structDef* v, u16* pBuffer, u32 Address, u16 DataNumToWrite, u16 LRCCheckEnable)
{
    // 上一条指令已处理完成 && 无错误发生 && 地址大于等于起始地址 && 地址小于等于结束地址 && 读取参数个数大于0 && 读取数据未超出1页范围
    // Take care：仔细测试  
    if (v->State.State_bits.bit.Command == 0 
        && v->State.Fault_bits.all == 0
        && Address >= FLASH_EEPROM_START_ADDRESS
        && (Address < (FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH * FLASH_EEPROM_BLOCK_NUMBER))
        && DataNumToWrite > 0
        && ((Address % FLASH_EEPROM_BLOCK_LENGTH + DataNumToWrite * 2) <= FLASH_EEPROM_BLOCK_LENGTH))   
    {
        // Take care：当校验和使能时，数据区末尾会多存储16bit的校验值，所以再次判断超限
        if (LRCCheckEnable > 0)
        {
            if ((Address % FLASH_EEPROM_BLOCK_LENGTH + DataNumToWrite * 2) >= FLASH_EEPROM_BLOCK_LENGTH)
            {
                return 1;
            }
        }           
        
        v->State.State_bits.bit.LRCCheckEnable = LRCCheckEnable;        // 设置是否需要LRC校验
       
        v->Variable.LRCValue = 0;                                       // 清除LRC校验临时变量
        v->Variable.DataCnt = 0;
        
        // 设置ram区数据地址，EEPROM地址，写入字节
        v->Parameter.pBuffer = pBuffer;
        v->Parameter.Address = Address;
        v->Parameter.DataLength = DataNumToWrite;
        
        v->State.State_bits.bit.Command = FLASH_EEPROM_WRITE;
        
        return 0;
    }
    else
    {
        return 1;
    }   
}

// ------------------ flash擦除函数，会造成CPU约10ms无响应，注意影响 ------------------------- 
u16 flash_EEPROM_Erase(flash_EEPROM_structDef* v, u32 Address)
{
    // 地址必须保证为1页的起始地址 && 地址必须大于等于EEPROM区域起始地址 && 地址必须小于等于最后1页的地址
    if ((Address % FLASH_EEPROM_BLOCK_LENGTH) == 0
        && Address >= FLASH_EEPROM_START_ADDRESS
        && FLASH_EEPROM_START_ADDRESS <= (FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH * (FLASH_EEPROM_BLOCK_NUMBER - 1)))
    {
        // Take care: FLASH_ErasePage函数会造成CPU约10ms无响应，注意影响
        if (FLASH_ErasePage(Address) == FLASH_COMPLETE)
        {
            // 擦除成功，无需置位指令完成标志位
            return 0;               // flash 擦除成功
        }
        else
        {
            // flash 擦除失败
            v->State.Fault_bits.bit.FlashEraseFail = 1;         

            return 1;               // Flash擦除失败
        }               
    }
    else
    {
        v->State.Fault_bits.bit.FlashEraseAddressWrong = 1;             // flash擦除地址错误
        
        return 1;
    }
}


// End of flash_EEPROM.c



/* ==================================================================================

 File name:     AO_EEPROM.c
 Originator:    BLJ
 Description:   EEPROM状态机，读取写入相关信息，内附一�?s定时器，处理运行记录
 Take care�?	所有状态活动对象，定时�?用于周期性定�?

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 06-30-2016     Version 0.9.1       长期测试版本
 04-12-2016     Version 0.9.0       测试功能通过
 12-07-2014     Version 0.0.1       测试功能通过
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                         // STM32器件寄存器定义头文件
#include "string.h"                             // 调用memset函数�?
#include "system_core.h"                        // 系统核心头文�?
#include "qpn_port.h"                           // 状态机头文�?

// ---------------------- 状态机运行记录，测试用 -----------------------------
//#define   AO_DEBUG    1

//  ID


#ifdef AO_DEBUG

u16 AO_Record[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void t_Record(u16 i)
{
    u16 n = 0;

    for (n = 9; n > 0; n--)
    {
        AO_Record[n] = AO_Record[n-1];
    }

    AO_Record[0] = i;
}

#endif              // 测试代码结束


// -------------------------------- 测试变量 ---------------------------------

// ------------------------------ 模拟内部可调用功能函�?--------------------------------
void SystemRecord_Update(AO_EEPROM * const me);                 // 系统运行记录更新函数�?s运行一�?

    
// ------------------------------ EEPROM活动对象变量 ---------------------------- 
// 64K flash MM32SPIN06 取最后5页，每页1K，做EEPROM   
// Page0：运行故障历史记录和故障次数记录
// Page1：系统信息及校验值（Check Inf.)
// Pag:2-3: 运行记录g_SystemRecord，占2页，交替写入
// Page4: 系统参数

// EEPROM写入地址                                                       1K                 * 2
const u32   EEPROM_Block_addr[] = { FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH * 2,     \
                                    FLASH_EEPROM_START_ADDRESS,     \
                                    FLASH_EEPROM_START_ADDRESS,     \
                                    FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH,     \
                                    FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH * 4,     \
                                    0x0000};

// EEPROM数据在RAM中的地址
/*
g_SystemRecord;// 设备运行记录
g_SystemFaultHistory;// 系统故障记录
g_SystemState;// 系统状态.EEPROM中的校验信息
g_SystemParameter;// 系统参数
g_SystemMonitor;// 系统采样值	
g_SystemRecord;// 设备运行记录									
*/															
u16*    EEPROM_Block_pBuf[] = {(u16*)&g_SystemRecord,       \
                               (u16*)&g_SystemFaultHistory,        \
                               (u16*)&g_SystemRecord,      \
                               (u16*)&g_SystemState.Variable.EEPROMCheckValue,   \
                               (u16*)&g_SystemParameter,       \
                               (u16*)&g_SystemRecord};

// 块区长度,单位为半字，2BYTES
// Take care: 数组长度必须正确，再三检验
// Take care：数0为运行记录，默认占用30Byte，即15个半字，带LRC校验，占1个半字，总长16个半字
// Take care：MCU ID 加密区默认长度为6个半字
const u16   EEPROM_Block_length[] = {15,                \
                                    sizeof(g_SystemFaultHistory)/2,             \
                                    7,              \
                                    6,              \
                                    sizeof(g_SystemParameter)/2,                \
                                    1024};

// 对应区块LRC校验使能位
const u16   EEPROM_Block_LRCEnable[] = {1, 0, 0, 0, 1, 0};


// -------------------------------- 清除函数 ------------------------------------
void AO_EEPROM_ctor(AO_EEPROM * const me) {
    QActive_ctor(&me->super, Q_STATE_CAST(&AO_EEPROM_initial));
}

// -------------------------------- 初始状�?------------------------------------
QState AO_EEPROM_initial(AO_EEPROM * const me) {
    return Q_TRAN(&AO_EEPROM_StartWait);
}

// ------------------------------- 1.启动等待，一段时间，再读取EEPROM ----------------------------------
QState AO_EEPROM_StartWait(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(1);
#endif
                
            QActive_armX((QActive *)me, 0U, 5U);      		// 开启定时器0  5ms==================================================调试中先关闭EEPROM

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:                             		// 定时�?被触�?
        {            
            status = Q_TRAN(&AO_EEPROM_FirstRead);      		// 跳转至启动读取状态，读取EEPROM数据        
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// 父状态为top状�?      
        }
        break;
    }
    
    return status;
}

// ------------------------- 2.启动读取状态，读取EEPROM数据 ---------------------------
QState AO_EEPROM_FirstRead(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(2);
#endif
                            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_ReadCheckInf);   		// 跳转至子状态，读取校验信息
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时
            
            status = Q_HANDLED();
        }
        break;
																
        case Q_TIMEOUT_SIG:                             		// 定时器被触发，超时
        {
			
            // 启动读取EEPROM信息发生超时事件时，统一由AO_EEPROM_FirstRead状态处�?
            
            // 发送FAULT_SIG，设置全局故障标志，跳转至故障状�?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_OVERTIME);
            
            status = Q_TRAN(&AO_EEPROM_Fault);          		// 读取校验区信息超时。跳转至EEPROM故障状�?
        }
        break;
        
        case EEPROM_FAIL_SIG:                         		    // EEPROM操作失败标志�?
        {
			
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时�?
            
            // 发送FAULT_SIG，设置全局故障标志，跳转至故障状�?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_FAIL);
            
            status = Q_TRAN(&AO_EEPROM_Fault);          		// 读取校验区信息超时。跳转至EEPROM故障状�?         
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// 父状态为top状�?
        }
        break;
    }
    
    return status;
}

u16 t_id[6];

// ----------------------------- 3.读取校验信息 ------------------------------------
QState AO_EEPROM_ReadCheckInf(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(3);
#endif
            
            // 读取EEPROM校验区信息   6个半字信息
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_CHECK], 
                                EEPROM_Block_addr[EEPROM_BLOCK_CHECK], 
                                EEPROM_Block_length[EEPROM_BLOCK_CHECK], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_CHECK]);
            
            QActive_armX((QActive *)me, 0U, 500U);      		// 开启定时器0�?00ms
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                         		// EEPROM校验区读取成�?
        {
			
			
            //status = Q_HANDLED();//可以删除-------------------
            
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时�?
            
            // 读取芯片ID号，96�?
            g_SystemState.Variable.MCUIDNumber[0] = *(__IO uint32_t*)(MCU_ID_NUMBER);
            g_SystemState.Variable.MCUIDNumber[1] = *(__IO uint32_t*)(MCU_ID_NUMBER + 4);
            g_SystemState.Variable.MCUIDNumber[2] = *(__IO uint32_t*)(MCU_ID_NUMBER + 8);   

        /*  t_id[0] = ((((((g_SystemState.Variable.MCUIDNumber[0] & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[0] >> 8) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) << 8);
            t_id[1] = (((((((g_SystemState.Variable.MCUIDNumber[0] >> 16) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[0] >> 24) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) << 8);
            
            t_id[2] = ((((((g_SystemState.Variable.MCUIDNumber[2] & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[2] >> 8) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) << 8);
            t_id[3] = (((((((g_SystemState.Variable.MCUIDNumber[2] >> 16) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[2] >> 24) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) << 8);

            t_id[4] = ((((((g_SystemState.Variable.MCUIDNumber[1] & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[1] >> 8) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) << 8);
            t_id[5] = (((((((g_SystemState.Variable.MCUIDNumber[1] >> 16) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[1] >> 24) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) << 8);
        */
        
            // MCU id号验证通过，转入下一步骤，跳转至寻找最新运行记录状�?
            me->Variable.Variable_bits.bit.SystemRecordPage = 0;
            me->Variable.SystemRecordCircleNumber = 0;
            me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 0;       // 系统运行记录所在页已找到标志位，清�?
            me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 0;           // 系统运行记录已找到标志位，清�?
            me->Variable.SystemRecordCircleNumber = 0;                          // 系统运行记录所在数组编号，清零
            
            status = Q_TRAN(&AO_EEPROM_SearchRecord);           
            
        /*  if ((g_SystemState.Variable.EEPROMCheckValue[0] == (u16)((((((g_SystemState.Variable.MCUIDNumber[0] & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[0] >> 8) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[1] == (u16)(((((((g_SystemState.Variable.MCUIDNumber[0] >> 16) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[0] >> 24) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[2] == (u16)((((((g_SystemState.Variable.MCUIDNumber[2] & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[2] >> 8) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[3] == (u16)(((((((g_SystemState.Variable.MCUIDNumber[2] >> 16) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[2] >> 24) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[4] == (u16)((((((g_SystemState.Variable.MCUIDNumber[1] & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[1] >> 8) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[5] == (u16)(((((((g_SystemState.Variable.MCUIDNumber[1] >> 16) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[1] >> 24) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) << 8)))
            {
                // MCU id号验证通过，转入下一步骤，跳转至寻找最新运行记录状�?
                me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 0;       // 系统运行记录所在页已找到标志位，清�?
                me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 0;           // 系统运行记录已找到标志位，清�?
                me->Variable.SystemRecordCircleNumber = 0;                          // 系统运行记录所在数组编号，清零
                status = Q_TRAN(&AO_EEPROM_SearchRecord);   
            }
            else                                        		// 校验区错误，跳转至fault状�?
            {
                // 设置EEPROM校验读取错误标志位，FAULT_EEPROM_FAILURE
                Protect_SetFaultCodeLv0(&g_Protect, FAULT_EEPROM_FAILURE);                  
                
                // 跳转至EEPROM初始化状态，擦除EEPROM，然后重新写入初始数�?
                status = Q_TRAN(&AO_EEPROM_Initial);    

                // 跳转至故障状�?
            //  status = Q_TRAN(&AO_EEPROM_Fault);
            //  status = Q_TRAN(&AO_EEPROM_SearchRecord);   
                
            }*/
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);         	// 父状态为AO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 4.寻找最新的行车记录 ------------------------------------
// Page1-2作为运行记录使用，每一条记录长度为16BYTE，一页可存放64组记录，两页做互为备�?
// 擦除逻辑：上电时，擦除不需要的一页。系统进入睡眠时，擦除需要的一页�?
// Take care�?若无可写入的区块，报错或强制擦除一页�?
QState AO_EEPROM_SearchRecord(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(4);
#endif
            
            // 读取EEPROM运行记录一次，从编�?-最�?
            // 第一次运行时SystemRecordCircleNumber == 0
            // 读取的运行记录直接存入运行记录数据结构体
            // Take care，寻找最新运行记录仅读取�?Byte，即总运行时间，不做LRC校验   
            // flash模拟EEPROM，一页有64组数据，0-63为第一页，64-127为紧邻的第二�?
            // Take care：此指令仅读取前2个半字数据，不做LRC校验
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD],                          \
                                EEPROM_Block_addr[EEPROM_BLOCK_RECORD]                                          \
                                + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage   \
                                + 2 * RECORD_CIRCLE_LENGTH * me->Variable.SystemRecordCircleNumber,             \
                                2, 0);
																
            QActive_armX((QActive *)me, 0U, 500U);      		// 开启定时器0�?00ms 
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                         		// EEPROM校验区读取成�?
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时�?
           
			
            // 保障代码，若无返回函数，则跳转故障状态，预防QP系统出错
            status = Q_TRAN(&AO_EEPROM_Fault);
            
            // 首先查找运行数据所在页，若需要则擦除另一�?
            // 第一次读取地址0，第二次读取地址64，比较两次数�?
            // 可能�?：其中一个有数值，另一个为0xFFFFFFFF，有数值一页为所需页，另一页已擦除
            // 可能�?：两个都有数值，大的一页为所需页，另一页需要擦�?
            // 可能�?：两个都�?xFFFFFFFF，第一页为所需页，另一页已擦除,且数据已找到            
            
            if (me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag == 0)      // 运行记录所在页暂未找到
            {
                if (me->Variable.Variable_bits.bit.SystemRecordPage == 0)           // 第一页起始数据已读取，则读取第二�?
                {
                    // 记录第一页数据值，作为后面比较依据
                    me->Variable.TotalTimeRecord = g_SystemRecord.TotalTime;
                    
                    // 查找第二页第一个数�?
                    me->Variable.Variable_bits.bit.SystemRecordPage = 1;
                    
                    status = Q_TRAN(&AO_EEPROM_SearchRecord);                       // 跳转至寻找最新运行记录状态，再次读取运行记录
                }
                else                    // 第二页起始数据已读取，比较两数据
                {
                    // 第一次读取地址0，第二次读取地址64，比较两次数�?
                    // 可能�?：其中一个有数值，另一个为0xFFFFFFFF，有数值一页为所需页，另一页已擦除
                    // 可能�?：两个都有数值，大的一页为所需页，另一页需要擦�?
                    // 可能�?：两个都�?xFFFFFFFF，第一页为所需页，另一页已擦除,EEPROM无数据，重新初始�?
                    // me->Variable.TotalTimeRecord�?为第一页起始地址数据                 
                    // g_SystemRecord.TotalTime�?为第二页起始地址数据
                    if (g_SystemRecord.TotalTime == 0xFFFFFFFF && me->Variable.TotalTimeRecord == 0xFFFFFFFF)
                    {
                        // EEPROM中无数据，执行数据初始化操作
                        // 此处直接跳转至AO_EEPROM_FixRecord，执行初始EEPROM数据写入修复操作
                       
                        // 见可能�?
                        me->Variable.Variable_bits.bit.SystemRecordPage = 0;
                        me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 1;       // 置位运行记录所在页已找到标志位
                        
                        // 系统运行记录数据已找�?
                        me->Variable.SystemRecordCircleNumber = 0;
                        me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 1;           // 置位运行记录已找到标志位
                        
                    //    status = Q_TRAN(&AO_EEPROM_ReadRecord);                           // 跳转至读取运行记录状态，读取运行记录
                        
                        // 清除系统运行记录,并写�?
                        memset(&g_SystemRecord, 0, sizeof(g_SystemRecord));                 // 设备运行记录
                        
                        // 运行记录无法读取时，默认设置电池电量�?,第一次充电充满会自动重置电量
                        g_AO_BMS.Output.BatteryCapacity = 0;
                        
                        // 跳转至修复运行记录状态，并清除运行记录错误标志位
                        me->Variable.Variable_bits.bit.LastRecordLRCFail = 0;
                        
                        status = Q_TRAN(&AO_EEPROM_FixRecord);      // 跳转至修复运行记录状�?                     
                    }
                    else if (g_SystemRecord.TotalTime != 0xFFFFFFFF && me->Variable.TotalTimeRecord == 0xFFFFFFFF)
                    {
                        // 见可能�?
                        me->Variable.Variable_bits.bit.SystemRecordPage = 1;
                        me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 1;       // 置位运行记录所在页已找到标志位     

                        status = Q_TRAN(&AO_EEPROM_SearchRecord);                   // 跳转至寻找最新运行记录状态，再次读取运行记录                       
                    }
                    else if (g_SystemRecord.TotalTime == 0xFFFFFFFF && me->Variable.TotalTimeRecord != 0xFFFFFFFF)
                    {
                        // 见可能�?
                        me->Variable.Variable_bits.bit.SystemRecordPage = 0;
                        me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 1;       // 置位运行记录所在页已找到标志位

                        status = Q_TRAN(&AO_EEPROM_SearchRecord);                   // 跳转至寻找最新运行记录状态，再次读取运行记录
                    }       
                    else
                    {
                        // 可能�?,擦除另一�?
                        if (g_SystemRecord.TotalTime > me->Variable.TotalTimeRecord)
                        {
                            // �?页数值大于第1页数值，擦除�?页数据，此处会造成mcu 20ms暂停
                            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_RECORD]);

                            me->Variable.Variable_bits.bit.SystemRecordPage = 1;            // 最新数据存放在�?页中，编�?                              
                        }
                        else
                        {
                            // �?页数值大于第2页数值，擦除�?页数据，此处会造成mcu 20ms暂停
                            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_RECORD] + FLASH_EEPROM_BLOCK_LENGTH);

                            me->Variable.Variable_bits.bit.SystemRecordPage = 0;            // 最新数据存放在�?页中，编�?   
                        }
                        
                        me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 1;       // 置位运行记录所在页已找到标志位
                        
                        status = Q_TRAN(&AO_EEPROM_SearchRecord);                           // 跳转至寻找最新运行记录状态，再次读取运行记录
                    }
                }
            }
            else                // 运行记录所在页已找�?me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag == 1
            {   
                // 其次查找最新数�?        
                // 最新运行记录有3种可能，列举如下
                // 可能�?：新编号运行时间�?xFFFFFFFF
                // 可能�?：已遍历全部运行记录，则最后一个运行记录即最新记�?
                
                if (me->Variable.SystemRecordCircleNumber == 0)                 // 读取编号0数值完成，继续读编�?，两个数据以上才能比�?
                {
                    me->Variable.SystemRecordCircleNumber++;                    // 读取编号1数据
                    
                    status = Q_TRAN(&AO_EEPROM_SearchRecord);                   // 跳转至寻找最新运行记录状态，再次读取运行记录                   
                }
                else                                                            // 编号1-63数据读取完成，与上一次数据进行比�?
                {                    
					if (g_SystemRecord.TotalTime == 0xFFFFFFFF)
                    {
                        // 若新读取数据�?xFFFFFFFF，则上一组数据为最新数�?
                        
                        // Take care：此处SystemRecordCircleNumber需做减1处理，重新指向上一个数据位�?
                        me->Variable.SystemRecordCircleNumber--;                
                        me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 1;   // 置位运行记录已找到标志位
                        
                        status = Q_TRAN(&AO_EEPROM_ReadRecord);                     // 跳转至读取运行记录状态，读取运行记录
                    }
                    else
                    {
                        // 若新读取的数值不�?xFFFFFFFF，则继续读取下一个数�?
                        me->Variable.SystemRecordCircleNumber++;                    
                        if (me->Variable.SystemRecordCircleNumber < RECORD_CIRCLE_NUMBER)       // 不要读取过界�?
                        {
                            status = Q_TRAN(&AO_EEPROM_SearchRecord);                   // 跳转至寻找最新运行记录状态，再次读取运行记录
                        }
                        else                            
                        {
                            // 若最后一个数据仍不为0xFFFFFFFF，则最后一个数据为最新数�?
                            // Take care�?运行记录编号为循环长度减1，因为编号从0开始编�?
                            me->Variable.SystemRecordCircleNumber = RECORD_CIRCLE_NUMBER - 1;
                            
                            me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 1;   // 置位运行记录已找到标志位
                             
                            status = Q_TRAN(&AO_EEPROM_ReadRecord);                     // 跳转至读取运行记录状态，读取运行记录
                        }
                    }
                }
            }
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // 父状态为AO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 5.读取行车记录 ------------------------------------
QState AO_EEPROM_ReadRecord(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(5);
#endif        
            
            // 读取EEPROM运行记录一�?
            // 读取的运行记录直接存入运行记录数据结构体
            // Take care：此指令做LRC校验
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD], \
                                EEPROM_Block_addr[EEPROM_BLOCK_RECORD] 
                                + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage 
                                + 2 * RECORD_CIRCLE_LENGTH * me->Variable.SystemRecordCircleNumber, \
                                EEPROM_Block_length[EEPROM_BLOCK_RECORD], 1);
                
            QActive_armX((QActive *)me, 0U, 500U);                  // 开启定时器0�?00ms 
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROM读取成功,且校验正�?
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            // 运行记录读取完成，将运行记录传递给相关模块
            g_AO_BMS.Output.BatteryCapacity = g_SystemRecord.BatteryCapacity;
            g_AO_BMS.Output.SOC = g_SystemRecord.SOC;
//            if(g_AO_BMS .Output.SOC <= 120)
//            {
//                if(g_AO_BMS.Output.SOC == 0)
//                {
//                    g_AO_BMS.Output.StartSOC = 1;
//                }
//                else
//                {
//                    g_AO_BMS.Output.StartSOC = g_SystemRecord.SOC;
//                }
//            }
            // 将运行记录中运行时间作为系统启动时间，仅供读取，当前时间减去启动时间，即系统本次已运行时�?
            // 此数据可作为OEM测试时间限制�?
            g_SystemState.Output.SystemStartTime = g_SystemRecord.TotalTime;

            if (me->Variable.Variable_bits.bit.LastRecordLRCFail == 0)      // 最后一组运行记录LRC校验失败标志位为0，数据正�?
            {
                status = Q_TRAN(&AO_EEPROM_ReadParameter);                  // EEPROM信息读取完毕,跳转至AO_EEPROM_ReadParameter状�?
            }
            else
            {       
                // 最后一次运行记录校验错误，且已获得上一次正确的运行记录，需要修复错误区域，否则会造成以后数据混乱
                // 跳转至修复运行记录状态，并清除运行记录错误标志位
                me->Variable.Variable_bits.bit.LastRecordLRCFail = 0;
                
                status = Q_TRAN(&AO_EEPROM_FixRecord);              // 跳转至修复运行记录状�?
            }
        }
        break;
        
        case EEPROM_LRC_FAIL_SIG:                                   // EEPROM读取发生LRC校验错误
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            // For test：用于记录启动时发生了多少次运行记录LRC校验错误
            me->Variable.RecordLRCFail++;                           // 运行记录LRC校验错误次数
                
            // 保障代码
        //    status = Q_HANDLED();
            
            // 置位最后一次运行记录LRC校验错误标志�?
            me->Variable.Variable_bits.bit.LastRecordLRCFail = 1;
            
            if (me->Variable.SystemRecordCircleNumber > 0)          // 系统运行记录编号不为0，即查找上一次数�?
            {
                me->Variable.SystemRecordCircleNumber--;
                
            //    me->Variable.SystemRecordRetryCount++;            // 系统运行记录重新读取尝试++
                
                status = Q_TRAN(&AO_EEPROM_ReadRecord);             // 跳转至读取运行记录状态，读取运行记录               
            }
            else                        // 已查找至最早的数据，仍错误，则清零系统运行记录，运行记录彻底废�?
            {       
                // 清除系统运行记录,并写�?
                memset(&g_SystemRecord, 0, sizeof(g_SystemRecord));         // 设备运行记录
                
                // 运行记录无法读取时，默认设置电池电量�?,第一次充电充满会自动重置电量
                g_AO_BMS.Output.BatteryCapacity = 0;
                g_AO_BMS.Output.SOC = 0;   
                
                // 系统启动时间也清�?
                g_SystemState.Output.SystemStartTime = 0;
                
                // 跳转至修复运行记录状态，并清除运行记录错误标志位
                me->Variable.Variable_bits.bit.LastRecordLRCFail = 0;
                
                status = Q_TRAN(&AO_EEPROM_FixRecord);              // 跳转至修复运行记录状�?
            }
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // 父状态为AO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ---------------------------- 51.最后一次运行记录错误，修复�?---------------------------------
QState AO_EEPROM_FixRecord(AO_EEPROM * const me) {
    
    QState status;
    
    u16 l_cnt = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(51);
#endif
                  
            // 擦除Record，使用编�?�?两页，页分配见AO_EEPROM.c�?0�?
            // Take care: 擦除flashEEPROM为即时指令，工作时会造成CPU停止工作，删�?页flash，约40ms
            for (l_cnt = 0; l_cnt < 2; l_cnt++)
            {
                flash_EEPROM_Erase(&g_flash_EEPROM, (EEPROM_Block_addr[EEPROM_BLOCK_RECORD] + l_cnt * FLASH_EEPROM_BLOCK_LENGTH));
            }

            // 清除record，然后写入第一页，第一�?
            me->Variable.Variable_bits.bit.SystemRecordPage = 0;
            me->Variable.SystemRecordCircleNumber = 0;

            // 写入运行记录，默认写入第一页第一条，无需计算写入地址
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD], 
                                EEPROM_Block_addr[EEPROM_BLOCK_RECORD], \
                                EEPROM_Block_length[EEPROM_BLOCK_RECORD], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_RECORD]);

            QActive_armX((QActive *)me, 0U, 500U);                  // 开启定时器0�?00ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:                
            // EEPROM擦除成功
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
    
            // 将运行记录中运行时间作为系统启动时间，仅供读取，当前时间减去启动时间，即系统本次已运行时�?
        //    g_SystemState.Output.SystemStartTime = g_SystemRecord.TotalTime;            
            
            status = Q_TRAN(&AO_EEPROM_ReadParameter);              // 跳转AO_EEPROM_ReadParameter状�?
        }
        break;      
		
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // 父状态为AO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 6.读取参数及控制器信息 ------------------------------------
QState AO_EEPROM_ReadParameter(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(6);
#endif          
            
            // 读取EEPROM参数一�?
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], \
                                EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], 1);
                    
            QActive_armX((QActive *)me, 0U, 1000U);                 // 开启定时器0�?000ms,参数可能较多，预留操作时间加�?
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROM读取成功
        {
            QActive_disarmX((QActive *)me, 0U);                     

            // 若发现系统参数的第一个（设备名称）不为正常值，则修复参数，有可能参数未正常读取或参数未初始化
            if (g_SystemParameter.System.Information.DeviceName[0] == 0x0000
                || g_SystemParameter.System.Information.DeviceName[0] == 0xFFFF)
            {
                // Lv0: 参数LRC校验通过，但是参数内容错误，可能未被初始化，例如全为0
                Protect_SetFaultCodeLv0(&g_Protect, EEPROM_PARAMETER_WRONG);        //设置LV0故障函数
                
                status = Q_TRAN(&AO_EEPROM_FixParameter);           // 参数未被初始化，跳转到修复参数及控制器信息
            }
            else if (g_SystemParameter.System.Information.SoftwareVersion != CONST_SOFTWARE_VERSION)//判断软件版本
			{
				status = Q_TRAN(&AO_EEPROM_UpdateInformation);           // 跳转至更新参数状
			}
			else
            {
                // 系统参数给定，并初始化部分模   根据EEPROM参数，配置系统模块参数
                System_ParameterSet();
                
                // 向SH36730x0模块发送启动允许事件，允许配置SH36730x0芯片
                // Take care：必须从EEPROM读取参数成功后，再发送BQ769x0活动对象START事件，触发SH36730x0芯片参数配置操作
                //QACTIVE_POST((QActive *)&g_AO_SH36730x0, START_SIG, 0);   
                	
                status = Q_TRAN(&AO_EEPROM_ReadFaultHistory);       // 跳转AO_EEPROM_ReadFaultHistory 
            }
        }
        break;
        
        case EEPROM_LRC_FAIL_SIG:                                   // EEPROM读取发生LRC校验错误
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?   
            
            // Lv0: 参数LRC校验错误
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_PARAMETER_LRC_FAIL);
            
            status = Q_TRAN(&AO_EEPROM_FixParameter);               // 参数LRC校验失败，跳转至修复参数状�? 
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // 父状态为AO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 61.修复参数及控制器信息 ------------------------------------
QState AO_EEPROM_FixParameter(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(61);
#endif
            
            // 系统参数初始化为默认�?
            memset(&g_SystemParameter, 0, sizeof(g_SystemParameter));       // 先将参数全部清零
            SystemParameter_Init();                                         // 再写入默认参�?
            
            // 擦除参数所在区域flash，此操作会造成mcu 停止工作�?0ms
            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER]);
                        
            // 写入EEPROM参数一�?
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], \
                                EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);
                    
            QActive_armX((QActive *)me, 0U, 1000U);     // 开启定时器0�?000ms，此操作可能耗时较长
                                
            status = Q_HANDLED();           
        }
        break;

        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROM写入成功
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?

            // 系统参数给定，并初始化部分模�?
            System_ParameterSet();
            
            // 向BQ769x0模块发送启动允许事件，允许配置BQ769x0芯片
            // Take care：必须从EEPROM读取参数成功后，再发送BQ769x0活动对象START事件，触发BQ769x0芯片参数配置操作
        //    QACTIVE_POST((QActive *)&SH36730x0, START_SIG, 0);   
            
            status = Q_TRAN(&AO_EEPROM_ReadFaultHistory);           // 跳转AO_EEPROM_ReadFaultHistory状�?      
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // 父状态为AO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 63.更新参数及控制器信息，软件版本更�?------------------------------------
QState AO_EEPROM_UpdateInformation(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(63);
#endif
            
            // 软件版本更新后，自动清除故障记录
            // 注意：flash写入前需擦除页，此操作为即时操作，但会导致cpu暂停工作
            // 电机控制软件当PWM使能时，严禁擦除flash，会导致PWM暂停更新
            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY]);             
            
            // 更新参数区域，软件版本更�?
            g_SystemParameter.System.Information.SoftwareVersion = CONST_SOFTWARE_VERSION;
			
            
            
            // 擦除参数所在区域flash，此操作会造成mcu 停止工作�?0ms
            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER]);
                        
            // 写入EEPROM参数一�?
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], \
                                EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);
                    
            QActive_armX((QActive *)me, 0U, 1000U);     // 开启定时器0�?000ms，此操作可能耗时较长
                                
            status = Q_HANDLED();           
        }
        break;

        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROM写入成功
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?

            // 系统参数给定，并初始化部分模�?
            System_ParameterSet();
            
            // 向BQ769x0模块发送启动允许事件，允许配置BQ769x0芯片
            // Take care：必须从EEPROM读取参数成功后，再发送BQ769x0活动对象START事件，触发BQ769x0芯片参数配置操作
        //    QACTIVE_POST((QActive *)&SH36730x0, START_SIG, 0);   
            
            status = Q_TRAN(&AO_EEPROM_ReadFaultHistory);           // 跳转AO_EEPROM_ReadFaultHistory状�?      
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // 父状态为AO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 62.读取故障历史记录 ------------------------------------
QState AO_EEPROM_ReadFaultHistory(AO_EEPROM * const me) {
    
    QState status;
    
    s16 i = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(62);
#endif
            
            // 读取故障历史记录一�?
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_FAULT_HISTORY], \
                                EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY], 
                                EEPROM_Block_length[EEPROM_BLOCK_FAULT_HISTORY], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_FAULT_HISTORY]);
                    
            QActive_armX((QActive *)me, 0U, 1000U);                 // 开启定时器0�?000ms,此操作可能耗时较长
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROM读取成功
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时器0

            // To be delete
            // Take care�?旧版软体�?.2.2.0002及之前）更新至新版时，需处理0xFF数据，清除之
            // 若故障代码为0xFF，即不存在，则清除此条记�?
            for (i = FAULT_HISTORY_LENGTH; i >= 0; i--)
            {
                if (g_SystemFaultHistory.FaultHistory[i].bit.FaultCode == 0xFF)
                {
                    g_SystemFaultHistory.FaultHistory[i].all = 0;
                }
            }
            
            for (i = FAULT_COUNT_LENGTH; i >= 0; i--)
            {
                if (g_SystemFaultHistory.FaultCount[i].bit.FaultCode == 0xFF)
                {
                    g_SystemFaultHistory.FaultCount[i].all = 0;
                }
            }   
			
            // 向BQ769x0模块发送启动允许事件，允许配置BQ769x0芯片
            // Take care：必须从EEPROM读取参数成功后，再发送BQ769x0活动对象START事件，触发BQ769x0芯片参数配置操作


			QACTIVE_POST((QActive *)&g_AO_SH36730x0, START_SIG, 0);   //启动SH36730  配置信号
	
	
            
            status = Q_TRAN(&AO_EEPROM_Normal);                     // 跳转至AO_EEPROM_Normal状�?       
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // 父状态为AO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 7.EEPROM正常处理状态，可接收写入读取指�?------------------------------------
QState AO_EEPROM_Normal(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(7);
#endif
            
            QActive_armX((QActive *)me, 2U, 1000U);                 // 开启定时器2�?000ms，用于定时刷�?
                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_Idle);                       // 默认跳转至AO_EEPROM_Idle
        }
        break;
        
        case Q_EXIT_SIG:
        {   
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            QActive_disarmX((QActive *)me, 2U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;      
        
        case Q_TIMEOUT_SIG:                                         // 定时�?被触发，超时
        {
            // 正常处理EEPROM信息发生超时事件时，统一由AO_EEPROM_Normal状态处�?
            
            // 发送FAULT_SIG，设置全局故障标志，跳转至故障状�?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_OVERTIME);
            
            status = Q_TRAN(&AO_EEPROM_Fault);                      // 跳转至EEPROM故障状�?
        }
        break;      

        case Q_TIMEOUT2_SIG:                                        // 定时�?被触发，超时
        {
            QActive_armX((QActive *)me, 2U, 1000U);                 // 开启定时器2�?000ms，用于定时刷�?
            
            // 1s定时更新函数，定时刷新运行记�?
            SystemRecord_Update(me);
            
            status = Q_HANDLED(); 
        }
        break;
        
        case EEPROM_FAIL_SIG:
        {
            // EEPROM操作失败事件，统一由AO_EEPROM_Normal状态处�?
            
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            // 发送FAULT_SIG，设置全局故障标志，跳转至故障状�?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_FAIL);
            
            status = Q_TRAN(&AO_EEPROM_Fault);                      // 读取校验区信息超时。跳转至EEPROM故障状�?         
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                            // 父状态为top状�?
        }
        break;
    }
    
    return status;
}

// ----------------------------- 8.EEPROM空闲状态，可接收写入读取指�?------------------------------------
QState AO_EEPROM_Idle(AO_EEPROM * const me) {
    
    QState status;
    
    u16 i = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(8);
#endif
            
            // 此处将队列缓存中指令予以再次发�?
            if (me->Variable.EEPROMWriteBlockPointer > 0)           // 发送指令缓存队列中有数�?
            {
                // 发送EEPROM写入指令，发送最早一个指令，队列先进先出
                QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, me->Variable.EEPROMWriteBlockWait[0]);
                
                // 队列缓存长度--，队列缓存向前推进一位，清除最早一个指令，后续指令向前移动一�?
                me->Variable.EEPROMWriteBlockPointer--;         
                for (i = 0; i < me->Variable.EEPROMWriteBlockPointer; i++)
                {
                    me->Variable.EEPROMWriteBlockWait[i] = me->Variable.EEPROMWriteBlockWait[i + 1];                    
                }
				
				me->Variable.EEPROMWriteBlockWait[me->Variable.EEPROMWriteBlockPointer] = 0;
            }
                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_WRITE_SIG:                                      // EEPROM写指�?
        {
            me->Variable.EEPROMWriteBlock = (u16)Q_PAR(me);         // 读取写入指令参数，为待写入区�?
        
            status = Q_TRAN(&AO_EEPROM_Write);                      // 跳转至EEPROM写入状�?   
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Normal);                    // 父状态为AO_EEPROM_Normal状�?
        }
        break;
    }
    
    return status;
}

s32 t_EEPROM[5];

// ------------------------- 9.EEPROM写入状态，可接收写入读取指�?--------------------------
QState AO_EEPROM_Write(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(9);
#endif
            
            switch (me->Variable.EEPROMWriteBlock)
            {
                case EEPROM_BLOCK_RECORD:
                {
                    // 运行记录采用两页flash交替写入的方式，需做相应处�?
                    me->Variable.SystemRecordCircleNumber++;    // 单页中已写入运行记录数量++，决定flash中写入的位置     
                    if (me->Variable.SystemRecordCircleNumber >= RECORD_CIRCLE_NUMBER)      // 不准超过单页限制长度
                    {       
                        // 当前页写完后，更换至另一页继续写�?
                        if (me->Variable.Variable_bits.bit.SystemRecordPage == 0)
                        {
                            me->Variable.Variable_bits.bit.SystemRecordPage = 1;
                        }
                        else
                        {
                            me->Variable.Variable_bits.bit.SystemRecordPage = 0;
                        }
                            
                        // 单页中已写入运行记录数量归零
                        me->Variable.SystemRecordCircleNumber = 0;
                        
                        // Take care: 若需写入的页起始地址数据不为0xFFFFFFFF，即残留有以前的运行记录，则需要先擦除整页
                        // 仪表等时序要求不高的产品，允许运行中擦除flash,系统暂停�?0ms
                        // 电机控制软件当PWM使能时，严禁擦除flash，会导致PWM暂停更新
                        if (*(__IO uint32_t*)(EEPROM_Block_addr[EEPROM_BLOCK_RECORD] + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage) != 0xFFFFFFFF)
                        {
                            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_RECORD] + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage);
                        }
                    }                   
                    
                    // 写入运行记录到指定位置，一页写入完成后才会擦除
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD],                                 \
                                        EEPROM_Block_addr[EEPROM_BLOCK_RECORD]                                                  \
                                        + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage           \
                                        + 2 * RECORD_CIRCLE_LENGTH * me->Variable.SystemRecordCircleNumber,                     \
                                        EEPROM_Block_length[EEPROM_BLOCK_RECORD], EEPROM_Block_LRCEnable[EEPROM_BLOCK_RECORD]);                         
                }
				break;
                
                case EEPROM_BLOCK_PARAMETER:        // 参数写入，仅允许在空闲或参数调试时写�?
                {
                    // 注意：flash写入前需擦除页，此操作为即时操作，但会导致cpu暂停工作
                    // 电机控制软件当PWM使能时，严禁擦除flash，会导致PWM暂停更新
                    flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER]);     
                    
                    // 写入EEPROM一�?
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER],      \
                                        EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER],                      \
                                        EEPROM_Block_length[EEPROM_BLOCK_PARAMETER],                    \
                                        EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);    
                  
                }
                break;
                
                case EEPROM_BLOCK_FAULT_HISTORY:
                {
                    
                    // 注意：flash写入前需擦除页，此操作为即时操作，但会导致cpu暂停工作
                    // 电机控制软件当PWM使能时，严禁擦除flash，会导致PWM暂停更新
                    flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY]);     
                    
                    // 写入EEPROM一�?
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_FAULT_HISTORY],      \
                                        EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY],                      \
                                        EEPROM_Block_length[EEPROM_BLOCK_FAULT_HISTORY],                    \
                                        EEPROM_Block_LRCEnable[EEPROM_BLOCK_FAULT_HISTORY]);                        
                }
                break;
            }       
    
            // 统一开启一�?s定时�?
            QActive_armX((QActive *)me, 0U, 5000U);                 // 开启定时器0�?000ms
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {   
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_WRITE_SIG:                                      // EEPROM写指�?
        {
			
            // 当执行写入操作时收到写入指令，存入指令缓存队列中
            if (me->Variable.EEPROMWriteBlockPointer < EEPROM_WRITE_COMMAND_LENGTH)
            {
                me->Variable.EEPROMWriteBlockWait[me->Variable.EEPROMWriteBlockPointer] = (u16)Q_PAR(me);           // 读取写入指令参数，为待写入区�?
            
                me->Variable.EEPROMWriteBlockPointer++;
				
				t_EEPROM[0]++;
				
				if (t_EEPROM[1] < me->Variable.EEPROMWriteBlockPointer)
				{
					t_EEPROM[1] = me->Variable.EEPROMWriteBlockPointer;
				}
            }
            else
            {
                // EEPROM希尔指令队列缓存溢出，仅报错，不处理，此问题不属于EEPROM活动对象错误，由外部系统过于频繁请求写入造成
                Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_OVER_FLOW);
            }
            
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?          
            
            // 参数写入完成后，置位芯片重启请求标志�?当下次进入BMS_Idle空闲状态时，触发mcu重启
            // To be update�?此处功能应该无效了，待确�?
            if (me->Variable.EEPROMWriteBlock == EEPROM_BLOCK_PARAMETER)
            {
                g_AO_BMS.State.ChipResetAsk = 1;
            }
			
			status = Q_TRAN(&AO_EEPROM_Idle);                       // EEPROM信息读取完毕,跳转至AO_EEPROM_Idle状�?
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Normal);                    // 父状态为AO_EEPROM_Normal状�?
        }
        break;
    }
    
    return status;
}

// ------------------------- 100.EEPROM故障，仅处理紧急事�?---------------------------
QState AO_EEPROM_Fault(AO_EEPROM * const me) {
    
    QState status;
    
    u16 l_cnt = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(100);
#endif
   
            // 置位LV0级别故障，故障内容为EEPROM读写失败
            // 此操作为冗余操作，若进入此状态前未设置具体故障信息，则统一报EEPROM读写失败
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_EEPROM_FAILURE);
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_WRITE_SIG:                                      // EEPROM写指�?
        {
            // EEPROM故障状态时，仍允许接收部分写入指令
            switch ((u16)Q_PAR(me))
            {
                case EEPROM_BLOCK_CHECK:                            // 写入校验数据
                {   
                    // 擦除Check信息对应flash，即时指令，会造成CPU暂停�?0ms
                    flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_CHECK]);
                    
                    // 写入校验数据
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_CHECK], EEPROM_Block_addr[EEPROM_BLOCK_CHECK], \
                                        EEPROM_Block_length[EEPROM_BLOCK_CHECK], EEPROM_Block_LRCEnable[EEPROM_BLOCK_CHECK]);                   
                }
                break;
                
                case EEPROM_BLOCK_PARAMETER:                        // 写入系统参数
                {
                    // 注意：flash写入前需擦除页，此操作为即时操作，但会导致cpu暂停工作
                    flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER]);     
                    
                    // 写入系统参数
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], 
                                        EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], \
                                        EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);                               
                }
                break;
                
                case EEPROM_BLOCK_ALL:                              // 擦除EEPROM全部数据
                {                       
                    // Take care: 擦除flashEEPROM为即时指令，工作时会造成CPU停止工作，删�?页flash，约100ms
                    for (l_cnt = 0; l_cnt < FLASH_EEPROM_BLOCK_NUMBER; l_cnt++)
                    {
                        flash_EEPROM_Erase(&g_flash_EEPROM, FLASH_EEPROM_START_ADDRESS + l_cnt * FLASH_EEPROM_BLOCK_LENGTH);
                    }           
                }
                break;
            }

            status = Q_HANDLED();
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                            // 父状态为top状�?
        }
        break;
    }
    
    return status;
}

// ---------------------------------- 20.EEPROM初始�?-----------------------------------------
QState AO_EEPROM_Initial(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(20);
#endif

            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_Erase);                      // 跳转AO_EEPROM_Erase状�?     
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            status = Q_HANDLED();
        }
        break;      
        
        case EEPROM_FAIL_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                     // 关闭定时�?
            
            // 发送FAULT_SIG，设置全局故障标志，跳转至故障状�?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_FAIL);
            
            status = Q_TRAN(&AO_EEPROM_Fault);                      // 读取校验区信息超时。跳转至EEPROM故障状�?         
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_Fault);                      // 擦除超时，跳转至EEPROM故障状�?
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                            // 父状态为top状�?
        }
        break;
    }
    
    return status;
}

// ---------------------------------- 21.擦除EEPROM -----------------------------------------
QState AO_EEPROM_Erase(AO_EEPROM * const me) {
    
    QState status;
    
    u16 l_cnt = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(21);
#endif

            // 擦除EEPROM全部数据
            // Take care: 擦除flashEEPROM为即时指令，工作时会造成CPU停止工作
            for (l_cnt = 0; l_cnt < FLASH_EEPROM_BLOCK_NUMBER; l_cnt++)
            {
                // Take care：禁止擦除CPU ID所在区块：
            //    if ((FLASH_EEPROM_START_ADDRESS + l_cnt * FLASH_EEPROM_BLOCK_LENGTH) != 0x0800E000)   
                if ((FLASH_EEPROM_START_ADDRESS + l_cnt * FLASH_EEPROM_BLOCK_LENGTH) != EEPROM_Block_addr[EEPROM_BLOCK_CHECK])                          
                {
                    flash_EEPROM_Erase(&g_flash_EEPROM, FLASH_EEPROM_START_ADDRESS + l_cnt * FLASH_EEPROM_BLOCK_LENGTH);
                }
            }

            QActive_armX((QActive *)me, 0U, 100U);                  // 开启定时器0�?00ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_InitialRecord);              // 跳转AO_EEPROM_InitialRecord状�?     
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Initial);                   // 父状态为AO_EEPROM_Initial状�?
        }
        break;
    }
    
    return status;
}

// ---------------------------- 22.初始化运行记�?---------------------------------
QState AO_EEPROM_InitialRecord(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(22);
#endif
            
            memset(&g_SystemRecord, 0, sizeof(g_SystemRecord));         // 设备运行记录清零
            
            // 写入运行记录，默认写入第一页第一条，无需计算写入地址
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD], 
                                EEPROM_Block_addr[EEPROM_BLOCK_RECORD], \
                                EEPROM_Block_length[EEPROM_BLOCK_RECORD], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_RECORD]);
            
            QActive_armX((QActive *)me, 0U, 1000U);                     // 开启定时器0�?000ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:                                         // EEPROM擦除成功
        {
            QActive_disarmX((QActive *)me, 0U);                         // 关闭定时�?

            status = Q_TRAN(&AO_EEPROM_InitialFaultHistory);            // 跳转AO_EEPROM_InitialFaultHistory状�?       
        }
        break;          
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Initial);                       // 父状态为AO_EEPROM_Initial状�?
        }
        break;
    }
    
    return status;
}

// ---------------------------- 23.初始化故障历史记�?---------------------------------
QState AO_EEPROM_InitialFaultHistory(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(23);
#endif
            
            // 清除故障历史记录
            memset(&g_SystemFaultHistory, 0, sizeof(g_SystemFaultHistory)); 
            
            // 写入故障记录
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_FAULT_HISTORY], 
                                EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY], \
                                EEPROM_Block_length[EEPROM_BLOCK_FAULT_HISTORY], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_FAULT_HISTORY]);
            
            QActive_armX((QActive *)me, 0U, 1000U);     // 开启定时器0�?000ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:                                         // EEPROM擦除成功
        {
            QActive_disarmX((QActive *)me, 0U);                         // 关闭定时�?

            status = Q_TRAN(&AO_EEPROM_InitialParameter);               // 跳转AO_EEPROM_InitialParameter状�?      
        }
        break;          
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Initial);                       // 父状态为AO_EEPROM_Initial状�?
        }
        break;
    }
    return status;
}

// ---------------------------- 25.初始化系统参�?---------------------------------
QState AO_EEPROM_InitialParameter(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(25);
#endif
            
            // 系统参数初始化为默认�?
            memset(&g_SystemParameter, 0, sizeof(g_SystemParameter));   // 先将所有参数清�?
            SystemParameter_Init();                                     // 再赋值初始�?
            
            // 写入系统参数
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], \
                                EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);
            
            QActive_armX((QActive *)me, 0U, 1000U);                     // 开启定时器0�?000ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:                                         // EEPROM擦除成功
        {
            QActive_disarmX((QActive *)me, 0U);                         // 关闭定时�?
            
            // To be update�?此处跳转至EEPROM活动对象起始状态。同时限制若再次进入Initial状态，则报EEPROM初始化失败，并进入Fault状�?
            status = Q_TRAN(&AO_EEPROM_Fault);                          // 跳转AO_EEPROM_Fault状�?可响应校验值写入指�?  
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Initial);                       // 父状态为AO_EEPROM_Initial状�?
        }
        break;
    }
    
    return status;
}


// ------------------------------- 以下为内部调用功能函�?-----------------------------------------



// ---------------------------------- 运行记录定时更新函数，时�?s ---------------------------------
void SystemRecord_Update(AO_EEPROM * const me)
{       
    // 运行时间++
    g_SystemRecord.TotalTime++;
    
    // 电池电量等信息传递给运行记录
    g_SystemRecord.BatteryCapacity = g_AO_BMS.Output.BatteryCapacity;
    
    // 当电池充满电量变化时，更新之
    if (g_SystemRecord.BatteryFullCapacity != g_AO_BMS.Output.FullCapacity 
        && g_AO_BMS.Output.FullCapacity > 0)
    {
        g_SystemRecord.BatteryFullCapacity = g_AO_BMS.Output.FullCapacity;
    }
    
    // 更新电量百分�?
    g_SystemRecord.SOC = g_AO_BMS.Output.SOC;   
    
    // 电池循环次数增加
    if (g_AO_BMS.Output.CircleNumberAdd > 0)
    {
        g_SystemRecord.CircleNumber += g_AO_BMS.Output.CircleNumberAdd;
        g_AO_BMS.Output.CircleNumberAdd = 0;
        
        // 限制电池循环次数最大为6000次，达不到的
        if (g_SystemRecord.CircleNumber > 60000)                        //  CircleNumber:电池循环次数，单�?.1�?bit
        {
            g_SystemRecord.CircleNumber = 60000;
        }
    }
    
    // 电池补偿寿命次数增加
    if (g_AO_BMS.Output.LifeCalibrateCircleNumberAdd > 0)
    {
        g_SystemRecord.LifeCalibrateCircleNumber += g_AO_BMS.Output.LifeCalibrateCircleNumberAdd / 10;
        g_AO_BMS.Output.LifeCalibrateCircleNumberAdd = 0;    
        
        // 最大限�?000次，�?0%电量
        if (g_SystemRecord.LifeCalibrateCircleNumber > 20000)           //  LifeCalibrateCircleNumber:电池循环次数，单�?.1�?bit
        {
            g_SystemRecord.LifeCalibrateCircleNumber = 20000;
        }      
    }
    
	// 记录电池历史最高温�?
	if (g_AO_BMS.Output.BatteryTemperature[0] > g_SystemRecord.MaxBatteryTemperature)
	{
		g_SystemRecord.MaxBatteryTemperature = g_AO_BMS.Output.BatteryTemperature[0];
	}

	if (g_AO_BMS.Output.BatteryTemperature[1] > g_SystemRecord.MaxBatteryTemperature)
	{
		g_SystemRecord.MaxBatteryTemperature = g_AO_BMS.Output.BatteryTemperature[1];
	}
    // 记录BMS历史最高温�?
    if(g_AO_BMS.Output.BMSTemperatureHi > g_SystemRecord.BMSMaxTemp)
    {
        g_SystemRecord.BMSMaxTemp = g_AO_BMS.Output.BMSTemperatureHi;
    }
	
    // 默认60s写入一次运行记录，或有写入请求
    if (g_AO_BMS.State.OutputAllow == 0)        // 使能输出时禁止写入运行记�?
    {
        me->Variable.RecordWriteDelay++;    
        if (me->Variable.RecordWriteDelay >= RECORD_WRITE_TIMER
            || me->State.RecordWriteAsk > 0)            
        {   
            // 发送EEPROM写入事件,写入运行记录
            QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, EEPROM_BLOCK_RECORD);   
            
            me->Variable.RecordWriteDelay = 0;
            me->State.RecordWriteAsk = 0;
        }
    }
}


// End of AO_EEPROM.c


/* ==================================================================================

 File name:     system_module.h
 Originator:    BLJ
 Description:   系统及功能模块

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-1-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/



// --------------------------- 外部可调用函数 ------------------------------------
void System_LED_Update(void);					// LED输出更新，运行在1ms中断中
void I2C_BQ769x0_SIG_Update(void);				// BQ769x0 I2C 通信相关事件更新
void flash_EEPROM_SIG_Update(void);				// flash模拟EEPROM 相关事件更新
void SystemMonitor_Update(void);				// 系统状态变量更新函数，1s运行1次
void SystemParameter_Init(void);				// 系统参数初始化，赋值默认值
//void SystemComm_Update(void);					// 系统通信更新函数，运行在主循环中
void UsartCommand_Update(void);                 // 系统串口通讯更新函数，运行在主循环中
void Debuger_Update(void);						// 系统调试用处理函数，运行在主循环中
void System_ParameterSet(void);					// 系统模块参数根据EEPROM参数初始化
void ChargerCommand_Update(void);               // 更新充电机控制指令 

int get_temp_current_v(short t, int c);

// End of system_module.h


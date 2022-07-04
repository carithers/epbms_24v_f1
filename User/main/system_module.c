/* ==================================================================================

 File name:     system_module.c
 Originator:    BLJ
 Description:   系统及功能模块

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-1-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#include "system_core.h"   //系统核心头文件
#include "system_module.h"


u16	t_ChargeStateDelay = 0;
u16	t_LED_TEST_Delay = 0;
u16 t_LEDDelay = 0;
u16 t_FAULTDelay = 0;
// ---------------------------------- LED灯刷新函数，运行周期1ms ---------------------------------
void System_LED_Update(void)
{
#if (CONTROLLER_TARGET == BMS_EP_A5 || BMS_EP_20_REV1_1_2TEMP || BMS_EP_ALPHA)

    // 2017-09-14
    // 中力提出新方案，仅保留一盏灯，红灯。控制逻辑如下：
    // 正常状态慢闪，周期2s
    // 单体欠压：闪1下（时间小一些） 停2S 闪2下 停3S后循环
    // 单体过压：闪1下（时间小一些） 停2S 闪3下 停3S后循环
    // 电池温度过高：闪1下（时间小一些） 停2S 闪4下 停3S后循环
    // 电池温度过低：闪1下（时间小一些） 停2S 闪5下 停3S后循环
    // 其余故障都黄色快闪,周期0.4s  
    if(g_Input.Output.DIN_bits.bit.KEY > 0)
    {
        if (Protect_GetFaultCode(&g_Protect) > 0)
        {
            switch (Protect_GetFaultCode(&g_Protect))
            {
                case FAULT_BQ769_UV:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 1;
                    g_LED_Flicker.Input.LEDLength2 = 2;
                    LED_Flicker_Update(&g_LED_Flicker);	                
                }
                break;
                
                case FAULT_BQ769_OV:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 1;
                    g_LED_Flicker.Input.LEDLength2 = 3;
                    LED_Flicker_Update(&g_LED_Flicker);	                
                }
                break;    
                
                case FAULT_BQ769_SCD     :
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0); 
                    g_LED_Flicker.Input.LEDLength1 = 1;
                    g_LED_Flicker.Input.LEDLength2 = 4;
                    LED_Flicker_Update(&g_LED_Flicker);	
                }
                break;
               
                case FAULT_BQ769_OCD:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 1;
                    g_LED_Flicker.Input.LEDLength2 = 5;
                    LED_Flicker_Update(&g_LED_Flicker);	            
                }
                break;

                case FAULT_BAT_OT:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 2;
                    g_LED_Flicker.Input.LEDLength2 = 3;
                    LED_Flicker_Update(&g_LED_Flicker);	                
                }
                break;  

                case FAULT_BAT_UT:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_YELLOW_Update(&g_Output, 0);
                    g_LED_Flicker.Input.LEDLength1 = 2;
                    g_LED_Flicker.Input.LEDLength2 = 4;
                    LED_Flicker_Update(&g_LED_Flicker);	                
                }
                break;  

                default:
                {
                    Output_LED_BLUE_Update(&g_Output, 0);
                    Output_LED_GREEN_Update(&g_Output, 0);
                    t_LED_TEST_Delay++;
                    if (t_LED_TEST_Delay < 500)
                    {
                        Output_LED_YELLOW_Update(&g_Output, 0);
                    }
                    else if (t_LED_TEST_Delay < 1000)
                    {
                        Output_LED_YELLOW_Update(&g_Output, 1);					
                    }
                    else
                    {
                        t_LED_TEST_Delay = 0;
                    }	                
                }
                break;
            }
            
        }
        else
        {
    		// 目前没有通讯确认，暂时根据电流判断充放电状态，电流连续为充电电流，持续5s以上，认为进入充电状态
    		
    		if (g_AO_SH36730x0.Output.BatteryCurrent > 500)			// 充电电流大于100mA
    		{
    			if (t_ChargeStateDelay < 11000)
    			{
    				t_ChargeStateDelay++;
    			}
    		}
    		else
    		{
    			t_ChargeStateDelay = 0;
    		}
    		
    		if (t_ChargeStateDelay > 10000)
    		{
    			// 充电状态
    			// 充电时，红灯亮，充电100%，绿灯亮
    			if (g_AO_BMS.Output.SOC >= 1000)
    			{
    				Output_LED_GREEN_Update(&g_Output, 1);
    				Output_LED_BLUE_Update(&g_Output, 0);		// 蓝色暂定为红色
    				Output_LED_YELLOW_Update(&g_Output, 0);
    			}
    			else
    			{
    				Output_LED_GREEN_Update(&g_Output, 0);
    				Output_LED_BLUE_Update(&g_Output, 1);		// 蓝色暂定为红色
    				Output_LED_YELLOW_Update(&g_Output, 0);				
    			}			
    		}
    		else
    		{
    			// 放电状态
    			// 放电时，电量小于15%，红灯2s闪烁
    			// 电量小于30%，黄灯亮
    			// 电量大于30%，绿灯亮
    			if (g_AO_BMS.Output.SOC < g_SystemParameter.BMS.Warning.BDILowLimitSpeed * 10)
    			{
    				t_LEDDelay++;
    				if (t_LEDDelay < 500)
    				{
    					Output_LED_GREEN_Update(&g_Output, 0);
    					Output_LED_BLUE_Update(&g_Output, 0);		// 蓝色暂定为黄色
    					Output_LED_YELLOW_Update(&g_Output, 0);
    				}
    				else if (t_LEDDelay < 1000)
    				{
    					Output_LED_GREEN_Update(&g_Output, 0);
    					Output_LED_BLUE_Update(&g_Output, 1);		// 蓝色暂定为黄色
    					Output_LED_YELLOW_Update(&g_Output, 0);					
    				}
    				else
    				{
    					t_LEDDelay = 0;
    				}				
    			}
                else if(g_AO_BMS.Output.SOC < g_SystemParameter.BMS.Warning.BDILowLevel2Percent * 10)
                {
    				t_LEDDelay++;
    				if (t_LEDDelay < 1000)
    				{
    					Output_LED_GREEN_Update(&g_Output, 0);
    					Output_LED_BLUE_Update(&g_Output, 0);		// 蓝色暂定为黄色
    					Output_LED_YELLOW_Update(&g_Output, 0);
    				}
    				else if (t_LEDDelay < 2000)
    				{
    					Output_LED_GREEN_Update(&g_Output, 0);
    					Output_LED_BLUE_Update(&g_Output, 1);		// 蓝色暂定为黄色
    					Output_LED_YELLOW_Update(&g_Output, 0);					
    				}
    				else
    				{
    					t_LEDDelay = 0;
    				}

                }
    			else if (g_AO_BMS.Output.SOC < g_SystemParameter.BMS.Warning.BDILowLevel1Percent * 10)
    			{
    				Output_LED_GREEN_Update(&g_Output, 0);
    				Output_LED_BLUE_Update(&g_Output, 0);		// 蓝色暂定为黄色
    				Output_LED_YELLOW_Update(&g_Output, 1);				
    			}				
    			else
    			{

    				Output_LED_GREEN_Update(&g_Output, 1);
    				Output_LED_BLUE_Update(&g_Output, 0);		// 蓝色暂定为黄色
    				Output_LED_YELLOW_Update(&g_Output, 0);					
    			}
                
    		}
    	}
    }
    else
    {
    	Output_LED_GREEN_Update(&g_Output, 0);
    	Output_LED_BLUE_Update(&g_Output, 0);		// 蓝色暂定为黄色
    	Output_LED_YELLOW_Update(&g_Output, 0);	        
    }

#endif					
	
}

u16	t_I2CFailCount = 0;

// --------------------- BQ769x0 I2C通信状态机相关事件发送函数，运行在主循环中 ----------------------
void I2C_BQ769x0_SIG_Update(void)
{
	// 模块故障,发送故障事件到AO_EEPROM状态机
	if (g_I2C_SH36730x0.State.Fault_bits.all > 0)							
	{		
        g_SystemMonitor.System.Reverse2.Data[3] = g_I2C_SH36730x0.State.Fault_bits.all;
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag1 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[4]++;//记录通信失败次数及具体原因做调试用
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag2 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[5]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag3 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[6]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag4 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[7]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag5 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[8]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag6 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[9]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag7 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[10]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag8 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[11]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag9 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[12]++;
        }
        
        if (g_I2C_SH36730x0.State.Fault_bits.bit.OverTimeFlag10 > 0)
        {
            g_SystemMonitor.System.Reverse2.Data[13]++;
        }
        
		QACTIVE_POST((QActive *)&g_AO_SH36730x0, I2C_SH36730x0_FAIL_SIG, 0);//发布通讯失败信号//应答函数中超时
		
		t_I2CFailCount++;
		
		g_I2C_SH36730x0.State.Fault_bits.all = 0; //v->State.Fault_bits.bit.ByteSendAckHi = 1;//应答失败置位为1 
	}
	
	// EEPROM指令处理完成，发送EEPROM指令完成事件到AO_EEPROM活动对象
	if (g_I2C_SH36730x0.State.State_bits.bit.CommandFinish > 0)  // 读写指令正常完成，内部置位，需外部清除
	{
		if (g_I2C_SH36730x0.State.State_bits.bit.CRCCheckFail == 0)
		{
			QACTIVE_POST((QActive *)&g_AO_SH36730x0, I2C_SH36730x0_FINISH_SIG, 0);	
		}
		else
		{
			QACTIVE_POST((QActive *)&g_AO_SH36730x0, I2C_SH36730x0_FAIL_SIG, 0);	//方便处理把应答失败和校验失败统一为通信失败
			
			g_I2C_SH36730x0.State.State_bits.bit.CRCCheckFail = 0;
		}
		
		g_I2C_SH36730x0.State.State_bits.bit.CommandFinish = 0;
	}
}


// -------------------------- flash模拟EEPROM模块事件发生函数，用于产生状态机需要的事件 ------------------------
void flash_EEPROM_SIG_Update(void)
{
	// flash模拟EEPROM模块故障,发送故障事件到AO_EEPROM状态机
	if (g_flash_EEPROM.State.Fault_bits.all > 0)							
	{		
		// for test
		if (g_flash_EEPROM.State.Fault_bits.bit.FlashWriteCheckFail > 0)
		{
			// Lv1: flash写入数据检查错误
			Protect_SetFaultCodeLv0(&g_Protect, EEPROM_PARAMETER_WRONG);
		}
		
		if (g_flash_EEPROM.State.Fault_bits.bit.FlashWriteForbid > 0)
		{
			// Lv1: 待写入区域不为0xFFFF，禁止写入
			Protect_SetFaultCodeLv0(&g_Protect, FLASH_WRITE_FORBID);
		}
		
		if (g_flash_EEPROM.State.Fault_bits.bit.FlashWriteFail > 0)
		{
			// Lv1: 写入失败
			Protect_SetFaultCodeLv0(&g_Protect, FLASH_WRITE_FAIL);
		}		
		
		QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_FAIL_SIG, 0);	
		
		g_flash_EEPROM.State.Fault_bits.all = 0;
	}
	
	// EEPROM指令处理完成，发送EEPROM指令完成事件到AO_EEPROM活动对象
	if (g_flash_EEPROM.State.State_bits.bit.CommandFinish > 0)
	{
		if (g_flash_EEPROM.State.State_bits.bit.LRCCheckFail == 0)
		{
			QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_FINISH_SIG, 0);	
		}
		else
		{
			QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_LRC_FAIL_SIG, 0);	
			
			g_flash_EEPROM.State.State_bits.bit.LRCCheckFail = 0;
		}
		
		g_flash_EEPROM.State.State_bits.bit.CommandFinish = 0;
	}	
}


// ------------------------- 系统状态变量更新函数，1s运行1次 ----------------------------------
void SystemMonitor_Update(void)
{
	u16	i = 0;
	
	g_SystemMonitor.BMS.Battery.Voltage = g_AO_SH36730x0.Output.BatteryVoltage / 100;			// 电池组电压，单位0.1V
	
	g_SystemMonitor.BMS.Battery.Current = g_AO_SH36730x0.Output.BatteryCurrent / 100;			// 电池组电流，单位0.1A
	g_SystemMonitor.BMS.Battery.CurrentCali = g_AO_SH36730x0.Output.BatteryCurrent % 100;		// 电池组总电流校准，单位0.001A，范围0-0.099A


	g_SystemMonitor.BMS.Battery.Capacity = g_AO_BMS.Output.BatteryCapacity / 100;			// 电池组剩余电量，单位0.1AH
	g_SystemMonitor.BMS.Battery.CapacityCali = g_AO_BMS.Output.BatteryCapacity % 100;		// 电池组剩余电量校准，单位0.001AH，范围0-0.099AH

	g_SystemMonitor.BMS.Battery.SOC = g_AO_BMS.Output.SOC;									// 电池组剩余电量百分比
	
	// 单体最高/最低电压
	g_SystemMonitor.BMS.Battery.CellMaxVoltage = g_AO_SH36730x0.Output.SingleMaxVoltage;
	g_SystemMonitor.BMS.Battery.CellMaxVoltagePointer = g_AO_SH36730x0.Output.SingleMaxVoltagePointer;
	g_SystemMonitor.BMS.Battery.CellMinVoltage = g_AO_SH36730x0.Output.SingleMinVoltage;
	g_SystemMonitor.BMS.Battery.CellMinVoltagePointer = g_AO_SH36730x0.Output.SingleMinVoltagePointer;	
	/*
	// 电池温度
	g_SystemMonitor.BMS.Battery.Temperature[0] = g_AO_BMS.Output.BatteryTemperature[0];
	g_SystemMonitor.BMS.Battery.Temperature[1] = g_AO_BMS.Output.BatteryTemperature[1];
	g_SystemMonitor.BMS.Battery.Temperature[2] = g_AO_BMS.Output.BatteryTemperature[2];
	*/
	// 单体均衡信息
	g_SystemMonitor.BMS.Battery.CellBalanceEnable = g_AO_SH36730x0.State.BatteryBalanceEnable;		// 电池单体均衡使能功能开启
	g_SystemMonitor.BMS.Battery.CellBalancing = g_AO_SH36730x0.State.BatteryBalancing;				// 电池单体均衡中
	g_SystemMonitor.BMS.Battery.CellBalancePointer = g_AO_SH36730x0.State.BatteryBalancePointer;    // 电池正在均衡单体编号
	
	// 电池单体电压
	for (i = 0; i < 15; i++)
	{
		g_SystemMonitor.BMS.Cell.CellVoltage[i] = g_AO_SH36730x0.Output.SingleVoltage[i];
	}
//==============================================应该不用了，后期检测一遍======================================================================	
//	// AFE芯片内部温度采样
//    if (g_AO_SH36730x0.BQ769x0Register.SYS_CTRL1.bit.TEMP_SEL == 0)
//    {
      	for (i = 0; i < 3; i++)
	    {
		    g_SystemMonitor.BMS.AFE.InnerTemperature[i] = g_AO_SH36730x0.Output.InnerTemperature[i] ;
	    }	  
//    }
//    else
//    {
//    	for (i = 0; i < 3; i++)
//    	{
//    		g_SystemMonitor.BMS.AFE.InnerTemperature[i] = g_AO_BQ769x0.Output.ExternalTemperature[i] ;
//    	}	
//    }
//====================================================================================================================	
    // 电池内部温度采样,共8路
	for (i = 0; i < 4; i++)
	{
		g_SystemMonitor.BMS.AFE.BatteryTemperature[i] = g_AO_BMS.Output.BatteryTemperature[i];
	}	

	// 充放电使能标志位
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.DischargeEnable = g_AO_SH36730x0.State.DSGControl;
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.ChargeEnable = g_AO_SH36730x0.State.CHGControl;
	
	// 充放电开始实际状态,直接通过SH36730x0芯片寄存器获得
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.DischargeState = g_AO_SH36730x0.SH36730x0Register.BSTATUS.bit.DSG;//用状态寄存器BSTATUS更好一些
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.ChargeState = g_AO_SH36730x0.SH36730x0Register.BSTATUS.bit.CHG;////更改//=========================
	
	// 充放电控制IO的输出信号，仅在MCU管理接触器设计中起作用
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.DischargeIOState = g_Input.Output.DIN_bits.bit.DSG;
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.ChargeIOState = g_Input.Output.DIN_bits.bit.CHG;	
	
	// KEY输入信号
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.KEYState = g_Input.Output.DIN_bits.bit.KEY;
	
	// 额外均流触发标志位
	g_SystemMonitor.BMS.Manage.Manage_bits.bit.ExtraBalance = g_AO_BMS.State.BatteryExtraBalance;
	
	// 额外均流已持续时间，单位min
	g_SystemMonitor.BMS.Manage.ExtraBalanceLastTime = g_AO_BMS.Variable.ExtraBallanceCount;
	
	// 充电限制电流，单位0.1A
	g_SystemMonitor.BMS.Manage.ChargeCurrentLimit = g_AO_BMS.Output.ChargeCurrentLimit;
	
	// BMS管理板温度，单位0.1摄氏度
	for(i = 0; i < 3; i++)
	{
	    g_SystemMonitor.BMS.BMSTemperature.BMSTemperature[i] = g_AO_BMS.Output.BMSTemperature[i];
	}
    // 内部驱动电压，单位0.1V
    g_SystemMonitor.BMS.Manage.DriveVoltage = g_ADC.Output.DriveVoltage;

	// BMS报错信息 
	if(Protect_GetFaultCode(&g_Protect) > 0)
	{
    	g_SystemMonitor.BMS.Fault.FaultCodeLv0 = g_Protect.FaultCode.SystemLockLevel0;
    	g_SystemMonitor.BMS.Fault.FaultCodeLv1 = g_Protect.FaultCode.OutputDisableLevel1;
	}   
    else
    {
        g_SystemMonitor.BMS.Fault.FaultCodeLv0 = g_Protect.FaultCode.SystemLockLevel0;
    	g_SystemMonitor.BMS.Fault.FaultCodeLv1 = g_Protect.FaultCode.DriveLimitLevel2;
    }
//================================================================================================================	
//	g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.ShortCurrent = g_AO_SH36730x0.SH36730x0Register.FLAG1.bit.SC;//
	//g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.OverCurrent = g_AO_SH36730x0.SH36730x0Register.FLAG1.bit.SC;;//电池过流
//	g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.CellOverVoltage = g_AO_SH36730x0.SH36730x0Register.FLAG1.bit.OV;
	//g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.CellUnderVoltage = g_AO_SH36730x0.BQ769x0Register.SYS_STAT.bit.UV;

//	g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.OverTemperature = g_AO_BQ769x0.BQ769x0Register.SYS_STAT.bit.SCD;
//	g_SystemMonitor.BMS.Fault.FaultInf_bits.bit.UnderTemperature = g_AO_BQ769x0.BQ769x0Register.SYS_STAT.bit.SCD;
//===================================================================================================================	
	g_SystemMonitor.System.Reverse.Data[14] = g_SystemState.Output.System1msInterruptCount;
	g_SystemMonitor.System.Reverse.Data[15] = AO_Record[0];
    
    // For test: Reverse3显示AO_BQ769x0状态机跳转记录
    for (i = 0; i < 16; i++)
    {
        g_SystemMonitor.System.Reverse3.Data[i] = g_AO_SH36730x0.Record[i];
    }
	
}

//u16	t_DebugCANCount = 0;
//u16	t_Address = 0;
//u16	t_Data = 0;

// -------------------------- 系统调试用处理函数，运行在主循环中 ------------------------------
void Debuger_Update(void)
{
    u16 i = 0;    	
	// 串口MODBUS模块会自动处理读取写入指令
	// 但参数写入指令收到后，需外部处理将参数写入EEPROM的操作
	if (g_USART_MODBUS.Output.ReceiveBufferUpdated[1] > 0)			// 序列1：系统参数写入更新标志位
	{
		g_USART_MODBUS.Output.ReceiveBufferUpdated[1] = 0;
	
        if (g_SystemParameter.System.Test.test_Shutdown > 0)			// 若上位机发送强制电池休眠指令
        {
            g_AO_SH36730x0.State.EnterSHIPModeAsk = 1;	// 请求进入SHIP低功耗模式
        }
        else if (g_SystemParameter.System.Test.test_ClearFaultHistory > 0)      // 清除故障历史记录
        {
            
            for (i = 0; i < FAULT_HISTORY_LENGTH; i++)
            {
                g_SystemFaultHistory.FaultHistory[i].all = 0;
            }
            
            for (i = 0; i < FAULT_COUNT_LENGTH; i++)
            {
                g_SystemFaultHistory.FaultCount[i].all = 0;
            }  
            
            // 发送故障记录写入事件
            QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, EEPROM_BLOCK_FAULT_HISTORY);
        }
        else if(g_SystemParameter.System.Test.test_ClearSOC > 0)
        {
            g_SystemParameter.System.Test.test_ClearSOC = 0;

            g_SystemRecord.BatteryCapacity = (g_AO_BMS.Parameter.BatteryDesignCapacity *3) / 10;  
//            g_SystemRecord.SOC = 300;        
            g_AO_BMS.Output.BatteryCapacity = g_SystemRecord.BatteryCapacity;
//            g_AO_BMS.Output.SOC = g_SystemRecord.SOC;   

            g_AO_EEPROM.State.RecordWriteAsk = 1;           // Ask  to write record to EEPROM.
        }
        else
        { 
            // 此参数不存储，仅当次有效
            g_AO_SH36730x0.State.test_ForceBatteryBalancePointer = g_SystemParameter.System.Test.test_BalancePointer;
            g_SystemParameter.System.Test.test_BalancePointer = 0;

            g_SystemState.Variable.ParameterWriteDelay = 0;
            g_SystemState.Variable.ParameterWriteAsk = 1;
            
            // 参数更新后，自动初始化一遍
            System_ParameterSet();
        }
	}
}

// ----------------------------- 系统参数初始化，赋值为默认参数 ------------------------------
void SystemParameter_Init(void)
{

#if (CONTROLLER_TARGET == BMS_EP_A5 || CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_ALPHA)						// 设备选择：中力BMS，A2/3版
	
    // 磷酸铁锂
	g_SystemParameter.System.Information.DeviceName[0] = 'E';
	g_SystemParameter.System.Information.DeviceName[1] = 'P';
	g_SystemParameter.System.Information.SerialNumber1 = 12345;
	g_SystemParameter.System.Information.SerialNumber2 = 54321;
	g_SystemParameter.System.Information.ManufactureData = 1824;			// 2018年第44周
	g_SystemParameter.System.Information.HardwareVersion = 600;
	g_SystemParameter.System.Information.SoftwareVersion = CONST_SOFTWARE_VERSION;				// 软件版本：
	g_SystemParameter.System.Information.PrototalVersion = 0;
    g_SystemParameter.System.Information.ParameterVersion = CONST_PARAMETER_VERSION;
    g_SystemParameter.System.Test.test_ChargeReqVol = 2;
	// 电流传感器放大比例和偏置，需校准
	// CC值单位为8.44uV，采样电阻0.001欧姆，所以CC = 1对应电流为1 * 8.44 / 0.001 = 8440	
	// 电流传感器放大比例，输出单位mA，基值1000	
	// 默认1m欧姆采样电阻，即100A时对应电压100mV

    g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent = 4000;		// 1000A  //400A
	g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage = 1933;		// 500mV  //200mv
	g_SystemParameter.System.Calibration.BMSIdleCurrent = 15;				// BMS自身损耗电流，单位mA，实测得到，补偿电量计算
    g_SystemParameter.System.Calibration.InnerDriveSensorGain = 251;        // 内部驱动电压传感器放大比例，单位0.1kOhm
    g_SystemParameter.System.Calibration.InnerDriveSensorOffset = 0;        // 内部驱动电压传感器偏置，单位0.1V  

		
	g_SystemParameter.BMS.Battery.AFESelect = BQ76930;

    g_SystemParameter.BMS.Battery.SeriesNumber = 8;                     // 电池总串数为8串   
    g_SystemParameter.BMS.BatteryType.BatteryType = 1;                   //电池类型，0：三元 1：铁锂
    g_SystemParameter.BMS.BatteryType.BatteryTypeReal = 1;    

    #if (BATTERY_VERSION == 121)
    g_SystemParameter.BMS.Battery.FullCapacity = 350;						// 电池完全充电电量，单位0.1AH
    g_SystemParameter.BMS.Battery.DesignCapacity = 350;						// 电池设计使用电量，单位0.1AH
    g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 380;
    g_SystemParameter.BMS.Battery.OverCutDownCurrent = 150;
    g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 15;					// 充电最大允许电流，15A，对应20AH电池	
    g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode = 2;		// 使能电池温度检测

    g_SystemParameter.BMS.Output.AutoCutoffCurrent = 600;					// 输出自动切断电流，600mA
    
    g_SystemParameter.BMS.Charge.ChargeOverCurrent = 22;                    // 充电过流电流20A        
    #else       
    g_SystemParameter.BMS.Battery.FullCapacity = 460;					// 电池完全充电电量，单位0.1AH
    g_SystemParameter.BMS.Battery.DesignCapacity = 450;						// 电池设计使用电量，单位0.1AH
    g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 380;
    g_SystemParameter.BMS.Battery.OverCutDownCurrent = 150;
    g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 30;					// 充电最大允许电流，15A，对应20AH电池
    g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode = 2;		// 使能电池温度检测
    
    g_SystemParameter.BMS.Output.AutoCutoffCurrent = 600;					// 输出自动切断电流，600mA

    g_SystemParameter.BMS.Charge.ChargeOverCurrent = 35;                    // 充电过流电流20A                
    #endif
    
    g_SystemParameter.BMS.Battery.CellOverVoltage = 3700;
    g_SystemParameter.BMS.Battery.CellUnderVoltage = 2200;
    g_SystemParameter.BMS.Battery.ShortCutDownDelay = 500;            //50 100 200 500us
    g_SystemParameter.BMS.Battery.OverCutDownDelay = 700;
    g_SystemParameter.BMS.Battery.CellChargeStopVoltage = 3600;             // 单体充电至3.6V截止
    g_SystemParameter.BMS.Battery.ChargeStopDelay = 60;                     // 电池充电完成延时，单位1s
   // g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode = 2;          // 使能电池温度检测
        
    g_SystemParameter.BMS.Output.AutoCutoffDelay = 900;                     // 输出自动切断延时，900s
    g_SystemParameter.BMS.Output.SleepDelay = 600;                          // BMS进入低功耗状态前延时，单位s
    g_SystemParameter.BMS.Output.SleepDelayLong = 0;                       // BMS进入低功耗状态前长时间延时，单位h，与SleepDelay累加
    g_SystemParameter.BMS.Output.OutputDelay = 500;                        // 开关按下1000ms后启动输出        
    
    g_SystemParameter.BMS.Charge.ChargeVoltageStep1 = 3400;                 // 充电电流阶段1，单体最高电压大于此电压，则充电电流由1C变为0.5C，单位1mV
    g_SystemParameter.BMS.Charge.ChargeVoltageStep2 = 3500;                 // 充电电流阶段2，单体最高电压大于此电压，则充电电流由0.5C变为0.25C，单位1mV 
    g_SystemParameter.BMS.Charge.ChargeVoltageStep3 = 3540;                 // 充电电流阶段3，单体最高电压大于此电压，则充电电流由0.25C变为0.125C，单位1mV   
    g_SystemParameter.BMS.Charge.ChargeForceStopVoltage = 3660;             // 充电强制结束电压，单位mV
    g_SystemParameter.BMS.Charge.ChargeFinishMinVoltage = 3500;           // 充电完成最小允许电压，单位mV
    g_SystemParameter.BMS.Charge.ChargeOverCurrentDelay = 30;               // 充电过流延时30s 

    g_SystemParameter.BMS.Charge2.ChargeLimitTemperature1 = 60;
    g_SystemParameter.BMS.Charge2.ChargeLimitTemperature2 = 45;
    g_SystemParameter.BMS.Charge2.ChargeLimitTemperature3 = 10;
    g_SystemParameter.BMS.Charge2.ChargeLimitTemperature4 = -2;
    
   // g_SystemParameter.BMS.ChargeGB.BalanceVoltage = 3100; 
    
    g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage = 2800;       // 强制结束放电电压
    g_SystemParameter.BMS.Discharge.DischargeStopVoltage = 3160;            // 放电截止电压设定为3.0V

     
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 = 2700;               // 校准电压1，对应电量1%
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 = 2550;       // 校准电压1，最低限制值
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 = 2800;               // 校准电压2，对应电量3.3%
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 = 2650;       // 校准电压2，最低限制值
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 = 3000;               // 校准电压3，对应电量10%
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 = 2750;       // 校准电压3，最低限制值
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 = 3100;               // 校准电压4，对应电量20%
    g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 = 2800;       // 校准电压4，最低限制值   

    g_SystemParameter.BMS.BatteryType.BatteryType = 1;                      // 电池类型，0：三元，1：磷酸铁锂
    g_SystemParameter.BMS.BatteryType.BatteryTypeReal = 1;


    g_SystemParameter.BMS.Fan.FanEnableTemperature = 0;                     // 默认不开启低温禁止充电功能
    g_SystemParameter.BMS.Fan.FanCurrent = 25;                              // 继电器工作电流25mA
    g_SystemParameter.BMS.Fan.FanMode = 2;
    g_SystemParameter.BMS.Warning.BDILowLevel1Percent = 30;					// 电池电量低百分比，单位%
    g_SystemParameter.BMS.Warning.BDILowLevel2Percent = 15;					// 电池电量严重过低百分比，单位%
    g_SystemParameter.BMS.Warning.BDILowLimitLift = 15;						// 电池电量低百分比，限制举升，单位%
    g_SystemParameter.BMS.Warning.BDILowLimitSpeed = 7;						// 电池电量低百分比，限制车速，单位%
    g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable = 0;			// 默认使能BMS管理板温度检测
    g_SystemParameter.BMS.Warning.BMSOverTemperature = 95;
    g_SystemParameter.BMS.Warning.LowSOCBeep = 17;
   // g_SystemParameter.BMS.Protect.CellSoftOverVoltage = 3680;               // 单体软件过压电压，单位1mV
   // g_SystemParameter.BMS.Protect.CellSoftOverVoltageDelay = 10;             // 单体软件过压延时，单位1s
    g_SystemParameter.BMS.Protect.CellHardwareOverVoltageDelay = 1;         // 单体硬件过压延时，单位1s
    g_SystemParameter.BMS.Protect.CellUnderVoltageDelay = 15;                // 单体欠压延时，单位s
   // g_SystemParameter.BMS.Protect.ShortCurrentTimeFactor = 50;

   	g_SystemParameter.BMS.Battery.OverTemperature = 65;
	g_SystemParameter.BMS.Battery.UnderTemperature = -25; 
    g_SystemParameter.BMS.Output.SleepDelay = 600;							// BMS进入低功耗状态前延时，单位s
    g_SystemParameter.BMS.Output.SleepDelayLong = 0;                        // BMS进入低功耗状态前长时间延时，单位h，与SleepDelay累加
    
    g_SystemParameter.BMS.Discharge.dsg_cc_low_k = 90;
    g_SystemParameter.BMS.Discharge.dsg_tmp_low_k = 10;
    
    g_SystemParameter.BMS.Contactor.ContactorBaseFrequency = 20;			// 设置PWM频率，单位Hz
	g_SystemParameter.BMS.Contactor.ContactorFullPercentTime = 2;		// 接触器启动时全占空比持续时间，单位ms
	g_SystemParameter.BMS.Contactor.ContactorLongLastPercent = 150;			// 接触器长时间工作时占空比，单位0.1%
    
#elif (CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)

	// 200系列BMS采用外部分流器，外部接触器控制正极通断设计，适用于60AH左右锂电池
	g_SystemParameter.System.Information.DeviceName[0] = 'E';
	g_SystemParameter.System.Information.DeviceName[1] = 'P';
	g_SystemParameter.System.Information.SerialNumber1 = 12345;
	g_SystemParameter.System.Information.SerialNumber2 = 54321;
	g_SystemParameter.System.Information.ManufactureData = 1611;
	g_SystemParameter.System.Information.HardwareVersion = 100;
	g_SystemParameter.System.Information.SoftwareVersion = CONST_SOFTWARE_VERSION;				// 软件版本：1.1.3
	g_SystemParameter.System.Information.PrototalVersion = 0;

	// 电流传感器放大比例和偏置，需校准
	// CC值单位为8.44uV，采样电阻0.001欧姆，所以CC = 1对应电流为1 * 8.44 / 0.001 = 8440	
	// 电流传感器放大比例，输出单位mA，基值1000	
	// 默认1m欧姆采样电阻，即100A时对应电压100mV
	g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent = 1000;		// 100A
	g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage = 750;		// 75mV
	g_SystemParameter.System.Calibration.BMSIdleCurrent = 15;				// BMS自身损耗电流，单位mA，实测得到，补偿电量计算
		
	g_SystemParameter.BMS.Battery.AFESelect = BQ76930;
	g_SystemParameter.BMS.Battery.SeriesNumber = 7;							// 电池总串数为7串
	g_SystemParameter.BMS.Battery.FullCapacity = 600;						// 电池完全充电电量，单位0.1AH
	g_SystemParameter.BMS.Battery.DesignCapacity = 600;						// 电池设计使用电量，单位0.1AH
	g_SystemParameter.BMS.Battery.CellOverVoltage = 4250;
	g_SystemParameter.BMS.Battery.CellUnderVoltage = 2600;
	g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 200;
	g_SystemParameter.BMS.Battery.ShortCutDownDelay = 400;
	g_SystemParameter.BMS.Battery.OverCutDownCurrent = 150;
	g_SystemParameter.BMS.Battery.OverCutDownDelay = 500;
	g_SystemParameter.BMS.Battery.CellChargeStopVoltage = 4195;				// 单体充电至4.195V截止
	g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 30;					// 充电最大允许电流，30A，对应60AH电池
	g_SystemParameter.BMS.Battery.BatteryTemperatureCheckEnable = 1;		// 使能电池温度检测
	g_SystemParameter.BMS.Battery.OverTemperature = 65;
	g_SystemParameter.BMS.Battery.UnderTemperature = -25;
	
	g_SystemParameter.BMS.Output.AutoCutoffCurrent = 50;					// 输出自动切断电流，50mA
	g_SystemParameter.BMS.Output.AutoCutoffDelay = 600;						// 输出自动切断延时，600s
	g_SystemParameter.BMS.Output.SleepDelay = 600;							// BMS进入低功耗状态前延时，单位s

	g_SystemParameter.BMS.Contactor.ContactorBaseFrequency = 1000;			// 设置PWM频率，单位Hz
	g_SystemParameter.BMS.Contactor.ContactorFullPercentTime = 2000;		// 接触器启动时全占空比持续时间，单位ms
	g_SystemParameter.BMS.Contactor.ContactorLongLastPercent = 750;			// 接触器长时间工作时占空比，单位0.1%

	g_SystemParameter.BMS.Warning.BDILowLevel1Percent = 15;					// 电池电量低百分比，单位%
	g_SystemParameter.BMS.Warning.BDILowLevel2Percent = 7;					// 电池电量严重过低百分比，单位%
	g_SystemParameter.BMS.Warning.BDILowLimitLift = 15;						// 电池电量低百分比，限制举升，单位%
	g_SystemParameter.BMS.Warning.BDILowLimitSpeed = 7;						// 电池电量低百分比，限制车速，单位%
	g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable = 1;			// 默认使能BMS管理板温度检测	
	g_SystemParameter.BMS.Warning.BMSOverTemperature = 90;					// 默认BMS管理板过温温度为85摄氏度

#elif (CONTROLLER_TARGET == BMS_XINXIN_A1)					// 设备选择，新昕滑板BMS，A1版
		
	g_SystemParameter.System.Information.DeviceName[0] = 1;
	g_SystemParameter.System.Information.DeviceName[1] = '2';
	g_SystemParameter.System.Information.SerialNumber1 = 12345;
	g_SystemParameter.System.Information.SerialNumber2 = 54321;
	g_SystemParameter.System.Information.ManufactureData = 1611;
	g_SystemParameter.System.Information.HardwareVersion = 100;
	g_SystemParameter.System.Information.SoftwareVersion = 33;				// 软件版本：0.3.3
	g_SystemParameter.System.Information.PrototalVersion = 0;

	// 电流传感器放大比例和偏置，需校准
	// CC值单位为8.44uV，采样电阻0.001欧姆，所以CC = 1对应电流为1 * 8.44 / 0.001 = 8440	
	// 电流传感器放大比例，输出单位mA，基值1000	
	// 默认1m欧姆采样电阻，即100A时对应电压100mV
	g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent = 1000;		// 100A
	g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage = 1000;		// 100mV
	g_SystemParameter.System.Calibration.BMSIdleCurrent = 10;				// BMS自身损耗电流，单位mA，实测得到，补偿电量计算
		
	g_SystemParameter.BMS.Battery.AFESelect = BQ76930;
	g_SystemParameter.BMS.Battery.SeriesNumber = 6;							// 6串
	g_SystemParameter.BMS.Battery.FullCapacity = 80;						// 电池完全充电电量，单位0.1AH
	g_SystemParameter.BMS.Battery.DesignCapacity = 80;						// 电池设计使用电量，单位0.1AH
	g_SystemParameter.BMS.Battery.CellOverVoltage = 4230;
	g_SystemParameter.BMS.Battery.CellUnderVoltage = 2800;
	g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 100;
	g_SystemParameter.BMS.Battery.ShortCutDownDelay = 400;
	g_SystemParameter.BMS.Battery.OverCutDownCurrent = 40;
	g_SystemParameter.BMS.Battery.OverCutDownDelay = 500;
	g_SystemParameter.BMS.Battery.CellChargeStopVoltage = 4200;				// 单体充电至4.2V截止
	g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 5;					// 充电最大允许电流，5A，对应8AH电池
	g_SystemParameter.BMS.Battery.BatteryTemperatureCheckEnable = 0;		// 不使能电池温度检测
	g_SystemParameter.BMS.Battery.OverTemperature = 65;
	g_SystemParameter.BMS.Battery.UnderTemperature = -25;
	
	g_SystemParameter.BMS.Output.AutoCutoffCurrent = 50;					// 输出自动切断电流，500mA
	g_SystemParameter.BMS.Output.AutoCutoffDelay = 600;						// 输出自动切断延时，600s
	g_SystemParameter.BMS.Output.SleepDelay = 600;							// BMS进入低功耗状态前延时，单位s
	g_SystemParameter.BMS.Output.OutputDelay = 1000;						// 开关按下1s后启动输出
	g_SystemParameter.BMS.Output.PreDischargeTime = 500;					// 预放电时间，单位ms
		
	g_SystemParameter.BMS.Warning.BDILowLevel1Percent = 15;					// 电池电量低百分比，单位%
	g_SystemParameter.BMS.Warning.BDILowLevel2Percent = 5;					// 电池电量严重过低百分比，单位%
	g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable = 0;			// 默认不使能BMS管理板温度检测
	g_SystemParameter.BMS.Warning.BMSOverTemperature = 90;					// 默认BMS管理板过温温度为85摄氏度
	
#elif (CONTROLLER_TARGET == BMS_XINXIN_A2)					// 设备选择，新昕滑板BMS，A2版
		
		
	g_SystemParameter.System.Information.DeviceName[0] = 'X';
	g_SystemParameter.System.Information.DeviceName[1] = 'I';
	g_SystemParameter.System.Information.DeviceName[2] = 'N';
	g_SystemParameter.System.Information.DeviceName[3] = ' ';
	g_SystemParameter.System.Information.DeviceName[4] = 'X';
	g_SystemParameter.System.Information.DeviceName[5] = 'I';
	g_SystemParameter.System.Information.DeviceName[6] = 'N';

	g_SystemParameter.System.Information.SerialNumber1 = 12345;
	g_SystemParameter.System.Information.SerialNumber2 = 54321;
	g_SystemParameter.System.Information.ManufactureData = 1611;
	g_SystemParameter.System.Information.HardwareVersion = 100;
	g_SystemParameter.System.Information.SoftwareVersion = 44;				// 软件版本：0.4.4
	g_SystemParameter.System.Information.PrototalVersion = 0;

	// 电流传感器放大比例和偏置，需校准
	// CC值单位为8.44uV，采样电阻0.001欧姆，所以CC = 1对应电流为1 * 8.44 / 0.001 = 8440	
	// 电流传感器放大比例，输出单位mA，基值1000	
	// 默认1m欧姆采样电阻，即100A时对应电压100mV
	g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent = 1000;		// 100A
	g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage = 2000;		// 200mV
	g_SystemParameter.System.Calibration.BMSIdleCurrent = 10;				// BMS自身损耗电流，单位mA，实测得到，补偿电量计算
		
	g_SystemParameter.BMS.Battery.AFESelect = BQ76930;
	g_SystemParameter.BMS.Battery.SeriesNumber = 6;							// 6串
	g_SystemParameter.BMS.Battery.FullCapacity = 80;						// 电池完全充电电量，单位0.1AH
	g_SystemParameter.BMS.Battery.DesignCapacity = 80;						// 电池设计使用电量，单位0.1AH
	g_SystemParameter.BMS.Battery.CellOverVoltage = 4250;
	g_SystemParameter.BMS.Battery.CellUnderVoltage = 2600;
	g_SystemParameter.BMS.Battery.ShortCutDownCurrent = 60;
	g_SystemParameter.BMS.Battery.ShortCutDownDelay = 500;
	g_SystemParameter.BMS.Battery.OverCutDownCurrent = 30;
	g_SystemParameter.BMS.Battery.OverCutDownDelay = 500;
	g_SystemParameter.BMS.Battery.CellChargeStopVoltage = 4200;				// 单体充电至4.2V截止
	g_SystemParameter.BMS.Battery.ChargeCurrentLimit = 6;					// 充电最大允许电流，6A，对应10AH电池
	g_SystemParameter.BMS.Battery.BatteryTemperatureCheckEnable = 0;		// 不使能电池温度检测
	g_SystemParameter.BMS.Battery.OverTemperature = 65;
	g_SystemParameter.BMS.Battery.UnderTemperature = -25;
	
	g_SystemParameter.BMS.Output.AutoCutoffCurrent = 50;					// 输出自动切断电流，100mA
	g_SystemParameter.BMS.Output.AutoCutoffDelay = 60;						// 输出自动切断延时，60s
	g_SystemParameter.BMS.Output.SleepDelay = 600;							// BMS进入低功耗状态前延时，单位s
	g_SystemParameter.BMS.Output.OutputDelay = 1000;						// 开关按下1s后启动输出
	g_SystemParameter.BMS.Output.PreDischargeTime = 500;					// 预放电时间，单位ms
		
	g_SystemParameter.BMS.Warning.BDILowLevel1Percent = 15;					// 电池电量低百分比，单位%
	g_SystemParameter.BMS.Warning.BDILowLevel2Percent = 5;					// 电池电量严重过低百分比，单位%
	g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable = 0;			// 默认不使能BMS管理板温度检测
	g_SystemParameter.BMS.Warning.BMSOverTemperature = 90;					// 默认BMS管理板过温温度为85摄氏度
	
#endif						// CONTROLLER_TARGET == BMS_EP_A5

}

// -------------------- 根据EEPROM参数，配置系统模块参数 ------------------------
void System_ParameterSet(void)
{
    // 输出关断延时，最长为24H
    if (g_SystemParameter.BMS.Output.SleepDelayLong > 24)
    {
        g_SystemParameter.BMS.Output.SleepDelayLong = 24;
    }
    
    g_SystemParameter.System.Test.test_BalancePointer = 0;
    g_SystemParameter.System.Test.test_Shutdown = 0;
    g_SystemParameter.System.Test.test_ClearFaultHistory = 0;    
    
	// 软件版本及部分信息由软件target.h宏定义决定，不受EEPROM存储参数影响
	g_SystemParameter.System.Information.SoftwareVersion = CONST_SOFTWARE_VERSION;
	g_Protect.ProtectMode.bit.BatteryTemperatureCheck = g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode;
	
	// 电流传感器放大比例和偏置，需校准
	// CC值单位为8.44uV，采样电阻0.001欧姆，所以CC = 1对应电流为1 * 8.44 / 0.001 = 8440
	// 电流传感器放大比例，输出单位mA，基值1000	
	g_AO_SH36730x0.Parameter.CCGain = (s32)g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent * 8440 / g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage;										
	g_AO_SH36730x0.Parameter.CCOffset = 0;	
	
	g_AO_SH36730x0.Parameter.SH36730x0_Type = g_SystemParameter.BMS.Battery.AFESelect;	
	
	g_AO_SH36730x0.Parameter.CellNumber = g_SystemParameter.BMS.Battery.SeriesNumber;
	
	// Take care: 根据不同的芯片和不同串数，采样单体不同！！！
	switch (g_SystemParameter.BMS.Battery.AFESelect)			// 前端采样芯片选择
	{
		case BQ76930:
		{
			switch (g_AO_SH36730x0.Parameter.CellNumber)
			{
				case 6:
				{
					g_AO_SH36730x0.Parameter.CellSelect[0] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[1] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[2] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[3] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[4] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[5] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[6] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[7] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[8] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[9] = 1;			
				}
				break;
				
				case 7:
				{
					g_AO_SH36730x0.Parameter.CellSelect[0] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[1] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[2] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[3] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[4] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[5] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[6] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[7] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[8] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[9] = 1;			
				}
				break;

				case 8:
				{
					g_AO_SH36730x0.Parameter.CellSelect[0] = 1;//修改过注意----------------------------------------------------------------
					g_AO_SH36730x0.Parameter.CellSelect[1] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[2] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[3] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[4] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[5] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[6] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[7] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[8] = 0;
					g_AO_SH36730x0.Parameter.CellSelect[9] = 0;			
				}
				break;
										
				case 10:
				{
					g_AO_SH36730x0.Parameter.CellSelect[0] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[1] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[2] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[3] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[4] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[5] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[6] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[7] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[8] = 1;
					g_AO_SH36730x0.Parameter.CellSelect[9] = 1;			
				}
				break;		
				
				default:
				{
					// 报错，参数设置错误
					Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);					
				}
				break;
			}
		}
		break;
		
		default:						// 前端采样芯片不可识别，报错
		{
			// 报错，参数设置错误
			Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);
		}
		break;
	}
    if (g_SystemParameter.BMS.BatteryType.BatteryType != g_SystemParameter.BMS.BatteryType.BatteryTypeReal)
    {
        // 电池类型发生改变，未生效
        // 报错，参数设置错误
        Protect_SetFaultCodeLv2(&g_Protect, ERROR_ILLEGAL_PARAMETER);   

        g_SystemMonitor.System.Reverse.Data[0] = 4;
    }
    //设置单体过压、欠压电压
	g_AO_SH36730x0.Parameter.SingleOverVoltage = g_SystemParameter.BMS.Battery.CellOverVoltage;			// 单体电池过压，3700mV
	g_AO_SH36730x0.Parameter.SingleUnderVoltage = g_SystemParameter.BMS.Battery.CellUnderVoltage;	   // 单体电池欠压，2400mV

    switch (g_SystemParameter.BMS.BatteryType.BatteryType)
    {
        case 0: // 三元
        default:
        {
            if (g_SystemParameter.BMS.Battery.CellOverVoltage > 4350 
                || g_SystemParameter.BMS.Battery.CellOverVoltage < 4000)
            {
                // 三元电池单体过压电压设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);  
                
                g_SystemMonitor.System.Reverse.Data[0] = 5;
            }
            
            if (g_SystemParameter.BMS.Battery.CellUnderVoltage > 3400 
                || g_SystemParameter.BMS.Battery.CellUnderVoltage < 2000)
            {
                // 三元电池单体欠压压电压设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 6;                
            }          

            if (g_SystemParameter.BMS.Battery.CellChargeStopVoltage > 4250 
                || g_SystemParameter.BMS.Battery.CellChargeStopVoltage < 4000)
            {
                // 三元电池充电截止电压设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 7;
            }
            
            if (g_SystemParameter.BMS.Charge.ChargeVoltageStep1 > 4250
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep1 < 3850
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep2 > 4250
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep2 < 3850
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep3 > 4250
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep3 < 3850
                || g_SystemParameter.BMS.Charge.ChargeForceStopVoltage > 4250
                || g_SystemParameter.BMS.Charge.ChargeFinishMinVoltage < 4000)
            {
                // 三元电池充电阶段电压设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);

                g_SystemMonitor.System.Reverse.Data[0] = 8;
            }
            
            if (g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage < 2500 
                || g_SystemParameter.BMS.Discharge.DischargeStopVoltage < 2900)
            {
                // 三元电池放电参数设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 9;                
            }
            
           /* if (g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 > 3500
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 < 3000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 > 3500)
            {
                // 三元电池电量校准参数设置错误
                Protect_SetFaultCodeLv2(&g_Protect, ERROR_ILLEGAL_PARAMETER);  

                g_SystemMonitor.System.Reverse.Data[0] = 10;                
            }*/
        }
        break;
    
        case 1:                             // 磷酸铁锂
        {
            if (g_SystemParameter.BMS.Battery.CellOverVoltage > 3800 
                || g_SystemParameter.BMS.Battery.CellOverVoltage < 3400)
            {
                // 磷酸铁锂电池单体过压电压设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);  
                
                g_SystemMonitor.System.Reverse.Data[0] = 11;
            }
            
            if (g_SystemParameter.BMS.Battery.CellUnderVoltage > 2600 
                || g_SystemParameter.BMS.Battery.CellUnderVoltage < 1800)
            {
                // 磷酸铁锂电池单体欠压压电压设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);  
                
                g_SystemMonitor.System.Reverse.Data[0] = 12;
            }  

            if (g_SystemParameter.BMS.Battery.CellChargeStopVoltage > 3700 
                || g_SystemParameter.BMS.Battery.CellChargeStopVoltage < 3400)
            {
                // 磷酸铁锂电池充电截止电压设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER);  
                
                g_SystemMonitor.System.Reverse.Data[0] = 13;
            } 

            if (g_SystemParameter.BMS.Charge.ChargeVoltageStep1 > 3700
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep1 < 2600
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep2 > 3700
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep2 < 2600
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep3 > 3700
                || g_SystemParameter.BMS.Charge.ChargeVoltageStep3 < 2600
                || g_SystemParameter.BMS.Charge.ChargeForceStopVoltage > 3760
                || g_SystemParameter.BMS.Charge.ChargeFinishMinVoltage < 3350)
            {
                // 磷酸铁锂电池充电阶段电压设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 14;                
            }  

            if (g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage < 1600 ||
                 g_SystemParameter.BMS.Discharge.DischargeStopVoltage < 1900)
            {
                // 磷酸铁锂电池放电参数设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 15;                
            } 

            if (g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3 > 3200
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 < 2000
                || g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4 > 3200)
            {
                // 磷酸铁锂电池电量校准参数设置错误
                Protect_SetFaultCodeLv1(&g_Protect, ERROR_ILLEGAL_PARAMETER); 

                g_SystemMonitor.System.Reverse.Data[0] = 16;                
            }            
        }
        break;
    }        
        
	g_AO_BMS.Parameter.BatteryFullCapacity = g_SystemParameter.BMS.Battery.FullCapacity * 100;			// 电池组实际容量，单位mAH
	g_AO_BMS.Parameter.BatteryDesignCapacity = g_SystemParameter.BMS.Battery.DesignCapacity * 100;		// 设计电池组容量为8.4AH，单位mAH，实际容量大于此值约10%
	g_AO_BMS.Parameter.BMSIdleCurrent = g_SystemParameter.System.Calibration.BMSIdleCurrent;			// BMS自身负载电流，单位mA，用于补偿电量损耗
    

	//g_AO_SH36730x0.State.BatteryBalanceEnable = 0;				// 电池均衡初始化时为关闭，由BMS状态机控制
	g_AO_SH36730x0.Parameter.BallanceErrVoltage = 2000;				// 默认电压误差超过20mV，电池均衡开启
/*	
	// 当参数中禁用电池温度检测时，protect模块停止检测电池温度
	if (g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode > 0)
	{
		g_Protect.ProtectMode.bit.BatteryTemperatureCheck = 1;		// 使能电池温度检测
	}
	else
	{
		g_Protect.ProtectMode.bit.BatteryTemperatureCheck = 0;		// 禁用电池温度检测
	}
*/					// CONTROLLER_TARGET == BMS_EP_200_A1
	    
    // AD采样参数更新
    g_ADC.Parameter.VdcSensorGain = g_SystemParameter.System.Calibration.VdcSensorGain;                         // 直流电压传感器放大比例，单位0.1kOhm
    g_ADC.Parameter.VdcSensorOffset = g_SystemParameter.System.Calibration.VdcSensorOffset;                     // 直流电压传感器偏置，单位0.1V
    g_ADC.Parameter.InnerDriveSensorGain = g_SystemParameter.System.Calibration.InnerDriveSensorGain;           // 内部驱动电压传感器放大比例，单位0.1kOhm
    g_ADC.Parameter.InnerDriveSensorOffset = g_SystemParameter.System.Calibration.InnerDriveSensorOffset;       // 内部驱动电压传感器偏置，单位0.1V 
    g_AO_BMS.Parameter.FanCurrent = g_SystemParameter.BMS.Fan.FanCurrent;                                       // 风扇运行电流，单位mA，

}

s32 get_temp_current_v(s16 t, s32 c)
{
    s32 temp_current_v = 0;
    if(c < -1000)temp_current_v = -g_SystemParameter.BMS.Discharge.dsg_cc_low_k * c / g_SystemParameter.BMS.Battery.DesignCapacity / 100;
    if(t < 150)
    {
        if(!g_SystemParameter.BMS.Discharge.dsg_tmp_low_k)g_SystemParameter.BMS.Discharge.dsg_tmp_low_k = 10;
        temp_current_v += (250 - t) / 10 * (250 - t) * g_SystemParameter.BMS.Discharge.dsg_tmp_low_k / 500;
    }
    return temp_current_v;
}


// End of system_module.c


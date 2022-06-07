/* ==================================================================================

 File name:     system_core.c
 Originator:    BLJ
 Description:   系统核心文件，全部程序连接。变量实例化，中断循环处理，原vehicle.c

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 11-24-2014     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#include "system_core.h"                    	// 系统核心头文件
#include "CRC8.h"								// CRC校验值模块
#include <math.h>

// ----------------------------- 内部可调用函数 ------------------------------------
void System_10msTimerUpdate(void);						// 10ms慢速时基更新函数

// ----------------------------- 数据变量结构体实例化 -------------------------------
//CAN_structDef       		g_CAN;                      // CAN模块结构体实例化
//I2C_BQ769x0_structDef		g_I2C_BQ769x0;				// I2C EEPROM模块
I2C_BQ769x0_structDef		g_I2C_SH36730x0;		    // I2C EEPROM模块
flash_EEPROM_structDef		g_flash_EEPROM;				// flash模拟EEPROM
USART_MODBUS_structDef		g_USART_MODBUS;				// 串口MODBUS通信模块
Protect_structDef			g_Protect;					// Protect模块

Input_structDef				g_Input;					// 数字输入模块
Output_structDef			g_Output;					// 数字驱动模块
ADC_structDef				g_ADC;						// ADC采样模块
LED_Flicker_structDef		g_LED_Flicker;				// LED闪烁模块
//Contactor_structDef			g_Contactor;				// 主接触器控制模块

Comm_structDef      		g_communication;            // 设备间通信数据结构体实例化

SYSTEM_STATE_structDef		g_SystemState;				// 系统状态
SYSTEM_PARAMETER_structDef	g_SystemParameter;			// 系统参数
SYSTEM_MONITOR_structDef	g_SystemMonitor;			// 系统采样值
SYSTEM_RECORD_structDef		g_SystemRecord;				// 设备运行记录
FAULT_HISTORY_structDef		g_SystemFaultHistory;		// 系统故障记录//故障记录


// -------------------------------- 系统全局变量 ----------------------------------


// -------------------------------- 内部局部变量 ----------------------------------
u16	l_10msTimerFlag = 0;						// 10ms时基更新标志位
u16	l_200msTimerFlag = 0;						// 200ms时基更新标志位
u16	l_1sTimerFlag = 0;							// 1s时基更新标志位

// -------------------------------- 内部临时变量 ----------------------------------

u16 t_ADC[5];

//u16	l_KEYState = 0;

u16 t_LCDtest[5] ={0, 0, 0, 0, 0};

u32	t_EEPROMtest[5] = {0, 0x09, 1, 0, 0};
u16	t_EEPROMtest2[20];
u16	t_EEPROMtest3[20];

u8	t_EERPOMCRC[5] = {0x10, 0x09, 0xBD, 0, 0};
u8	t_BQTest[5];

u8	t_DSGEnable[5];



u16	l_FaultSIGSendFlag = 0;

u16	t_flashEEPROMtest[5] = {0, 0, 0, 0, 0};

u16	t_OutTest[5];
u16	t_CANtest3[5];

u16	l_ChargeCurrentLimitDivision = 1;		// 充电电流限制分频，例：2：即0.5C
u16	l_ChargeCurrentLimit = 0;				// 充电电流限制值，单位0.1A

s32	t_SingleMaxVoltage = 3900;

u16	l_CANNoChargerStateCount = 0;
//u16	l_ChargerFaultFlag = 0;

u16 t_LED[5];

u16 t_OutputCmdDisMatchCnt = 0;
u16 t_ResetAsk = 0;
u16 t_ResetCnt = 0;

u16 t_ttRecord[5];
u16 t_TempLowError = 0;

u16 g_ChargerOfflineCnt = 0;
u16 g_ChargerOnlineFlag = 0;
u16 l_ChargeFaultRecoverDelay = 0;
u16 test_rebuffer[225];
u16 t_BeepDelay = 0;

//u16 boot_flag=0;//==============================================================测试
// ----------------------------- 系统定时中断，1ms --------------------------------
void System_1msInterrupt(void)
{
    
////=============================IIC通讯更新=======================================	

//	I2C_BQ769x0_Process(&g_I2C_SH36730x0);//读写事件
//	I2C_BQ769x0_SIG_Update();            //更新读写事件结果

////===============================================================================
	g_SystemState.Output.System1msInterruptCount++;
	
//	// 此处增加一个充电机指令接收判断，超时判断，故障判断
//	if (g_SystemParameter.BMS.Charge.ChargerCommEnable > 0) //无此功能
//	{
//		l_CANNoChargerStateCount++;				
//		if (l_CANNoChargerStateCount > 1000)
//		{
//			l_CANNoChargerStateCount = 1000;
//			
//			// 超时，1s内未收到充电机发出的数据帧
//            g_ChargerOnlineFlag = 0;			
//		}
//        
//        // 充电机未在线且KEY为无效位时，方允许清除充电机故障标志位
//        // 防止出现以下情况：充电机报错，KEY未断开，电池响应并切断输出，此时发生CAN总线断线，若清除充电机故障，会导致电池立刻重新输出
//        // 必须等到KEY断开后，再清除充电机故障，保证电池KEY重新使能后，电池才允许再次输出
//        if (g_ChargerOnlineFlag == 0 && g_Input.Output.DIN_bits.bit.KEY == 0)
//        {
//            g_SystemState.State.bit.ChargerFaultFlag = 0;
//        } 
//        if(g_AO_SH36730x0.Output.BatteryCurrent >= g_SystemParameter.BMS.Charge2.ChargeCurrent)
//        {
//            if(g_AO_BMS.Output.BatteryTemperatureLow < g_SystemParameter.BMS.Charge2.ChargeLimitTemperature3*10)//10度
//            {
//                g_AO_BMS.State.OutputAllow = 0; 
//            }
//            else
//            {
//                g_AO_BMS.State.OutputAllow = 1;
//            }
//        }
//	}
//	else
//	{
//		l_CANNoChargerStateCount = 0;
//	}		
//上位机烧录指令读取
//===========================================================================================
//		//30ms执行一次
//	if(g_SystemState.Output.System1msInterruptCount%30 == 13)//
//    {
//        //上位机烧录响应指令				
//        if(g_communication.BootCMD.BYTE0==0x3A && g_communication.BootCMD.BYTE1==0x00 \
//            && g_communication.BootCMD.BYTE2==0x00 && g_communication.BootCMD.BYTE4==0x00 \
//        && g_communication.BootCMD.BYTE5==0x00 && g_communication.BootCMD.BYTE6==0xF0)
//        {
//			boot_flag++;//===============================
//	  //重新复制一份程序无偏移测试，是否可以接收上位机指令进入
//          
//        }
//				 
//    }
	
//============================================================================================
	
	// 系统慢速时基,注意余数取不同值，防止时基被同时触发	
	if ((g_SystemState.Output.System1msInterruptCount % 200) == 0)//
	{
		l_200msTimerFlag = 1;	        
		// 系统定时通信更新,包含定时对外广播电池状态
		// SystemComm_Update();		
	}
	if ((g_SystemState.Output.System1msInterruptCount % 1000) == 5)//
	{
		l_1sTimerFlag = 1;
		
		// Take care: 3号定时器时基是1s
		QF_tickXISR(3U);          				// process time events at rate 2 	
		// --------------------- 1s更新一次系统状态 -----------------------
		SystemMonitor_Update();			
        
	}	
	
    if(g_SystemParameter.BMS.Fan.FanMode == 2)//低电量提醒
    {
        // 判断是否禁用举升
        if((g_AO_BMS.Output.SOC < g_SystemParameter.BMS.Warning.LowSOCBeep * 10)//17%
            && (g_Input.Output.DIN_bits.bit.KEY > 0))
        {
            // 当电量低，蜂鸣器工作
            Output_BUZZER_Update(&g_Output, 1);
            g_Output.State.bit.FanEnable = 1;
        }
        else
        {
            t_BeepDelay = 0;
            g_Output.State.bit.FanEnable = 0;
            Output_BUZZER_Update(&g_Output, 0);
        }  
    }
    else
    {
        Output_BUZZER_Update(&g_Output, 0);
        g_Output.State.bit.FanEnable = 0;
    }	

	// 默认所有产品支持串口调试，不再支持旧版无串口调试功能硬件
//#if (CONTROLLER_TARGET == BMS_EP_A5 || BMS_EP_20_REV1_1_2TEMP || CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_XINXIN_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)				// EP BMS A3版本，带串口调试功能
	
	// 串口MODBUS模块时基函数，运行在1ms时基中
	USART_MODBUS_TimeTick(&g_USART_MODBUS);

	if ((g_SystemState.Output.System1msInterruptCount % 5) == 0)	
	{
		// 串口MODBUS模块进程函数
		USART_MODBUS_Process(&g_USART_MODBUS);
	}
	
//#endif						
	
	// 数字输入模块更新函数
	Input_Update(&g_Input);//包含钥匙KEY及分流器连接状态检测
    
    // ADC采样，1ms一次
    ADC_SlowUpdate(&g_ADC);//不能注释否则上电瞬间有一定可能会导致，ADC检测错误

	// 慢速保护更新
	Protect_SlowUpdate(&g_Protect);
	
#if (CONTROLLER_TARGET == BMS_EP_200_A1 || CONTROLLER_TARGET == BMS_EP_200_A2 || CONTROLLER_TARGET == BMS_EP_200_B1 || CONTROLLER_TARGET == BMS_EP_200_B2_1)
	
	// 更新接触器控制输出，仅带接触器版本生效
	if (g_Protect.FaultCode.SystemLockLevel0 == 0 
		&& g_Protect.FaultCode.OutputDisableLevel1 == 0 
		&& g_Input.Output.DIN_bits.bit.DSG == 1 
		&& g_Input.Output.DIN_bits.bit.CHG == 1)
	{
		g_Contactor.Input.ContactorEnable = 1;		
	}
	else
	{
		g_Contactor.Input.ContactorEnable = 0;
	}
		
	Contactor_Update(&g_Contactor);
	
#endif						// CONTROLLER_TARGET == BMS_EP_200_A1

// LED刷新
	System_LED_Update();
//	Output_LED_RED_Update(&g_Output, t_LED[0]);
//	Output_LED_GREEN_Update(&g_Output, t_LED[1]);
//	Output_LED_BLUE_Update(&g_Output, t_LED[2]);
	
    // For test: 增加故障检测，充放电指令与反馈不一致时报错
    if (g_AO_SH36730x0.State.DSGControl != g_AO_SH36730x0.SH36730x0Register.SCONF2.bit.DSG_C 
        || g_AO_SH36730x0.State.CHGControl != g_AO_SH36730x0.SH36730x0Register.SCONF2.bit.CHG_C)//CHG_C
    {
        t_ttRecord[0] = g_AO_SH36730x0.State.DSGControl;
        t_ttRecord[1] = g_AO_SH36730x0.SH36730x0Register.SCONF2.bit.CHG_C;
        t_ttRecord[2] = g_AO_SH36730x0.State.CHGControl;
        t_ttRecord[3] = g_AO_SH36730x0.SH36730x0Register.SCONF2.bit.DSG_C;        
        
        t_OutputCmdDisMatchCnt++;
        if (t_OutputCmdDisMatchCnt > 3000) //检测到3000次不一致
        {
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_CMD_DISMATCH);
                       
            t_ResetAsk = 1;
            
            t_OutputCmdDisMatchCnt = 0;
        }
    }
    else
    {
        if (t_OutputCmdDisMatchCnt > 0)
        {
            t_OutputCmdDisMatchCnt--;
        }
    }
    
	if (g_SystemState.State.bit.IWDGReloadEnable > 0)			// 允许喂狗
	{
        if (t_ResetAsk > 0)
        {
            t_ResetCnt++;
            
            if (t_ResetCnt > 3000)
            {
                t_ResetAsk = 0;
                t_ResetCnt = 0;
                
                g_SystemState.State.bit.IWDGReloadEnable = 0;
            }
        }
        
		IWDG_ReloadCounter();
	}
    else
    {
        t_ResetAsk = 0;
        t_ResetCnt = 0;        
    }
			

}
u16 l_BatteryTemperatureCalcCnt = 0;
s32 t_TempHi = 0;
s32 t_TempLow = 0;
u16 l_LEDFlashCnt = 0;
u16 system_reset_cnt = 0;
// ----------------------------- 系统主循环函数 -------------------------------------
void System_MainLoop(void)
{		
	g_SystemState.Output.SystemMainLoopCount++;			// 主循环执行次数++
    
//=============================IIC通讯更新=======================================	

	I2C_BQ769x0_Process(&g_I2C_SH36730x0);//读写事件
	I2C_BQ769x0_SIG_Update();            //更新读写事件结果

//===============================================================================

	// 外部KEY输入口事件更新
	if (g_SystemState.State.bit.KEYState == 0)//
	{
		// KEY使能，且充电机故障标志位未置位,(有钥匙与充电机无故障)
		if (g_Input.Output.DIN_bits.bit.KEY > 0 && g_SystemState.State.bit.ChargerFaultFlag == 0)
		{			
			QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 1);	//注意最后一个参数为1
			
			g_SystemState.State.bit.KEYState = g_Input.Output.DIN_bits.bit.KEY;//
		}
	}
	else
	{
		// KEY不使能，或充电机故障标志位置位
		if (g_Input.Output.DIN_bits.bit.KEY == 0 || g_SystemState.State.bit.ChargerFaultFlag > 0)
		{
		//	g_SystemMonitor.System.Reverse2.Data[15]++;
			
			QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 0);	//注意最后一个参数为0
			
			g_SystemState.State.bit.KEYState = 0;
		}		
	}    
	
	
	// flash模拟EEPROM事件更新
    flash_EEPROM_SIG_Update();
	
	// 系统调试更新相关函数
	Debuger_Update();
	
	if (l_200msTimerFlag > 0)			// 200ms定时时基
	{
		l_200msTimerFlag = 0;
       // g_communication.Test1.Byte[0] = (g_AO_BQ769x0.Output.ExternalTemperature[1] >> 8) & 0x00FF;
       // g_communication.Test1.Byte[1] = (g_AO_BQ769x0.Output.ExternalTemperature[1] & 0x00FF);
       // CAN_Send(&g_CAN, 11);
		// 当收到参数写入EEPROM请求后，延时约1s左右执行命令
		if (g_SystemState.Variable.ParameterWriteAsk > 0)
		{
			if (g_SystemState.Variable.ParameterWriteDelay++ > 5)//延时写入
			{		
				// 发送EEPROM写入事件
				QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, EEPROM_BLOCK_PARAMETER);	
				
				g_SystemState.Variable.ParameterWriteDelay = 0;
				g_SystemState.Variable.ParameterWriteAsk = 0;
			}
		}
		else
		{
			g_SystemState.Variable.ParameterWriteDelay = 0;
		}
        
		// 200ms发送一次故障信息
		if (Protect_GetFaultCode(&g_Protect) > 0)//故障
		{
			QACTIVE_POST((QActive *)&g_AO_BMS, FAULT_SIG, Protect_GetFaultCode(&g_Protect));
		} 

        if (Protect_GetFaultCode(&g_Protect) == 0
            && Protect_GetWarningCode(&g_Protect) == 0)
        {
            // 无故障时2s闪烁一次
            l_LEDFlashCnt++;
            system_reset_cnt = 0;
            
            if (l_LEDFlashCnt >= 5)
            {
                // 测试灯闪烁
                if (g_Output.State.bit.LED_RED == 0)
                {
                    Output_LED_RED_Update(&g_Output, 1);
                }
                else
                {
                    Output_LED_RED_Update(&g_Output, 0);
                }
                
                l_LEDFlashCnt = 0;
            }
        }
        else
        {
            if(g_Input.Output.DIN_bits.bit.KEY == 0 &&  Protect_GetFaultCode(&g_Protect) >0)
            {
                if(system_reset_cnt++ > 100)
                    NVIC_SystemReset(); //如果检测长时间没有没钥匙则软件复位
            }
            // 有故障时0.4s闪烁一次
            // 测试灯闪烁
            if (g_Output.State.bit.LED_RED == 0)
            {
                Output_LED_RED_Update(&g_Output, 1);
            }
            else
            {
                Output_LED_RED_Update(&g_Output, 0);
            }            
        }  
		// 中力使用电池温度NTC型号为103AT，Res = 10K，B = 3435
		// ADC采样温度，200ms一次即可  
        // 电池包的温度检测，使能两路 
        ADC_SlowUpdate(&g_ADC);
        
         if(g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode > 0)
         {
             l_BatteryTemperatureCalcCnt++;
             if (l_BatteryTemperatureCalcCnt >= g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode
                 || l_BatteryTemperatureCalcCnt >= 8)
             {
                 l_BatteryTemperatureCalcCnt = 0;
                 
                 g_AO_BMS.Output.BatteryTemperatureHi = t_TempHi;
                 g_AO_BMS.Output.BatteryTemperatureLow = t_TempLow;    
                 t_TempHi = -1000;
                 t_TempLow = 1000;    
             }
                 
             // NTC型号：103AT
             g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt] = (s32)10.0 / (((log(g_ADC.Output.BatteryNTCRes[l_BatteryTemperatureCalcCnt]) - log(10000))/3435) + 1.0/298.15) - 2730;       
             
             if (g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt] > t_TempHi)
             {
                 t_TempHi = g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt];
             }
             
             if (g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt] < t_TempLow)
             {
                 t_TempLow = g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt];
             }  
             if (g_AO_BMS.Output.BatteryTemperature[l_BatteryTemperatureCalcCnt] < -500)
             {
                 Protect_SetFaultCodeLv2(&g_Protect, WARNING_BATTERY_NTC_FAILURE);
             }   
        
         }
         else
         {
            g_AO_BMS.Output.BatteryTemperature[0] = 0;
            g_AO_BMS.Output.BatteryTemperature[1] = 0;
         }
			
#if (CONTROLLER_TARGET == BMS_EP_ALPHA)

       // BMS均流板温度检测 使能两路
        if (g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable > 0)           // BMS管理板温度检测使能，一共两路
        {
            // 200ms运行一次
            // NTC型号：Res = 10K， B = 3950
            g_AO_BMS.Output.BMSTemperature[0] = (s32)10.0 / (((log(g_ADC.Output.BalanceBoardNTCRes[0]) - log(10000))/3950) + 1.0/298.15) - 2730;
            
            if (g_AO_BMS.Output.BMSTemperature[0] < -500)
            {
                Protect_SetFaultCodeLv2(&g_Protect, WARNING_BMS_NTC_FAILURE);
            }   

            // NTC型号：Res = 10K， B = 3950
            g_AO_BMS.Output.BMSTemperature[1] = (s32)10.0 / (((log(g_ADC.Output.BalanceBoardNTCRes[1]) - log(10000))/3950) + 1.0/298.15) - 2730;
            
            if (g_AO_BMS.Output.BMSTemperature[1] < -500)
            {
                Protect_SetFaultCodeLv2(&g_Protect, WARNING_BMS_NTC_FAILURE);
            }  
            
            if(g_AO_BMS.Output.BMSTemperature[0] > g_AO_BMS.Output.BMSTemperature[1])
            {
                g_AO_BMS.Output.BMSTemperatureHi = g_AO_BMS.Output.BMSTemperature[0];
            }
            else
            {
                g_AO_BMS.Output.BMSTemperatureHi = g_AO_BMS.Output.BMSTemperature[1];
            }
        }
        else
        {
            g_AO_BMS.Output.BMSTemperature[0] = 0;
            g_AO_BMS.Output.BMSTemperature[1] = 0;
        }     	
#elif (CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP)
		
		if (g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable > 0)			// BMS管理板温度检测使能
		{
			// 20AH量产版，多一路温度采样，用于采集BMS自身温度，暂时仅采集显示，不做保护
			// 200ms运行一次
			// NTC型号：Res = 10K， B = 3950
			g_AO_BMS.Output.BMSTemperature[0] = (s32)10.0/(((log(g_ADC.Output.NTCRes2)-log(10000))/3950) + 1.0/298.15) - 2730;
			
			if (g_ADC.Output.ADCResult[1] > 4050)
			{
				Protect_SetFaultCodeLv1(&g_Protect, WARNING_BMS_NTC_FAILURE);
			}	
            g_AO_BMS.Output.BMSTemperatureHi = g_AO_BMS.Output.BMSTemperature[0];
		}
		else
		{
			g_AO_BMS.Output.BMSTemperature[0] = 0;
		}
		
#elif (CONTROLLER_TARGET == BMS_EP_200_B2_1)		// 此硬件版本BMS管理板温度采用LMT87测得
		
		if (g_SystemParameter.BMS.Warning.BMSOTemperatureCheckEnable > 0)			// BMS管理板温度检测使能
		{
			// LMT87温度传感器，转换公式，T = -0.0718*LMT87Voltage + 189.16
			// 注意温度单位为0.1摄氏度
			g_AO_BMS.Output.BMSTemperature = 1892 - (s32)g_ADC.Output.LMT87Voltage * 718 / 1000;
			
			if (g_ADC.Output.ADCResult[2] > 4050 || g_ADC.Output.ADCResult[2] < 50)
			{
				Protect_SetFaultCodeLv2(&g_Protect, WARNING_BMS_NTC_FAILURE);
			}			
		}
		else
		{
			g_AO_BMS.Output.BMSTemperature = 0;
		}				
		
#endif				// CONTROLLER_TARGET == BMS_EP_20_REV1_1_2TEMP
				
	}
	
	if (l_1sTimerFlag > 0)				// 1s定时时基
	{
		l_1sTimerFlag = 0;		

#if (CONTROLLER_TARGET == BMS_EP_200_B2_1)		// 此硬件带测试LED
		
		// 测试灯闪烁
		if (g_Output.State.bit.TestLED == 0)
		{
			Output_TestLED_Update(&g_Output, 1);
		}
		else
		{
			Output_TestLED_Update(&g_Output, 0);
		}
		
#endif
		
		
	}
	
	// 当发生故障时，向BMS发送FAULT_SIG
	if (l_FaultSIGSendFlag == 0)
	{
		if (Protect_GetFaultCode(&g_Protect) > 0)
		{
			QACTIVE_POST((QActive *)&g_AO_BMS, FAULT_SIG, Protect_GetFaultCode(&g_Protect));
			
			l_FaultSIGSendFlag = 1;
		}
	}
	else
	{
		if (Protect_GetFaultCode(&g_Protect) == 0)
		{
			l_FaultSIGSendFlag = 0;
		}
	}
	
	// for test： 测试flash EEPROM模块
	if (t_flashEEPROMtest[0] > 0)
	{
		t_flashEEPROMtest[0] = 0;
		
		QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, t_flashEEPROMtest[1]);	
	}
						
//	LCD_Process(&g_LCD);					// LCD模块过程函数
	
	flash_EEPROM_Process(&g_flash_EEPROM);		// flash模拟EEPROM模块过程函数
	
	// for test:
/*	if (g_flash_EEPROM.State.State_bits.bit.Command > 0)
	{
		Output_Out1_Update(1);
	}
	else
	{
		Output_Out1_Update(0);
	}*/
	
//	EEPROM_SIG_Update();					// EEPROM相关模块事件发送函数
	
/*	if (l_10msTimerFlag > 0)
	{
	//	I2C_EEPROM_Process(&g_I2C_EEPROM);		// I2C EEPROM模块过程函数
		
		System_10msTimerUpdate();					// 10ms慢速时基更新函数
		
		l_10msTimerFlag = 0;
	}*/
	
	
    // 若进入bootloader请求被置位，则发送ENTER_BOOTLOADER_SIG事件，并清除此标志位
//    if (g_SystemState.State.bit.EnterBootloaderAsk > 0)
//    {
//    
//        // 向BMS状态机发送进入Bootloader事件
//        QACTIVE_POST((QActive *)&g_AO_BMS, ENTER_BOOTLOADER_SIG, 0);	

//        // 任务完成，清除标志位
//        g_SystemState.State.bit.EnterBootloaderAsk = 0;        
//    }
    
	
	
	
}

// ------------------------ 10ms慢速时基更新函数 -----------------------------
void System_10msTimerUpdate(void)
{

}

// End of system_core.c


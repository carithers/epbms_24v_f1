/* ==================================================================================

 File name:     AO_BMS.c
 Originator:    BLJ
 Description:   电池管理系统状态机
 Take care： 所有状态活动对象，定时器1用于周期性定时

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 12-20-2015     Version 0.0.1           基本功能
-----------------------------------------------------------------------------------*/


// ---------------------------- 电池电量计算方法 --------------------------------
// To be update： 待定

/* BQ769x状态机会发送数据更新事件，BMS状态机收到此事件后，处理数据，并计算电池剩余电量。
1.电量计算方法基于AH法，并做一定补偿。
2.充电时默认为0.5C充电，标准AH法计算电量，当电量大于设计电量时停止充电。或电池单体最高电压大于4.2V，
  此时电池计算电量小于设计电量，存储此参数为电池实际电量。若电池实际电量小于设计值，则以实际电量为准，计算SOC。
3.放电时默认0.5C，但最低单体电压较低时，根据电流补偿，估算剩余电量（此电量略大于实际值，仅作补偿）。
  如计算电量大于估算剩余电量，则以一定速率降低计算电量，电量越低，速率越高。
*/


#include "stm32f0xx.h"                         // STM32器件寄存器定义头文件
#include "string.h"                             // 调用memset函数用
#include "system_core.h"                        // 系统核心头文件
#include "qpn_port.h"                           // 状态机头文件
 

// ---------------------- 状态机运行记录，测试用 -----------------------------
#define AO_DEBUG    1

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
//u16 ChargeCnt = 0;            // 用于辅助判断充放电状态
//u16 CutOffDelay = 0;

// ------------------------------ 内部可调用功能函数 --------------------------------
void BatteryCapacity_Update(AO_BMS * const me);                 // 电池电量估算
void BatteryCapacity_Calibrate(AO_BMS * const me);				// 电池电量补偿，1s运行一次


// -------------------------------- 清除函数 ------------------------------------
void AO_BMS_ctor(AO_BMS * const me) {
    QActive_ctor(&me->super, Q_STATE_CAST(&AO_BMS_initial));
}

// -------------------------------- 初始状态 ------------------------------------
QState AO_BMS_initial(AO_BMS * const me) {
    return Q_TRAN(&AO_BMS_StartWait);
}

// ------------------------------- 1.启动等待，一段时间，再读取EEPROM ----------------------------------
QState AO_BMS_StartWait(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(1);
#endif
            
            // 开启定时对外发送CAN数据功能
            me->State.CANCommEnable = 1;						// 允许开始CAN总线通讯
            
            g_AO_SH36730x0.State.BatteryBalanceEnable = 0;        // 关闭均衡 
            
			// 启动后等待2s时间，若超时，认为系统启动失败，BMS无法正常工作
            QActive_armX((QActive *)me, 0U, 2000U);        	// 开启定时器0，2s

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {
            // 设置故障信息，BMS启动失败
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_START_FAIL);
            
            status = Q_TRAN(&AO_BMS_Fault);             		// 跳转至BMS故障状态
        }
        break;
        
        case START_SIG:                                 		// 启动事件，BQ769x0芯片初始化完成，无错误发生
        {			
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时器0       
            	
            // 进入主流程前，需配置一次电池最大可充入电量，由运行记录处读取
            if (g_SystemRecord.BatteryFullCapacity == 0)		// 第一次启动时，运行记录中可能无电池充满电量，则赋值为设计电量
            {
                me->Output.FullCapacity = me->Parameter.BatteryDesignCapacity;
            }
            else
            {
				// 充满电量，单位1mAH
                me->Output.FullCapacity = g_SystemRecord.BatteryFullCapacity;			//进-------
            }
                    
            // 若进入Idle状态时，KEY已闭合，则发送DIN_SIG，电池进入输出状态
            if (g_Input.Output.DIN_bits.bit.KEY > 0)
            {
                QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 1);                        //进-------
            }   

            // KEY使能，且充电机故障标志位未置位
            if (g_Input.Output.DIN_bits.bit.KEY > 0 && g_SystemState.State.bit.ChargerFaultFlag == 0)
            {		
                QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 1);	//数字事件更新事件，钥匙或分流器连接状态
                
                g_SystemState.State.bit.KEYState = g_Input.Output.DIN_bits.bit.KEY;	//	更新系统状态：钥匙闭合
            }                   
            
            status = Q_TRAN(&AO_BMS_Normal);            		// 跳转至BMS正常工作状态
        }
        break;
		
        case ENTER_BOOTLOADER_SIG:                      		// 进入Bootloader事件
        {
            status = Q_TRAN(&AO_BMS_EnterBootloader);  
        }
        break;
        
        case FAULT_SIG:
        {
            status = Q_TRAN(&AO_BMS_Fault);  
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// 父状态为top状态       
        }
        break;
    }
	
    return status;
}

// ------------------------------- 2.BMS正常工作状态 ----------------------------------
QState AO_BMS_Normal(AO_BMS * const me) {
    
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
            // 默认跳转至空闲状态
            status = Q_TRAN(&AO_BMS_Idle);      
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case FAULT_SIG:     //CANCommEnable                                  //故障事件
        {
            status = Q_TRAN(&AO_BMS_Fault);  
        }
        break;
        
        case ENTER_BOOTLOADER_SIG:                      		// 进入Bootloader事件
        {
            status = Q_TRAN(&AO_BMS_EnterBootloader);  
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// 父状态为top状态       
        }
        break;
    }
    
    return status;
}

// ------------------------------- 3.BMS空闲状态，关闭输出 ----------------------------------
QState AO_BMS_Idle(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(3);
#endif
            
            me->State.OutputAllow = 0;                          // 电池输出总开关清除，输出软件控制标志
            
            // 更新充放电mosfet控制        
            g_AO_SH36730x0.State.CHGControl = 0;          		// BQ769x0关闭充电MOSFET
            g_AO_SH36730x0.State.DSGControl = 0;          		// BQ769x0关闭放电MOSFET
            
            // 每次进入空闲状态时，检查是否有mcu重启请求，若有，则重启之
            // 参数修改并写入完成后会触发一个mcu重启请求
			// To be update： 再次确认此部分代码是否有用？
            if (g_AO_BMS.State.ChipResetAsk > 0)                // 收到芯片重启请求，重启mcu
            {
                g_AO_BMS.State.ChipResetAsk = 0;
                
                NVIC_SystemReset();                             //软件复位   
            } 

			
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG:
        {
//            // 当额外电池均衡被触发时，持续均衡4H，退出空闲状态时取消
//            if (me->State.BatteryExtraBalance == 1)
//            {
//                me->State.BatteryExtraBalance = 0;
//                
//                status = Q_TRAN(&AO_BMS_ExtraBalance);      	// 跳转至空闲额外均流状态
////                status = Q_TRAN(&AO_BMS_SleepDelay);            // 跳转至空闲睡眠状态
//            }
//            else
//            {
                status = Q_TRAN(&AO_BMS_SleepDelay);            // 跳转至空闲睡眠状态                            
//            }       
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case DIN_SIG:                                         //数字信号输入事件
        {
            switch ((u16)Q_PAR(me))    //#define Q_PARAM_SIZE 
            {
                case 1:         								// 开启充放电回路
                {   
                    //status = Q_TRAN(&AO_BMS_On);            	// 跳转至BMS正常状态
                    status=Q_TRAN(&AO_BMS_OnDelay);             //跳转至BMS延迟         //进
                }
                break;
                
                default:
                {                 
                    status = Q_HANDLED();
                }
                break;
            }
        }
        break;
        
        case BMS_UPDATE_SIG:                					// BQ769芯片采样值更新事件
        {                   
            // 更新电量
            BatteryCapacity_Update(me); //进--------没报故障
            
            status = Q_HANDLED();
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_BMS_Normal);               	// 父状态为Normal状态       
        }
        break;
    }
    
    return status;
}

// ------------------------------- 32.BMS空闲时额外均流状态 ----------------------------------
QState AO_BMS_ExtraBalance(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(32);
#endif
            
            g_AO_SH36730x0.Parameter.BallanceErrVoltage = 5;      // 超过5mV电流均衡开启
			
            g_AO_SH36730x0.State.BatteryBalanceEnable = 1;        // 开启均衡             
            
            me->Variable.ExtraBallanceCount = 0;				  // 额外均流次数清零
            
			// 定时器3时基为1s，注意与其它定时器区分
			// 额外均流设计持续4H，时间较长，每次定时器设定为60s，连续执行240次
            QActive_armX((QActive *)me, 3U, 60U);       		// 开启定时器3，60s

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 3U);         		// 关闭定时器3
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT3_SIG:                        			// 额外均流延时定时器，60s触发一次
        {
            status = Q_HANDLED();
            
            // 电池额外均衡持续时间，默认4H
            me->Variable.ExtraBallanceCount++;
            
            if (me->Variable.ExtraBallanceCount < 240)			// 均流时间未超过4H
            {                               
                // 额外均流时间未到，检测单体电压差，若小于5mV则退出额外均流
                // 若最低单体电压低于3.3V，强制停止补充均衡
                if ((g_AO_SH36730x0.Output.SingleMaxVoltage - g_AO_SH36730x0.Output.SingleMinVoltage) <= g_AO_SH36730x0.Parameter.BallanceErrVoltage
                    || g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4)
                {
                    status = Q_TRAN(&AO_BMS_SleepDelay);        // 跳转至空闲睡眠状态
                }
                else
                {
                    QActive_armX((QActive *)me, 3U, 60U);       // 开启定时器0，60s
                    
                    status = Q_HANDLED();
                }               
            }
            else
            {
				// 已连续均流4H，强制结束均流
                status = Q_TRAN(&AO_BMS_SleepDelay);            // 跳转至空闲睡眠状态
            }
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_BMS_Idle);                 	// 父状态为Idle状态       
        }
        break;
    }
	
    return status;
}

// ------------------------------- 33. 空闲睡眠状态 ----------------------------------
// 此状态为最低功耗状态，可考虑在此增加待机功能
QState AO_BMS_SleepDelay(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(33); 
#endif
            
            g_AO_SH36730x0.State.BatteryBalanceEnable = 0;        // 关闭均衡 
//=========================================================时间计算有点奇怪，SleepDlayLong始终为0================================================================			
                                                           //这样的话就会造成，如果有过额外均衡，进入到这时会立即触发定时器3进入低功耗
			                                               //否则就会延时600s后进入低功耗
			//均衡最长4小时 均衡次数每60s加1 最大240次     //进入低功耗前长时间延时 单位H
            if (me->Variable.ExtraBallanceCount > g_SystemParameter.BMS.Output.SleepDelayLong * 60) //g_SystemParameter.BMS.Output.SleepDelay
            {
                  //QActive_armX((QActive *)me, 3U, 30);//测试
                  QActive_armX((QActive *)me, 3U, g_SystemParameter.BMS.Output.SleepDelay); //开启定时器3 倒计时进入低功耗
            }
            else
            {    
                 //QActive_armX((QActive *)me, 3U, 10);//测试
                // 默认延时600s，进入睡眠                                                                                                                    //如果进入else说明ExtraBallanceCount=0 时间是固定的600s
                QActive_armX((QActive *)me, 3U, (g_SystemParameter.BMS.Output.SleepDelay + (u32)(g_SystemParameter.BMS.Output.SleepDelayLong) * 3600 - (u32)(me->Variable.ExtraBallanceCount) * 60));       // 开启定时器3，默认600s                    
            }
//=============================================================================================================================================================
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 3U);         		// 关闭定时器3
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT3_SIG:                                	// 定时器3被触发，启动进入SHIP
        {   
        //  QActive_armX((QActive *)me, 3U, 1U);        		// 开启定时器3，1s
            
            g_AO_SH36730x0.State.EnterSHIPModeAsk = 1;        	// 请求进入SHIP低功耗模式 
            
            status = Q_HANDLED();
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_BMS_Idle);                 	// 父状态为Idle状态       
        }
        break;
    }
    
    return status;
}

// ------------------------- 34.正常输出前延时状态，关闭输出 ----------------------------
QState AO_BMS_OnDelay(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(34);
#endif
            
            g_SystemMonitor.System.Reverse3.Data[15]++;
            
            me->State.OutputAllow = 0;                          // 电池输出总开关清除
            
            // 更新充放电mosfet控制        
            g_AO_SH36730x0.State.CHGControl = 0;          		// BQ769x0关闭充电MOSFET
            g_AO_SH36730x0.State.DSGControl = 0;          		// BQ769x0关闭放电MOSFET
            
           // me->Output.DischargeTimeOnce = 0;                   // 清除本次放电累计时间
            
            // 若输出延时为0s，需手动设置定时器时间为1（1ms后触发），定时器时间设置为0则无法启动定时器。
            if (g_SystemParameter.BMS.Output.OutputDelay > 0)
            {            
                QActive_armX((QActive *)me, 0U, g_SystemParameter.BMS.Output.OutputDelay);     		// 开启定时器0，1s
			}
            else
            {
                QActive_armX((QActive *)me, 0U, 1U);     		// 开启定时器0，1ms,立刻触发
            }
            
            status = Q_HANDLED();           
        }
        break;       
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {           
            status = Q_TRAN(&AO_BMS_On);            	// 跳转至BMS正常状态
        }
        break;
        
        case DIN_SIG:                                  //数字输入更新事件
        {
            switch ((u16)Q_PAR(me))//Q_PAR(me) 就是  QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 1);	最后一个参数
            {
                case 0:         								// 开启充放电回路
                {   
                    status = Q_TRAN(&AO_BMS_Idle);            	// 跳转至BMS空闲状态//钥匙断开，最终进入额外均衡或者等待睡眠
                }
                break;
                
              /*  case 2:         								// 开启充电回路
                {   
                //    status = Q_TRAN(&AO_BMS_On);            	// 跳转至BMS正常状态
                    status = Q_TRAN(&AO_BMS_OnChargeDelay);     // 跳转至BMS充电前延时状态
                }
                break;   */             
                
                default:
                {                 
                    status = Q_HANDLED();
                }
                break;
            }
        }
        break;
        
        case BMS_UPDATE_SIG:                					// BQ769芯片采样值更新事件
        {                   
            // 更新电量
            BatteryCapacity_Update(me);      //进-----------
            
            status = Q_HANDLED();
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_BMS_Normal);               	// 父状态为Normal状态       
        }
        break;
    }
    
    return status;
}


// ------------------------------- 31.输出状态 ----------------------------------
// 收到钥匙开启信号，开启放电回路，等待一定时间，若无通信握手成功，则关闭输出。
// 若握手成功，根据握手信息判断控制器还是充电机，开启相应工作模式
// 无CAN通信模式时，此状态即正常工作状态
// BMS_ON即电池输出状态，此状态可进一步细分多个子状态，比如预放电，接触器闭合，充电，放电等
u8 Restart_balancing=0;//
extern u16 UV_RECHARGE;
//u8 uv_flag=0;//欠压标志位
QState AO_BMS_On(AO_BMS * const me) {
    
    QState status;

    s32 l_data = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(31);
#endif          
           
            me->State.OutputAllow = 1;                          // 电池输出总开关使能
            g_AO_BMS.Output.StartSOC = g_AO_BMS.Output.SOC;
            me->Variable.AutoCutoffDelay = 0;					// 自动切断输出延时清零
            
            // 进入充电时，先记录剩余电量，作为本次充放电电量计算用
            me->Variable.ChargeStartCapacity = me->Output.BatteryCapacity;
            
#if (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2) // 新昕滑板BMS带预放电功能
            
			Output_PreDischarge_Update(&g_Output, ENABLE);  	// 开启预放电
		
			g_AO_BQ769x0.State.CHGControl = 0;          		// BQ769x0关闭充电MOSFET
			g_AO_BQ769x0.State.DSGControl = 0;          		// BQ769x0关闭放电MOSFET            
		
			// 开启定时器0，默认1s，超时后关闭预放电，打开主回路
			QActive_armX((QActive *)me, 0U, g_SystemParameter.BMS.Output.PreDischargeTime);     
		
#else                       									// 不带预放电功能，直接打开充放电回路
            
			// 更新充放电mosfet控制        
			g_AO_SH36730x0.State.CHGControl = 1;          		// BQ769x0开启充电MOSFET
			g_AO_SH36730x0.State.DSGControl = 1;          		// BQ769x0开启放电MOSFET
			
#endif

			// To be update：此参数考虑通过参数配置
            g_AO_SH36730x0.Parameter.BallanceErrVoltage = 2000;     	// 充放电时超过8mV电流均衡开启
            g_AO_SH36730x0.State.BatteryBalanceEnable = 1;        // 开启均衡
     
     
            me->Variable.dsg_limit_cnt = 0;
            me->Variable.dsg_cnt = 0;
            me->Variable.chg_cnt = 0;

            // 定时器1作为1s定时时基用
            QActive_armX((QActive *)me, 1U, 1000U);     		// 开启定时器1，1s
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时器0
            QActive_disarmX((QActive *)me, 1U);         		// 关闭定时器1

#if (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)          // 新昕滑板BMS带预放电功能
        			
            Output_PreDischarge_Update(&g_Output, DISABLE);     // 关闭预放电

#endif
            
            me->State.OutputAllow = 0;                          // 电池输出总开关清除
            
            // 退出充电时，计算本次充放电电量，并计算循环次数
			// 正值为充电，负值为放电
			// To be update：此变量加入monitor
            me->Output.ChargeOrDischargeCapacity = me->Output.BatteryCapacity - me->Variable.ChargeStartCapacity;
            
            if (me->Output.ChargeOrDischargeCapacity > (me->Parameter.BatteryDesignCapacity / 2))
            {
                me->Output.CircleNumberAdd = 10;        		// 循环次数加1，单位为0.1次
            }
            else if (me->Output.ChargeOrDischargeCapacity > (me->Parameter.BatteryDesignCapacity / 5))
            {
                me->Output.CircleNumberAdd = 5;     			// 循环次数加0.5，单位为0.1次
            }
            
            g_AO_BMS.Variable.LowCapCutoffCnt = 0;
            g_AO_BMS.Variable.DischargeFilter = 0;
            
            status = Q_HANDLED();
        }
        break;
 
#if (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)          // 新昕滑板BMS带预放电功能
            		
        case Q_TIMEOUT_SIG:         // 定时器0用于预放电控制逻辑
        {
            // 预放电控制分为2步：
            // Step1：开启预放电，延时1s或规定时间后开启充放电回路
            // Step2：再延时1s，关闭预放电
            if (g_AO_BQ769x0.State.DSGControl == 0)
            {
                // Step1：开启预放电，延时规定时间后开启充放电回路
                QActive_armX((QActive *)me, 0U, 1000U);     	// 开启定时器0，1s，超时后关闭预放电，打开主回路

                // 更新充放电mosfet控制        
                g_AO_BQ769x0.State.CHGControl = 1;          	// BQ769x0开启充电MOSFET
                g_AO_BQ769x0.State.DSGControl = 1;          	// BQ769x0开启放电MOSFET                    
            }
            else
            {
                // Step2：再延时1s，关闭预放电                
                Output_PreDischarge_Update(&g_Output, DISABLE); 

				// 重新尝试CAN模块初始化
				CAN_Config(&g_CAN);
            }

            status = Q_HANDLED();           
        }
        break;
 
#endif						// (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)
		
        case Q_TIMEOUT1_SIG:            						// 定时器1为1s时基定时器
        {
            status = Q_HANDLED(); 
            
            QActive_armX((QActive *)me, 1U, 1000U);     		// 开启定时器1，1s
			
//=================================================================================================================			
//			if(Restart_balancing++>10 && (g_AO_SH36730x0.Output.BatteryCurrent) >-2000)//放电电流超过2A不开启均衡，放电电流持续检测50次符合条件开启均衡标志
//			{	
//			Restart_balancing=0;
//            g_AO_SH36730x0.State.BatteryBalanceEnable = 1;        // 开启均衡
//            }
//			else
//			{
//			Restart_balancing=0;
//			}
			
//===================================================================================================================           
            // 当处于输出状态，且电流不大于100mA时，电池放电累计时间++
            if (g_AO_SH36730x0.Output.BatteryCurrent < 100)
            {
                g_SystemRecord.DischargeTime++;                 // 累计放电时间+1s，单位1s
            }      
			
//			if (g_AO_SH36730x0.Output.BatteryCurrent > 100
//				&& g_AO_SH36730x0.Output.SingleMaxVoltage > 4050)
//			{
//				// 充电接近最终阶段时，主动均流使能电压强制变小。
//				// 判断条件，电池处于充电阶段，且最大单体电压高于4.05V
//				g_AO_SH36730x0.Parameter.BallanceErrVoltage = 3;     	// 充放电时超过3mV电流均衡开启
//			}
//			else
//			{
//				// 正常情况下，主动均流开启电压为8mV，不建议超过10mV
//				g_AO_SH36730x0.Parameter.BallanceErrVoltage = 8;     	// 充放电时超过8mV电流均衡开启
//			}
            //充电检测，每1s检测一次
            if(g_AO_SH36730x0.Output.BatteryCurrent > 1500 || g_SystemState.State.bit.ChargeTempError > 0)//充电温度限制
            {
                
                if(g_AO_BMS.Variable.ChargeCheckCnt > 20)        //充电判定时间暂定为5s
                {
                    g_SystemState.State.bit.ChargeOnFlag = 1;
                } else g_AO_BMS.Variable.ChargeCheckCnt++;
            }
            else
            {
                if(g_AO_BMS.Variable.ChargeCheckCnt)g_AO_BMS.Variable.ChargeCheckCnt--;
                else g_SystemState.State.bit.ChargeOnFlag = 0;
            }
			
            if(g_SystemState.State.bit.ChargeTempError > 0)
            {
                g_AO_SH36730x0.State.CHGControl = 0;          		// BQ769x0开启充电MOSFET
                g_AO_SH36730x0.State.DSGControl = 0;          		// BQ769x0开启放电MOSFET
            }
//==========================================================================================================================
//			else if(uv_flag==1)//欠压充电时关闭 放电MOS打开充电MOS
//			{
//			    g_AO_SH36730x0.State.CHGControl = 1;          		//		
//                g_AO_SH36730x0.State.DSGControl = 0;          		// 
//			}
//==========================================================================================================================
            else
            {
                g_AO_SH36730x0.State.CHGControl = 1;          		// BQ769x0开启放电MOSFET		
                g_AO_SH36730x0.State.DSGControl = 1;          		// BQ769x0开启放电MOSFET
            }
            
            me->Variable.tc_voltage = get_temp_current_v(g_AO_BMS.Output.BatteryTemperatureHi, g_AO_SH36730x0.Output.BatteryCurrent);
            
            
            if(me->Variable.dsg_limit_flg && g_AO_SH36730x0.Output.BatteryCurrent < -2000)
            {
                me->Variable.dsg_limit_cnt2++;
                if(me->Variable.dsg_limit_cnt2 > 1)
                {
                    me->Variable.dsg_limit_cnt2 = 0;
                    me->Output.SOC = 0;
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            }
            
            /// 充放电停止处理
            if(g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage - me->Variable.tc_voltage) /// 2.8
            {
                me->Variable.dsg_cnt++;
                me->Variable.dsg_limit_cnt++;
                if(me->Variable.dsg_limit_cnt > ((g_SystemState.State.bit.ChargeOnFlag || g_AO_SH36730x0.Output.BatteryCurrent > -1000)?300:10))
                {
                    me->Variable.dsg_limit_cnt = 0;
                    me->Output.SOC = 50;
                    me->Variable.dsg_limit_flg = 1;
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            } else if (g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.Discharge.DischargeStopVoltage - me->Variable.tc_voltage && g_SystemState.State.bit.ChargeOnFlag == 0) /// 3.1
            {
                if(me->Variable.dsg_limit_cnt)me->Variable.dsg_limit_cnt--;
                me->Variable.dsg_cnt++;
                if(me->Variable.dsg_cnt > 60)
                {
                    me->Variable.dsg_cnt = 0;
                    me->Output.SOC = 150;
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            } else {
                me->Variable.dsg_limit_cnt = 0;
                if(me->Variable.dsg_cnt)me->Variable.dsg_cnt--;
                me->Variable.dsg_limit_flg = 0;
            }
            
            
            if (g_AO_SH36730x0.Output.SingleMaxVoltage > g_SystemParameter.BMS.Battery.CellChargeStopVoltage) /// 3.6
            {
                me->Variable.chg_cnt++;
                if(me->Variable.chg_cnt > 10)
                {
                    me->Output.SOC = 900;
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            } else {
                me->Variable.chg_cnt = 0;
            }
            
            
            /// 自动关机
            // 当放电电流小于设定值且超过设定时间，强制结束放电  充放电电流小于600ma 循环900次进入空闲
            // 当参数设置自动切断电流不为0时，方起作用
            if (g_SystemParameter.BMS.Output.AutoCutoffCurrent > 0
                && g_AO_SH36730x0.Output.BatteryCurrent < g_SystemParameter.BMS.Output.AutoCutoffCurrent
                && g_AO_SH36730x0.Output.BatteryCurrent > -g_SystemParameter.BMS.Output.AutoCutoffCurrent)
            {
                me->Variable.AutoCutoffDelay++;
                
                if (me->Variable.AutoCutoffDelay >= g_SystemParameter.BMS.Output.AutoCutoffDelay
                    && g_SystemState.State.bit.ChargeOnFlag == 0)
                {                           
                    status = Q_TRAN(&AO_BMS_Idle); 		// 跳转至BMS空闲状态                           
                }
                else if(me->Variable.AutoCutoffDelay >= g_SystemParameter.BMS.Output.AutoCutoffDelay * 8)
                {
                    status = Q_TRAN(&AO_BMS_Idle); 		// 跳转至BMS空闲状态  
                }
            }
            else
            {
                me->Variable.AutoCutoffDelay = 0;
            }
            
            
            
            
            
            
			/*
//			if(g_AO_SH36730x0.Output.SingleMinVoltage>g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage)//欠压充电到2.55关闭标志
//			{
//               uv_flag=0;
//			
//			}
				
			// Take care：本事件处理较为复杂，必须先设置默认跳转条件
//            if(g_SystemState.State.bit.ChargeOnFlag > 0)
//            {
//                g_AO_BMS.Output.StartSOC = 0;
//                g_AO_BMS.Variable.LowCapCutoffCnt = 0;
//            }
//            else
//            {                 
//                if(g_AO_BMS.Output.StartSOC == 0)
//                {
//                    if(g_AO_BMS.Output.SOC > 110)
//                    {
//                        g_AO_BMS.Output.StartSOC = 0;
//                    }
//                    else if(g_AO_BMS.Output.SOC >= 50)
//                    {
//                        g_AO_BMS.Output.StartSOC = g_AO_BMS.Output.SOC;
//                        g_AO_BMS.Parameter.LowSOCCutOffTime = (70 - (110-g_AO_BMS.Output.SOC));
//                    }
//                    else if(g_AO_BMS.Output.SOC > 0)
//                    {
//                        g_AO_BMS.Output.StartSOC = g_AO_BMS.Output.SOC;;
//                        g_AO_BMS.Parameter.LowSOCCutOffTime = 20;
//                    }
//                    else
//                    {
//                        g_AO_BMS.Output.StartSOC = 1;
//                        g_AO_BMS.Parameter.LowSOCCutOffTime = 20;
//                    }
//                    g_AO_BMS.Variable.LowCapCutoffCnt = 0;
//                }
//                else
//                {
//                    g_AO_BMS.Variable.LowCapCutoffCnt ++;                    
//                    if(((g_AO_BMS.Output.StartSOC - g_AO_BMS.Output.SOC) > 10) 
//                        || (g_AO_BMS.Variable.LowCapCutoffCnt > g_AO_BMS.Parameter.LowSOCCutOffTime))
//                    {
//                        g_AO_BMS.Output.StartSOC = 0;  //需要重新更新原始SOC，因为一般BMS不会断电重启
//                        g_AO_BMS.Variable.LowCapCutoffCnt = 0;             //清除时间计时参数
//                        me->State.OutputAllow = 0;                          // 电池输出总开关清除            
//                        status = Q_TRAN(AO_BMS_LowPower);
//                    }
//                }
//            }
            if(g_AO_BMS.Output.SOC > g_AO_BMS.Output.StartSOC) g_AO_BMS.Output.StartSOC = g_AO_BMS.Output.SOC;//小于15%每下降2%断开一次输出
            
            if(g_AO_BMS.Output.SOC <= 150 && !g_SystemState.State.bit.ChargeOnFlag)
            {
                g_AO_BMS.Variable.LowCapCutoffCnt++;
                if(g_AO_BMS.Output.StartSOC >= g_AO_BMS.Output.SOC + 10 || g_AO_BMS.Variable.LowCapCutoffCnt > 300)
                {
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            } 
			else g_AO_BMS.Variable.LowCapCutoffCnt = 0;
            
            if(g_AO_BMS.Output.SOC < 100  && g_AO_SH36730x0.Output.BatteryCurrent <= -2000)//SOC小于10%禁止放电进入空闲状态
            {
                g_AO_BMS.Variable.DischargeFilter++;
                if(g_AO_BMS.Variable.DischargeFilter)
                    status = Q_TRAN(AO_BMS_Idle);
            } else g_AO_BMS.Variable.DischargeFilter = 0;
            
            

            // 当放电时，若电压相差大于15mV，则开启额外均流
            // Take care: 连续超过5s，才会触发
            if ((g_AO_SH36730x0.Output.SingleMaxVoltage - g_AO_SH36730x0.Output.SingleMinVoltage) > 15)//15修改为20调试========================
            {
                me->Variable.BatteryExtraBalanceDelay++;
                if (me->Variable.BatteryExtraBalanceDelay >= 5)
                {
                    me->Variable.BatteryExtraBalanceDelay = 0;
                    me->State.BatteryExtraBalance = 1;   		// 使能额外均流，在进入空闲状态时会自动额外均流
                }
            }
            else
            {
                me->Variable.BatteryExtraBalanceDelay = 0;
            }
            
            // 电池电量校准，主要解决放电至电量f较低时，电量计算偏高问题
            BatteryCapacity_Calibrate(me);

			// 充放电结束条件判断
			// 1：单体最低电压低于2.55V且持续10s以上,且未处于充电状态
			// 2：单体最低电压低于3.0V且放电电流大于100mA且持续5s以上
			// 3：单体最高电压高于4.21V
			// 4：单体最高电压高于充电结束电压（默认4.18V），且充电电流大于100mA且持续5s以上
			// 5：其它结束条件，具体看下面代码                                                 //2.55
            if (g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage 
                && g_SystemState.State.bit.ChargeOnFlag  == 0 && g_AO_SH36730x0.Output.BatteryCurrent >-1000) 	// 放电截止电压为2.6V   ChargeOnFlag 充电开启标志 1s检测一次充电电流大于1A切持续时间5s
            {
                // 单体电压低于2.55V，且持续10s以上，认为电池无电，直接关闭
                // 此状态下无法再开机，必须对电池进行充电后才能使用
                // 若电压高于2.55V，允许开机，但不允许有放电电流，若接上充电机再开机，可以正常为电池补充电量。
				
				if(g_AO_SH36730x0.Output.BatteryCurrent <1500)
			{
                me->Variable.ShutdownDelay++;
                if (me->Variable.ShutdownDelay >= 60)     		// 连续3次电压低于2.7V，认为电压确实过低，约10s
                {
                    me->Variable.ShutdownDelay = 0;
                    
                    me->Output.BatteryCapacity = 0;     		// 单体电压低于2.7V，认为电量为0                    
																
                 //   g_SystemMonitor.System.Reverse2.Data[2]++;
    
                    status = Q_TRAN(&AO_BMS_Idle);          	// 跳转至BMS空闲状态  
					//uv_flag=1;					
                }
				
			}
			
            }
            else if (g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.Discharge.DischargeStopVoltage)	// 放电截止电压为2.7V
            {
                if ((g_AO_SH36730x0.Output.BatteryCurrent <-1000))      // 且大于1A
                {
                    me->Variable.ShutdownDelay++;
                    
                    if (me->Variable.ShutdownDelay >= 15)    	// 连续5次电压低于3.0V，认为电压确实过低，约15s
                    {
                        me->Variable.ShutdownDelay = 0;
                        
                        // 此处考虑更新算法，单体欠压时，直接将电量归零并不合理
                        // 例：低温下放电，电量剩余较多时即触发单体欠压，此时剩余电量是无法放出，而非没有。
                        me->Output.BatteryCapacity = 0;     	// 单体电压低于3.0V，认为电量为0                    
                        
                    //    g_SystemMonitor.System.Reverse2.Data[3]++;
                        
						
                      status = Q_TRAN(&AO_BMS_Idle);      	// 跳转至BMS空闲状态
                    }
                }
                else
                {
                    me->Variable.ShutdownDelay = 0;            //过压欠压延时
                }
            }
            else if (g_AO_SH36730x0.Output.SingleMaxVoltage > (g_SystemParameter.BMS.Battery.CellOverVoltage - 30))	// 测试中充电截止电压为3.7V
            {
                me->Variable.ShutdownDelay = 0;
                
                // 设计1000次循环后寿命降低20%
                // g_SystemRecord.CircleNumber即充放电次数，单位0.1次，LifeCalibrateCircleNumber为补偿用充放电次数，单位0.1次
                l_data = (me->Parameter.BatteryFullCapacity - (me->Parameter.BatteryFullCapacity / 50 * (g_SystemRecord.CircleNumber + g_SystemRecord.LifeCalibrateCircleNumber) / 1000));
                                
                if (me->Output.BatteryCapacity < l_data)
                {       
                    // 即AH法统计电量比设计电量小1%，补偿10次，即损失0.2%的电量
                    // LifeCalibrateCircleNumberAdd单位0.1次
                    // 首先计算电流偏小比例，单位1%
                    if (((l_data - me->Output.BatteryCapacity) * 100 / me->Output.BatteryCapacity) <= 5)            // 电量偏小5%
                    {
                        // 设计1000次循环后寿命降低20%
                        // 设计电量偏小5%，补偿次数增加5.0次，即最大电量减少0.1%
                        me->Output.LifeCalibrateCircleNumberAdd = (l_data - me->Output.BatteryCapacity) * 1000 / me->Output.BatteryCapacity;
                    }
                    else                    					// 电量偏小超过5%，设为5%
                    {
                        // 设计电量偏小5%，补偿次数增加5次，即最大电量减少0.1%
                        me->Output.LifeCalibrateCircleNumberAdd = 50;
                    }   
                    
                    me->Output.BatteryCapacity = l_data;                                   
                }
				
				me->Output.FullCapacity = me->Output.BatteryCapacity;	// 充满电量为实际电量                 
                me->Output.SOC = 1000;									// SOC重置为100.0%
                
                // 单体电压低于3V或高于4.2V，则停止放电
            //    g_SystemMonitor.System.Reverse2.Data[4]++;
                
                status = Q_TRAN(&AO_BMS_Idle);              	// 跳转至BMS空闲状态
            }           
            else if (g_AO_SH36730x0.Output.SingleMaxVoltage > g_SystemParameter.BMS.Battery.CellChargeStopVoltage)        // 测试中充电截止电压为3.7V
            {
                // 注意充满电时，若大电流制动，可能导致短时间超过阈值，不应该触发充电充满关机逻辑
                if (g_AO_SH36730x0.Output.BatteryCurrent > 100)   // 充电电流大于100mA
                {
                    me->Variable.ShutdownDelay++;
                    if (me->Variable.ShutdownDelay >= g_SystemParameter.BMS.Battery.ChargeStopDelay)     	// 延时5s
                    {
                        me->Variable.ShutdownDelay = 0;                      
                        
                        // 设计1000次循环后寿命降低20%
                        // g_SystemRecord.CircleNumber即充放电次数，单位0.1次，LifeCalibrateCircleNumber为补偿用充放电次数，单位0.1次
                        l_data = (me->Parameter.BatteryFullCapacity - (me->Parameter.BatteryFullCapacity / 50 * (g_SystemRecord.CircleNumber + g_SystemRecord.LifeCalibrateCircleNumber) / 1000));
                    
                        if (me->Output.BatteryCapacity < l_data)
                        {       
                            // 即AH法统计电量比设计电量小1%，补偿10次，即损失0.2%的电量
                            // LifeCalibrateCircleNumberAdd单位0.1次
                            // 首先计算电流偏小比例，单位1%
                            if (((l_data - me->Output.BatteryCapacity) * 100 / me->Output.BatteryCapacity) <= 5)            // 电量偏小5%
                            {
                                // 设计1000次循环后寿命降低20%
                                // 设计电量偏小5%，补偿次数增加5次，即最大电量减少0.1%
                                me->Output.LifeCalibrateCircleNumberAdd = (l_data - me->Output.BatteryCapacity) * 1000 / me->Output.BatteryCapacity;
                            }
                            else                    			// 电量偏小超过5%，设为5%
                            {
                                // 设计电量偏小5%，补偿次数增加5次，即最大电量减少0.1%
                                me->Output.LifeCalibrateCircleNumberAdd = 50;
                            }       
                            
                            me->Output.BatteryCapacity = l_data;
                        }

                        me->Output.FullCapacity = me->Output.BatteryCapacity;	// 充满电量为实际电量                    
                        me->Output.SOC = 1000;									// SOC重置为100.0%
						
                   //     g_SystemMonitor.System.Reverse2.Data[5]++;  

                        status = Q_TRAN(&AO_BMS_Idle);   		// 跳转至BMS空闲状态
                    }
                }
            }
            else                    // 未出现欠压过压，检查充电电量是否符合结束充电要求
            {
                // 未出现单体过压欠压，则检测电量是否充满   
				// 充电电量大于设计电量，且充电电流大于100mA，且单体最高电压不低于4.1V，且延时10s
                if (me->Output.BatteryCapacity >= me->Parameter.BatteryDesignCapacity
                    && g_AO_SH36730x0.Output.BatteryCurrent > 100
                    && g_AO_SH36730x0.Output.SingleMaxVoltage > g_SystemParameter.BMS.Charge.ChargeFinishMinVoltage)//最高电压高于3.4V
                {               
                    me->Variable.ShutdownDelay++;                 
                    if (me->Variable.ShutdownDelay >= 10)
                    {
                        me->Variable.ShutdownDelay = 0;
				
						// 当以电池容量达到设计容量条件，结束充电时，比较结束时充电电压，越接近终止电压，则越加大电量衰减补偿
						
                        // 设计1000次循环后寿命降低20%
                        // g_SystemRecord.CircleNumber即充放电次数，单位0.1次，LifeCalibrateCircleNumber为补偿用充放电次数，单位0.1次
                        l_data = (me->Parameter.BatteryFullCapacity - (me->Parameter.BatteryFullCapacity / 50 * (g_SystemRecord.CircleNumber + g_SystemRecord.LifeCalibrateCircleNumber) / 1000));
                    
						// 若估算电池可充最大电量已小于实际电量，则不再补偿，都补过头了啊！！！
						// 这边折算个99%，毕竟还能再充进去一点点电
                        if (me->Output.BatteryCapacity < (l_data * 99 / 100))
                        {       
							// 充电结束时电压越接近g_SystemParameter.BMS.Battery.CellChargeStopVoltage，补偿越大，运行补偿范围为-50mV～0V
							// 电压相差50mV,不补偿，相差1mV,补偿4.9次，即最大电量减少约0.1%
							if (g_SystemParameter.BMS.Battery.CellChargeStopVoltage - g_AO_SH36730x0.Output.SingleMaxVoltage > 0
								&& g_SystemParameter.BMS.Battery.CellChargeStopVoltage - g_AO_SH36730x0.Output.SingleMaxVoltage < 50)
							{
								me->Output.LifeCalibrateCircleNumberAdd = 50 - (g_SystemParameter.BMS.Battery.CellChargeStopVoltage - g_AO_SH36730x0.Output.SingleMaxVoltage);
							}							
						}
						
                        me->Output.FullCapacity = me->Output.BatteryCapacity;	// 充满电量为实际电量                    
                        me->Output.SOC = 1000;									// SOC重置为100.0%
                        
                    //    g_SystemMonitor.System.Reverse2.Data[6]++;
                        
                        // 电池电量大于设计值，则停止放电
                        status = Q_TRAN(&AO_BMS_Idle);      	// 跳转至BMS空闲状态
                    }
                }
                else                        
                {					
                    me->Variable.ShutdownDelay = 0;				// 未触发输出切断条件，切断延时清零
                    
                    // 当放电电流小于设定值且超过设定时间，强制结束放电  充放电电流小于600ma 循环900次进入空闲
                    // 当参数设置自动切断电流不为0时，方起作用
                    if (g_SystemParameter.BMS.Output.AutoCutoffCurrent > 0
                        && g_AO_SH36730x0.Output.BatteryCurrent < g_SystemParameter.BMS.Output.AutoCutoffCurrent
                        && g_AO_SH36730x0.Output.BatteryCurrent > -g_SystemParameter.BMS.Output.AutoCutoffCurrent)
                    {
                        me->Variable.AutoCutoffDelay++;
						//me->Variable.AutoCutoffDelay+=100;
						
                        if (me->Variable.AutoCutoffDelay >= g_SystemParameter.BMS.Output.AutoCutoffDelay
                            && g_SystemState.State.bit.ChargeOnFlag == 0)
                        {                           
                        //    g_SystemMonitor.System.Reverse2.Data[7]++;
                            
                            status = Q_TRAN(&AO_BMS_Idle); 		// 跳转至BMS空闲状态                           
                        }
                        else if(me->Variable.AutoCutoffDelay >= g_SystemParameter.BMS.Output.AutoCutoffDelay * 5)
                        {
                            status = Q_TRAN(&AO_BMS_Idle); 		// 跳转至BMS空闲状态  
                        }
                    }
                    else
                    {
                        me->Variable.AutoCutoffDelay = 0;
                    }
                }
            }
            */
        }
        break;
        
        case DIN_SIG:
        {
            switch ((u16)Q_PAR(me))
            {
																
                case 1:         								// 开启充放电回路
                {
					// 滑板A2版开始，输出状态下收到按钮按下信号，则关闭输出
					
#if (CONTROLLER_TARGET == BMS_XINXIN_A2)            

                    status = Q_TRAN(&AO_BMS_Idle);              // 跳转至BMS空闲状态
                    
#else
                    status = Q_HANDLED();
                    
#endif                      // CONTROLLER_TARGET == BMS_XINXIN_A2
					
                }
                break;
                
                default:
                {   
                    // 滑板A2版本开始，按钮按下后启动电池输出，按钮松开不关闭输出，仅当电池空闲时自动关闭输出
					
#if (CONTROLLER_TARGET == BMS_XINXIN_A2)            

                    status = Q_HANDLED();
                    
#else
                    
                //    g_SystemMonitor.System.Reverse2.Data[8]++;
                    
                    status = Q_TRAN(&AO_BMS_Idle);              // 跳转至BMS空闲状态
                    
#endif                  	// CONTROLLER_TARGET == BMS_XINXIN_A2
                    
                }
                break;
            }
        }
        break;
                        
        case BMS_UPDATE_SIG:                					// BMS更新事件，每次数据更新完成发生
        {           
            // 更新电量
//            BatteryCapacity_Update(me); 
            
            status = Q_HANDLED();
        }
        break;
                
        default: 
        {
            status = Q_SUPER(&AO_BMS_Normal);               	// 父状态为Normal状态       
        }
        break;
    }
	
    return status;
}


// ------------------------------- 100. BMS故障状态，试图恢复之 ----------------------------------
QState AO_BMS_Fault(AO_BMS * const me) {
    
    QState status;
	
    u16 i = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(100);
#endif
			
            me->State.OutputAllow = 0;                          // 电池输出总开关清除
            
            me->Variable.FaultSleepDelay = 0;					// 故障状态进入低功耗延时清零
			me->Variable.FaultNoRecoverDelay = 0;				// 故障未恢复延时清零
			
            g_AO_SH36730x0.State.BatteryBalanceEnable = 0;        // 关闭均衡 
//==============================================================================================            

			g_AO_SH36730x0.State.DSGControl = 0;          		    // BQ769x0关闭放电MOSFET   
			g_AO_SH36730x0.State.CHGControl = 0;
     
//==============================================================================================            
            // ------------------------------- 记录故障 ---------------------------------
            // 发现故障记录会写入一个0x00故障码，即无故障，原因待查
            if (Protect_GetFaultCode(&g_Protect) > 0)           // 仅当故障存在时进行故障记录
            {
                // 将现有故障记录后移一个，并填入新故障
                for (i = 127; i > 0; i--)
                {
                    g_SystemFaultHistory.FaultHistory[i].all = g_SystemFaultHistory.FaultHistory[i - 1].all;
                }
                
                g_SystemFaultHistory.FaultHistory[0].bit.FaultCode = Protect_GetFaultCode(&g_Protect);
                g_SystemFaultHistory.FaultHistory[0].bit.TimeStamp = g_SystemRecord.TotalTime / 60;
                
                // 故障次数计数寻找并加1
                for (i = 0; i <= 127; i++)
                {
                    // 当故障码匹配或故障码为0时，填入故障码并加1
                    if (g_SystemFaultHistory.FaultCount[i].bit.FaultCode == Protect_GetFaultCode(&g_Protect)
                        || g_SystemFaultHistory.FaultCount[i].bit.FaultCode == 0x00)
                    {
                        g_SystemFaultHistory.FaultCount[i].bit.FaultCode = Protect_GetFaultCode(&g_Protect);
                        g_SystemFaultHistory.FaultCount[i].bit.FaultCodeCount++;
                        
                        break;
                    }
                }
                
             //   g_SystemMonitor.System.Reverse2.Data[13] = 1;
                
                // 发送故障记录写入事件
                QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, EEPROM_BLOCK_FAULT_HISTORY);
            }           
            
            QActive_armX((QActive *)me, 0U, 500U);     		// 开启定时器0，0.5s

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {   
            status = Q_HANDLED();
            
            QActive_armX((QActive *)me, 0U, 100U);      		// 开启定时器0，0.1s
            
            if (g_Input.Output.DIN_bits.bit.KEY == 0)           // 启动开关断开
            {
                if (Protect_GetFaultCode(&g_Protect) == 0)
                {       
					// 若故障已清除，则回到空闲状态，可再次启动输出
                //    g_SystemMonitor.System.Reverse2.Data[9]++;
                    
                    status = Q_TRAN(&AO_BMS_Idle);              // 跳转至BMS空闲状态
                }
                else
                {
                    // 故障未清除，则清除故障
                    if (Protect_ClearFault(&g_Protect) > 0)		// 故障清除函数，0：清除成功，1：未清除
                    {  
                        g_AO_EEPROM.State.RecordWriteAsk = 1;
						// 1s后故障未恢复，则进入低功耗状态
						if (me->Variable.FaultSleepDelay++ < 10) 	// 0.1s×10
						{
							status = Q_HANDLED();
						}
						else
						{
							me->Variable.FaultSleepDelay = 0;
							
							// 超时1s，自动进入低功耗
							g_AO_SH36730x0.State.EnterSHIPModeAsk = 1;	// 请求进入SHIP低功耗模式
						}
								  
                    }
                    else
                    {
                        // 故障已清除
                     //   g_SystemMonitor.System.Reverse2.Data[10]++;
                        
                        status = Q_TRAN(&AO_BMS_Idle);      	// 跳转至BMS空闲状态
                    }
                }
            }
            else
            {
                me->Variable.FaultSleepDelay = 0;
                
                // 故障发生5min后仍未进行复位，则进入低功耗状态
                // 测试：看看拉高条件下能不能进入低功耗？？？可能不行
                if (me->Variable.FaultNoRecoverDelay++ < 3000)  	// 0.1s×600    
                {
                    status = Q_HANDLED();
                }
                else
                {
                    me->Variable.FaultNoRecoverDelay = 0;
			
					g_AO_SH36730x0.State.EnterSHIPModeAsk = 1;	// 请求进入SHIP低功耗模式
		
                }
            }
        }
        break;
		
        case BMS_UPDATE_SIG:                					// BMS更新事件，每次数据更新完成发生
        {           
            // 更新电量
            BatteryCapacity_Update(me); 
            
            status = Q_HANDLED();
        }
        break;		
        
        case ENTER_BOOTLOADER_SIG:                      		// 进入Bootloader事件
        {
            status = Q_TRAN(&AO_BMS_EnterBootloader);  
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// 父状态为top状态       
        }
        break;
    }
	
    return status;
}

//--------------------------------低电量提醒处理状态-------------------------
/*
QState AO_BMS_LowPower(AO_BMS * const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
#ifdef AO_DEBUG
    t_Record(200);
#endif
            g_AO_BQ769x0.State.BatteryBalanceEnable = 0;        // 关闭均衡 
            
            me->State.OutputAllow = 0;                          // 电池输出总开关清除
            // 更新充放电mosfet控制        
            g_AO_BQ769x0.State.CHGControl = 0;                  // BQ769x0关闭充电MOSFET
            g_AO_BQ769x0.State.DSGControl = 0;                  // BQ769x0关闭放电MOSFET
            g_AO_EEPROM.State.RecordWriteAsk = 1;
        }
        break;
        case Q_INIT_SIG:
        {
            QActive_armX((QActive *)me, 0U, 10);  //每10ms检测一次钥匙开关状态1000*60/10=6000
            QActive_armX((QActive *)me, 3U, 60); 
        }
        break;
        case Q_TIMEOUT_SIG:
        {
            QActive_armX((QActive *)me, 0U, 10);
            if(g_Input.Output.DIN_bits.bit.KEY == 0) 
            {
                g_AO_BQ769x0.State.EnterSHIPModeAsk = 1;            // 请求进入SHIP低功耗模式            
            }
            status = Q_HANDLED();
        }
        break;
        case Q_TIMEOUT3_SIG:
        {
            g_AO_BQ769x0.State.EnterSHIPModeAsk = 1;            // 请求进入SHIP低功耗模式  
        }
        break;  
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// 父状态为top状态       
        }
        break; 
    }
    return status;
}


*/
// ------------------------------- 200. 进入Bootloader延时状态 ----------------------------------
//进入bootloader时注意关闭AFE看门狗
QState AO_BMS_EnterBootloader(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(200);
#endif

            g_AO_SH36730x0.State.BatteryBalanceEnable = 0;        // 关闭均衡 
            
            // 更新充放电mosfet控制        
            g_AO_SH36730x0.State.CHGControl = 0;          		// BQ769x0关闭充电MOSFET
            g_AO_SH36730x0.State.DSGControl = 0;          		// BQ769x0关闭放电MOSFET
            
            QActive_armX((QActive *)me, 0U, 1000U);     		// 开启定时器0，1s

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// 关闭定时器0
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {   
            // 超时1s后重启mcu           
          //  NVIC_SystemReset();   
		  g_AO_SH36730x0.State.EnterSHIPModeAsk =1;//进入低功耗状态
          g_SystemState.State.bit.EnterBootloaderAsk =1;//
        
            
            status = Q_HANDLED();
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// 父状态为top状态       
        }
        break;
    }
    return status;
}
//int AH_count=0;
s32	t_test244[5];
s32 l_ContactorCurrent = 0;
s32 time_IIC=1;
//u8 value=0;
// ------------------------------- 电池电量估算 ---------------------------------
// 每次BQ769x0芯片采样值数据读取完成后执行，约100ms左右执行一次
// 电流精度为1mA，时间精度为1ms，电量精度为1mAH
void BatteryCapacity_Update(AO_BMS * const me)
{
    // --------------------- 根据与上一次数据更新之间时间查，计算本次变化的电池电量 -------------------------
    l_ContactorCurrent = 0;
    if(g_Output.State.bit.FanEnable > 0)
    {
        l_ContactorCurrent += me->Parameter.FanCurrent;   //风扇运行电流
    }
    // Take care： BMSIdleCurrent为BMS自身耗电电量
	// 仅当 ReadValueTimeOld 大于0时计算，抛弃第一次值，受上电初始化影响
	
	time_IIC=g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld;//调试用
	
	if (me->Variable.ReadValueTimeOld >0)
	{
		if (g_AO_SH36730x0.Output.ReadValueTime > me->Variable.ReadValueTimeOld)				
		{
			// 正常运行时，BQ769芯片采样值更新间隔不会大于1000ms，实际100-200ms左右
			if ((g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld) < 1000)//采样时间低于1000ms 
			{ 
				//AH_count++;//--------------------------------------------------------------------------------------------------测试
				//临时变量 单位mAS
				// Take care：BMSIdleCurrent：补偿的BMS自身耗电电流+电池的自放电                        //BMS自身负载电流   风扇电流*采样更新时间  
				me->Variable.BatteryCapacityTemp += (g_AO_SH36730x0.Output.BatteryCurrent - me->Parameter.BMSIdleCurrent - l_ContactorCurrent) * (g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld);
			}
			else//采样时间超过1000ms设置故障
			{
				t_test244[0]++;
				t_test244[1] = g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld;
				
				// Lv0：BQ769芯片采样值更新间隔超过1000ms
				Protect_SetFaultCodeLv0(&g_Protect, BQ769_UPDATE_TOO_SLOW);
				
				me->Variable.BatteryCapacityTemp += (g_AO_SH36730x0.Output.BatteryCurrent - me->Parameter.BMSIdleCurrent) * 500;
			}
		}
		else
		{
			// 仅出现在长时间运行后，ReadValueTime溢出的情况，约需连续运行49天！															
			if (g_AO_SH36730x0.Output.ReadValueTime < 1000)			// 新ReadValueTime < 1000，说明真的是溢出了，厉害啊！   //老铁:双击666
			{
				me->Variable.BatteryCapacityTemp += (g_AO_SH36730x0.Output.BatteryCurrent - me->Parameter.BMSIdleCurrent) * g_AO_SH36730x0.Output.ReadValueTime;
			}
			else
			{
				t_test244[2]++;
				t_test244[3] = g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld;
				
				// 不光溢出，还一下子1000ms都没更新，必须得报错啊
				// Lv0：BQ769芯片采样值更新间隔超过1000ms
				Protect_SetFaultCodeLv0(&g_Protect, BQ769_UPDATE_TOO_SLOW2);//=========
				
				me->Variable.BatteryCapacityTemp += (g_AO_SH36730x0.Output.BatteryCurrent - me->Parameter.BMSIdleCurrent) * 500;
			}
		}
	}
	
    me->Variable.ReadValueTimeOld = g_AO_SH36730x0.Output.ReadValueTime;			// 赋值语句，千万别忘了      //  
    
    
    // ------------------------- 当电量变化大于1mAH时，更新电池电量 -------------------------------
	// BatteryCapacityTemp，电量单位为1mA*ms      
    if (me->Variable.BatteryCapacityTemp >= 3600000)
    {
        // 即增加的电量大于3600mAS，即1mAH
        me->Output.BatteryCapacity += me->Variable.BatteryCapacityTemp / 3600000;
        
        // 限制电量不能超过设计最大电量
        if (me->Output.BatteryCapacity > me->Parameter.BatteryFullCapacity)
        {
            me->Output.BatteryCapacity = me->Parameter.BatteryFullCapacity;
        }

        me->Variable.BatteryCapacityTemp = me->Variable.BatteryCapacityTemp % 3600000;      
    }
    else if (me->Variable.BatteryCapacityTemp <= -3600000)
    {
        // 即减少的电量大于3600mAS，即1mAH
        me->Output.BatteryCapacity += me->Variable.BatteryCapacityTemp / 3600000;
		
		// 电量不可能出现负值
        if (me->Output.BatteryCapacity < 0)
        {
            me->Output.BatteryCapacity = 0;
        }
        
        me->Variable.BatteryCapacityTemp = me->Variable.BatteryCapacityTemp % 3600000;      
    }
    
    // --------------------------------- 计算电量百分比SOC --------------------------------------
	// 此处以充满电量的99%左右为满电量计算值，防止出现一放电即99%现象
	// SOC单位为0.1%
	if(me->Output.FullCapacity > 0)//电池可实际充入电量
    {       //电池容量
        if (me->Output.BatteryCapacity <= me->Output.FullCapacity * 100 / 101)//FullCapacity电池实际可充入电量mAH 的99%作为满电值计算
        {
            me->Output.SOC = me->Output.BatteryCapacity * 1000 / (me->Output.FullCapacity * 100 / 101);//电量百分比1计算
        }
        else
        {
            me->Output.SOC = 1000;
        }
    }
    else
    {
        if(g_SystemRecord.BatteryFullCapacity > 0)
        {
            me->Output.FullCapacity = g_SystemRecord.BatteryFullCapacity;
        }
        me->Output.SOC = 0;
    }
	 
	//TIM_SetCompare2(TIM2,(me->Output.SOC*3636)/1000);//PDI 输出 占空比表示SOC 有一定误差 //此版本BDI功能只是预留，软件不用加
	
	
	
}

s32 l_Voltage[4] = {0, 0, 0, 0};

// -------------------------- 电池电量补偿，运行在1s时基中 --------------------------
// 根据单体电压补偿电量
// 补偿1：当电池放电至电量较低时，电压与剩余电量有一定线性关系，同时与放电电流有关。
void BatteryCapacity_Calibrate(AO_BMS * const me)
{
    
    u16 temp_voltage_value = 0;
    if(g_AO_BMS.Output.BatteryTemperatureHi < 100 && g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode)
    {
        if(g_SystemParameter.BMS.Discharge.dsg_tmp_low_k < 10 || g_SystemParameter.BMS.Discharge.dsg_tmp_low_k > 100)g_SystemParameter.BMS.Discharge.dsg_tmp_low_k = 50;
        temp_voltage_value = g_SystemParameter.BMS.Discharge.dsg_tmp_low_k * (100 - g_AO_BMS.Output.BatteryTemperatureHi) / 50;
    }
    
    // 电量补偿算法
    if (g_AO_SH36730x0.Output.BatteryCurrent < -100)      // 仅当放电时，放电电流大于100mA，进行剩余电量补偿
    {
        /*    // 放电电流超过0.5C时，补偿电压随电流大小而变化
            if (g_AO_BQ769x0.Output.BatteryCurrent < -(s32)(g_SystemRecord.BatteryFullCapacity / 2))
            {
                // 1%电量， 0.5C放电，电压3.1V，1C放电，电压3.0V,电压不得低于3.01V
                l_Voltage[0] = 3100 - (g_AO_BQ769x0.Output.BatteryCurrent + g_SystemRecord.BatteryFullCapacity / 2) * 100 / g_AO_BQ769x0.Output.BatteryCurrent;
                if (l_Voltage[0] < 3100)
                {
                    l_Voltage[0] = 3100;
                }

                // 3%电量，0.5C放电，电压3.2V，1C放电，电压3.1V，1.5C放电，电压3.0V
                l_Voltage[1] = 3200 - (g_AO_BQ769x0.Output.BatteryCurrent + g_SystemRecord.BatteryFullCapacity / 2) * 100 / g_AO_BQ769x0.Output.BatteryCurrent;
                if (l_Voltage[1] < 3150)
                {
                    l_Voltage[1] = 3150;
                }

                // 10%电量，0.5C放电，电压3.3V，1C放电，电压3.2V, 1.5C放电，电压3.1V, 2C放电，电压3.0V
                l_Voltage[2] = 3300 - (g_AO_BQ769x0.Output.BatteryCurrent + g_SystemRecord.BatteryFullCapacity / 2) * 100 / g_AO_BQ769x0.Output.BatteryCurrent;
                if (l_Voltage[2] < 3200)
                {
                    l_Voltage[2] = 3200;
                }

                // 20%电量，0.5C放电，电压3.4V，1C放电，电压3.3V, 1.5C放电，电压3.2V, 2C放电，电压3.1V
                l_Voltage[3] = 3400 - (g_AO_BQ769x0.Output.BatteryCurrent + g_SystemRecord.BatteryFullCapacity / 2) * 100 / g_AO_BQ769x0.Output.BatteryCurrent;
                if (l_Voltage[3] < 3250)
                {
                    l_Voltage[3] = 3250;
                }
            }
            else
            {
                // 放电电流小于0.5C，补偿电压以0.5C放电为准
                l_Voltage[0] = 3100;            // 1%
                l_Voltage[1] = 3200;            // 3%
                l_Voltage[2] = 3300;            // 10%
                l_Voltage[3] = 3400;            // 20%
            }*/

        // 放电电流超过0.5C时，补偿电压随电流大小而变化
        if (g_AO_SH36730x0.Output.BatteryCurrent < -(s32)(g_SystemRecord.BatteryFullCapacity / 5))
        {
            // 放电电流每增加0.5C，校准电压下调100mV，但不允许低于限制值
            // 1%电量， 0.5C放电，电压3.1V，1C放电，电压3.0V,电压不得低于3.01V
            l_Voltage[0] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 + g_AO_SH36730x0.Output.BatteryCurrent * 50 / g_SystemRecord.BatteryFullCapacity - temp_voltage_value;
            if (l_Voltage[0] < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1)
            {
                l_Voltage[0] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1;
            }

            // 3%电量，0.5C放电，电压3.2V，1C放电，电压3.1V，1.5C放电，电压3.0V
            l_Voltage[1] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 + g_AO_SH36730x0.Output.BatteryCurrent * 50 / g_SystemRecord.BatteryFullCapacity - temp_voltage_value;
            if (l_Voltage[1] < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2)
            {
                l_Voltage[1] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2;
            }

            // 10%电量，0.5C放电，电压3.3V，1C放电，电压3.2V, 1.5C放电，电压3.1V, 2C放电，电压3.0V
            l_Voltage[2] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 + g_AO_SH36730x0.Output.BatteryCurrent * 50 / g_SystemRecord.BatteryFullCapacity - temp_voltage_value;
            if (l_Voltage[2] < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3)
            {
                l_Voltage[2] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3;
            }

            // 20%电量，0.5C放电，电压3.4V，1C放电，电压3.3V, 1.5C放电，电压3.2V, 2C放电，电压3.1V
            l_Voltage[3] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 + g_AO_SH36730x0.Output.BatteryCurrent * 50 / g_SystemRecord.BatteryFullCapacity - temp_voltage_value;
            if (l_Voltage[3] < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4)
            {
                l_Voltage[3] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4;
            }
        }
        else
        {
            // 放电电流小于0.5C，补偿电压以0.5C放电为准
            l_Voltage[0] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 - temp_voltage_value;            // 1%
            l_Voltage[1] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 - temp_voltage_value;            // 3%
            l_Voltage[2] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 - temp_voltage_value;            // 10%
            l_Voltage[3] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 - temp_voltage_value;            // 20%
        }

        // For test：放电时根据电压对电量进行补偿,经验参数
        // 当电压低于3.4V左右时，0.5C放电，电压快速下降，此时存储的电量只有设计值的10%
        // 若AH法计算的电量若大于此数值，则缓慢减小剩余电量
        if (g_AO_SH36730x0.Output.SingleMinVoltage < l_Voltage[0] && me->Output.SOC > 10)
        {
            // 电量大于设计电量的1%
//            if (me->Output.BatteryCapacity > (me->Parameter.BatteryDesignCapacity / 100))
            {
                // 每1s补偿1/300电量，即60s补偿约20%电量
                me->Output.BatteryCapacity -= me->Output.FullCapacity / 300;
                if (me->Output.BatteryCapacity < 0)
                {
                    me->Output.BatteryCapacity = 0;
                }
            }
        }
        else if (g_AO_SH36730x0.Output.SingleMinVoltage < l_Voltage[1] && me->Output.SOC > 30)
        {
            // 电量大于设计电量的3.3%
//            if (me->Output.BatteryCapacity > (me->Parameter.BatteryDesignCapacity / 30))
            {
                // 每1s补偿1/500电量，即60s补偿约12%电量
                me->Output.BatteryCapacity -= me->Output.FullCapacity / 500;
                if (me->Output.BatteryCapacity < 0)
                {
                    me->Output.BatteryCapacity = 0;
                }
            }
        }
        else if (g_AO_SH36730x0.Output.SingleMinVoltage < l_Voltage[2] && me->Output.SOC > 100)
        {
            // 电量大于设计电量的10%
//            if (me->Output.BatteryCapacity > (me->Parameter.BatteryDesignCapacity / 10))
            {
                // 每1s补偿1/750电量，即60s补偿约8%电量
                me->Output.BatteryCapacity -= me->Output.FullCapacity / 750;
                if (me->Output.BatteryCapacity < 0)
                {
                    me->Output.BatteryCapacity = 0;
                }
            }
        }
        else if (g_AO_SH36730x0.Output.SingleMinVoltage < l_Voltage[3] && me->Output.SOC > 200)
        {
            // 电量大于设计电量的20%
//            if (me->Output.BatteryCapacity > (me->Parameter.BatteryDesignCapacity / 5))
            {
                // 每1s补偿1/1200电量，即60s补偿约5%电量
                me->Output.BatteryCapacity -= me->Output.FullCapacity / 1200;
                if (me->Output.BatteryCapacity < 0)
                {
                    me->Output.BatteryCapacity = 0;
                }
            }
        }
    }
}


// End of AO_BMS.c


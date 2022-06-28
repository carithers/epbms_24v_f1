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

#include "stm32f0xx.h"                     		// STM32器件寄存器定义头文件
#include "qpn_port.h"                           	// 状态机头文件

// ------------------------------- 全局宏定义 ---------------------------------------
#define	BMS_COMMAND_IDLE			0				// 空闲待机指令
#define	BMS_COMMAND_DISCHARGE		1				// 放电指令
#define	BMS_COMMAND_CHARGE			2				// 充电指令



// -------------------------- 显示模块活动对象结构体 ---------------------------------
struct AO_BMS_State {
	u16	BatteryExtraBalance;				// 电池额外均衡使能，则在电池空闲状态，额外均衡10个小时，若离开空闲状态，则取消
	u16	ChipResetAsk;						// 芯片重启请求，当参数被设置后，会置位此标志位
	u16	CANCommEnable;						// CAN通信使能
    u16 OutputAllow;                        // 电池输出总使能，此位必须置位，方允许输出
};

struct AO_BMS_Parameter {
	s32	BatteryFullCapacity;				// 电池组实际容量，单位mAH
	s32	BatteryDesignCapacity;				// 电池组设计容量，单位mAH
	s32	BMSIdleCurrent;						// BMS自身负载电流，单位mA，用于补偿电量损耗
    s32 FanCurrent;                         // 风扇运行电流，单位mA，用于补偿电流损耗
    u16 LowSOCCutOffTime;                   // 低电量用于接触器切断时间
};

struct AO_BMS_Variable {
	s32	BatteryCapacityTemp;				// 电池容量，临时变量，单位mAS
	s32	ChargeStartCapacity;				// 进入充电状态时，剩余电量。充电结束时比较得出本次充电电量，并计算循环次数
	u32	ReadValueTimeOld;					// 数据读取时间，单位1ms
	u16	ExtraBallanceCount;					// 电池额外均衡，次数，每1H加1
	u16	ShutdownDelay;						// 过压欠压延时
	u16	FaultSleepDelay;					// 故障时延时关闭计数器
	u16	FaultNoRecoverDelay;				// 故障未执行恢复计数器
	u32	AutoCutoffDelay;					// 输出自动切断延时，单位s
	u16	BatteryExtraBalanceDelay;			// 电池额外均流使能延时
	u16	BatteryOutputDelay;					// 电池启动输出延时
	u16 LowCapCutoffCnt;                    // 用于低电量切断计时
    u16 DischargeFilter;
    u16 ChargeCheckCnt;                     // 充电状态判定计时
    u32 dsg_cnt;
    u32 dsg2_cnt;
    u32 dsg_limit_cnt;
    u32 dsg_limit_cnt2;
    u32 chg_cnt;
    u32 dsg_limit_flg;
    s32 tc_voltage;
    u16 dsg_limit_power_flg;
    u16 dsg_limit_power_cnt;
    u16 dsg_limit_power_cnt2;
};

struct AO_BMS_Output {
    s32 BatteryCurrent;                     // 电池电流，单位mA，正为充电，负为放电
    s32 BatteryCurrentFld;                  // 电池电流，低通滤波后，单位mA，正为充电，负为放电
	s32	BatteryCapacity;					// 电池容量，单位mAH
	s32	FullCapacity;						// 电池实际可充入电量，单位mAH，BMS模块赋值
	s32	ChargeOrDischargeCapacity;			// 本次充放电电量，单位mAH，正值为充电，负值为放电
	s32	BatteryTemperature[8];				// 电池温度，单位0.1摄氏度
    s32 BatteryTemperatureHi;               // 电池包最高温度，单位0.1摄氏度
    s32 BatteryTemperatureLow;              // 电池包最低温度，单位0.1摄氏度    
	s32	BMSTemperature[4];					// BMS控制板，单位0.1摄氏度
	s32 BMSTemperatureHi;                   // BMS控制板最高温度，单位0.1摄氏度
	u16	CircleNumberAdd;					// 循环次数增加次数，单位0.1次
	u16	LifeCalibrateCircleNumberAdd;		// 电池循环寿命校准次数，单位0.1次
	u16	SOC;								// 电池电量百分比，单位0.1%
	u16	SingleMinVoltagePointer;			// 单体最高电压对应第几节，1-7
	s16	ChargeCurrentLimit;					// 充电限制电流
    u16 DischargeTimeOnce;                  // 本次放电累计时间
    u16 StartSOC;                           // 小于15%时用于标记SOC，此后电量每降低2%切断一次输出
};

typedef struct AO_BMSTag {                      
    QActive super;							 			// derive from QActive
	struct 	AO_BMS_State		State;				// 模块状态
	struct 	AO_BMS_Parameter	Parameter;			// BQ769x0 芯片参数
	struct	AO_BMS_Variable		Variable;			// 模块临时变量
	struct 	AO_BMS_Output		Output;				// 输出采样值
} AO_BMS;

// ------------------------------ 声明清除函数 ---------------------------------
void AO_BMS_ctor(AO_BMS * const me);

// ------------------------------ 声明活动状态 ---------------------------------
static QState AO_BMS_initial(AO_BMS * const me);							// 初始状态
static QState AO_BMS_StartWait(AO_BMS * const me);						// 1.启动等待状态
static QState AO_BMS_Normal(AO_BMS * const me);							// 2.正常状态
static QState AO_BMS_Idle(AO_BMS * const me);						// 3.空闲状态
static QState AO_BMS_ExtraBalance(AO_BMS * const me);			// 32.额外均流状态
static QState AO_BMS_SleepDelay(AO_BMS * const me);				// 33.睡眠前延时状态
static QState AO_BMS_OnDelay(AO_BMS * const me);                // 34.输出前延时状态
static QState AO_BMS_On(AO_BMS * const me);							// 31.正常输出状态
static QState AO_BMS_Fault(AO_BMS * const me);							// 100.故障状态
static QState AO_BMS_LowPower(AO_BMS * const me);                 // 低电量处理逻辑
static QState AO_BMS_EnterBootloader(AO_BMS * const me);				// 200.进入Bootloader延时状态
// End of AO_BMS.h

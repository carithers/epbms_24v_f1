/* ==================================================================================

 File name:     system_parameter.h
 Originator:    BLJ
 Description:   系统参数结构体头文件，系统参数，储存在EEPROM中

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-28-2015     Version 0.0.1           测试功能
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                     // STM32器件寄存器定义头文件


// 系统信息 0x00-0x0F
struct SYSTEM_PARAMETER_Information {
	u16	DeviceName[8];						// 设备类型编号
	u16	SerialNumber1;						// 序列号1
	u16	SerialNumber2;						// 序列号2	
	u16	ManufactureData;					// 出厂日期	年/星期；例1350，即13年第50周
	u16	HardwareVersion;					// 硬件版本 例：1.01，存储值：101
	u16	SoftwareVersion;					// 软件版本 例：1.01，存储值：101
	u16	PrototalVersion;					// 通信协议版本 例：1.01，存储值：101
	u16	ParameterVersion;					// 参数版本 例：1.01，存储值：101
	u16	rsvd[1];
};

// 校准参数 0x10-0x17
struct SYSTEM_PARAMETER_Calibration {
	u16	CurrentSensorMaxCurrent;			// 电流传感器放大比例，默认100A
	u16 CurrentSensorMaxVoltage;			// 电流传感器放大比例，默认100mV，即1m欧姆采样电阻
	s16	CurrentSensorOffset;				// 电流传感器偏置	一般-255～255之间
	s16	VdcSensorGain;						// 直流电压传感器放大比例
	s16	VdcSensorOffset;					// 直流电压传感器偏置
	s16	InnerDriveSensorGain;			    // 内部驱动电压传感器放大比例，单位0.1kOhm
	s16	InnerDriveSensorOffset;				// 内部驱动电压传感器偏置，单位0.1V
	s16	BMSIdleCurrent;						// BMS自身损耗电流，单位mA	
};

// 测试用参数，不写入EEPROM， 0x18-0x1F
struct SYSTEM_PARAMETER_Test {
	u16	test_BalancePointer;				// 强制指定均流对象，测试用
    u16	test_Shutdown;						// 手动强制电池进入休眠模式
    u16 test_ClearFaultHistory;             // 清除故障记录
    u16 test_ClearSOC;
    u16 test_ChargeReqVol;
	u16	rsvd[3];
};

// 系统信息
struct SYSTEM_PARAMETER_System {
	struct SYSTEM_PARAMETER_Information 	Information;
	struct SYSTEM_PARAMETER_Calibration		Calibration;
    struct SYSTEM_PARAMETER_Test            Test;
};

// 电池参数 0x20-0x2F
struct SYSTEM_PARAMETER_Battery {
	u16	AFESelect;							// AFE芯片选择	0:错误选择；1：BQ76920；2：BQ76930；3：BQ76940；4：BQ76925
	u16 SeriesNumber;						// 电池组串联级数
	u16	FullCapacity;						// 电池实际容量，单位0.1AH
	u16	DesignCapacity;						// 电池组设计容量，单位0.1AH
	u16	CellOverVoltage;					// 单体过压限制值，单位mV
	u16	CellUnderVoltage;					// 单体欠压限制值，单位mV
	u16	ShortCutDownCurrent;				// 电池短路电流,单位A
	u16	ShortCutDownDelay;					// 电池短路延时，单位us
	u16	OverCutDownCurrent;					// 电池过流电流，单位A
	u16	OverCutDownDelay;					// 电池过流延时，单位ms
	u16	CellChargeStopVoltage;				// 单体充电截止电压，单位mV
	u16	ChargeCurrentLimit;					// 电池充电限制电流，单位A
	u16	BatteryTemperatureCheckMode;		// 电池温度检测使能
	u16	OverTemperature;					// 电池温度过高限制，单位摄氏度
	s16	UnderTemperature;					// 电池温度过低限制，单位摄氏度
	u16 ChargeStopDelay;                    // 充电完成延迟，单位s
};



// 输出控制参数 0x30-0x37
struct SYSTEM_PARAMETER_Output {
	s16	AutoCutoffCurrent;					// 输出自动切断电流，单位mA
	u16	AutoCutoffDelay;					// 输出自动切断延时，单位s
	u16	SleepDelay;							// BMS进入低功耗状态前延时，单位s
	u16	OutputDelay;						// 启动输出延时，单位ms
	u16	PreDischargeTime;					// 预放电时间，单位ms	
    u16 SleepDelayLong;                     // BMS进入低功耗状态前长时间延时，单位h，与SleepDelay累加
	u16	rsvd[2];
};

// 主接触器控制相关参数，0x38-0x3F
struct SYSTEM_PARAMETER_Contactor {
	u16 ContactorBaseFrequency;				// 主接触器PWM频率，单位Hz
	u16 ContactorFullPercentTime;			// 接触器启动时全占空比持续时间，单位ms
	u16	ContactorLongLastPercent;			// 接触器长时间工作时占空比，单位0.1%
	u16 rsvd[5];
};


// 对外报错信息设置，0x40-0x47
struct SYSTEM_PARAMETER_Warning {
	u16	BDILowLevel1Percent;				// 电池电量低报警，Lv1，对应电量，单位%
	u16	BDILowLevel2Percent;				// 电池电量严重过低报警，Lv2，对应电量，单位%
	u16	BDILowLimitLift;					// 低电量，限制举升，对应电量单位1%
	u16	BDILowLimitSpeed;					// 低电量，限制车速，对应电量单位1%
	u16	BMSOTemperatureCheckEnable;			// BMS管理板温度检测使能
	u16 BMSOverTemperature;              // BMS限制起升温度，暂定为80摄氏度
    u16 LowSOCBeep;                         // 低于此温度时蜂鸣器开始工作
	u16 rsvd[1];
};


// 电池类型切换，0x48-0x4F
struct SYSTEM_PARAMETER_BatteryType {
	u16	BatteryType;				        // 电池类型，0：三元，1：磷酸铁锂
    u16	BatteryTypeReal;				    // 实际电池类型，用于判断电池类型是否修改，0：三元，1：磷酸铁锂
	u16	rsvd[6];
};

// 充电机控制，0x50-0x57
struct SYSTEM_PARAMETER_Charge {
    u16 ChargeVoltageStep1;                 // 充电电流阶段1，单体最高电压大于此电压，则充电电流由1C变为0.5C，单位1mV
    u16 ChargeVoltageStep2;                 // 充电电流阶段2，单体最高电压大于此电压，则充电电流由0.5C变为0.25C，单位1mV
    u16 ChargeVoltageStep3;                 // 充电电流阶段3，单体最高电压大于此电压，则充电电流由0.25C变为0.125C，单位1mV
    u16 ChargeForceStopVoltage;             // 充电强制结束电压，单位mV
    u16 ChargeFinishMinVoltage;             // 充电完成最小允许电压，单位mV
    u16 ChargeOverCurrent;                  // 充电保护电流，单位1A
    u16 ChargeOverCurrentDelay;             // 充电过流延时，单位1s
	u16	ChargerCommEnable;                  // CAN总线充电机通信使能
};

// 充放电控制 ，0x58-0x5F
struct SYSTEM_PARAMETER_Discharge {
    u16 DischargeForceStopVoltage;          // 单体放电强制结束电压
    u16 DischargeStopVoltage;               // 单体放电结束电压，电流为放电电流，且持续超过5s，结束放电
	// cc_flg-17
	u16 CalibrateCapacity;
    u16 dsg_cc_low_k;
    u16 dsg_tmp_low_k;
    u16 rsvd[3];
	// cc_flg-17
    
};

// 电池电量校准，0x60-0x6F
struct SYSTEM_PARAMETER_CapacityCalibrate {
    u16 CalibrateVoltage1;                  // 校准电压1，对应电量1%
    u16 CalibrateVoltageLowLimit1;          // 校准电压1，最小限制值
    u16 CalibrateVoltage2;                  // 校准电压2，对应电量3.3%
    u16 CalibrateVoltageLowLimit2;          // 校准电压2，最小限制值
    u16 CalibrateVoltage3;                  // 校准电压3，对应电量10%
    u16 CalibrateVoltageLowLimit3;          // 校准电压3，最小限制值
    u16 CalibrateVoltage4;                  // 校准电压4，对应电量20%
    u16 CalibrateVoltageLowLimit4;          // 校准电压4，最小限制值
    u16 rsvd[8];
};

// 风扇控制，0x70-0x77
struct SYSTEM_PARAMETER_Fan {
	u16 FanBaseFrequency;				    // 风扇PWM频率，单位Hz
	u16 FanFullPercentTime;			        // 风扇启动时全占空比持续时间，单位ms
	u16	FanLongLastPercent;			        // 风扇长时间工作时占空比，单位0.1%
    u16 FanEnableTemperature;               // 风扇使能温度，当电池温度高于此温度时，开启风扇
    s16	FanCurrent;		        			// 风扇运行电流，单位mA
	u16 FanFullPercent;                     // 风扇最大工作占空比
    u16 FanMode;                            // 风扇工作模式，0：根据温度使能，1：根据Key使能，2用于低电量提醒
    u16 OutputClsoseDelay;                   // 输出延时，仅在模式1中起作用
};

// 48V60AH电池用通信协议相关,0x78-0x7F
struct SYSTEM_PARAMETER_CAN_Protocal_60AH {
    u16 CANFrameEnable;                     // 此数据帧发送使能
	u16	BDILowStopLift;					    // 低电量，限制举升，对应电量单位1%
	u16	BDILowSpeedRedution1;			    // 低电量，限制车速1，对应电量单位1%
    u16	BDILowSpeedRedution2;			    // 低电量，限制车速1，对应电量单位1%
    u16	BDILowOpenMC;			            // 低电量，限制车速1，对应电量单位1%
    u16 rsvd[3];
};

// 并联电池用参数，0x80-0x87
struct SYSTEM_PARAMETER_Parallel {
    u16 ParallelEnable;                     // 电池并联功能使能
    u16 rsvd[7];
};

// 电流传感器,0x88-0x8F
struct SYSTEM_PARAMETER_CurrentSensor {
    u16 CurrentSensorSelect;                // 电流传感器选择，0：BQ769芯片外接分流器，1：外接单磁霍尔电流传感器,2：外接双磁霍尔电流传感器,
    u16 rsvd[7];
};

// 电池保护相关参数，0x90-0x97
struct SYSTEM_PARAMETER_Protect {
    u16 CellSoftOverVoltage;                    // 单体软件过压电压，单位1mV
    u16 CellSoftOverVoltageDelay;               // 单体软件过压延时，单位1s
    u16 CellHardwareOverVoltageDelay;           // 单体硬件过压延时，单位1s
    u16 ShortCurrentTimeFactor;                 // 电流传感器短路电流限制值放大比例，即真正的短路保护触发延时 = ShortCutDownDelay * ShortCurrentTimeFactor    
    u16 CellUnderVoltageDelay;                  // 电池欠压延时，单位s
    u16 rsvd[3];
};

// 异口充电相关参数,0x98-0x9F
struct SYSTEM_PARAMETER_Charge_GB {
    u16 ChargeMode;                             // 0：同口充放电，1：异口充放电，协议不变，2：异口充放电，国标充电协议
    u16 rsvd[7];
};
// 充电机温度控制，0xA0-0xA7
struct SYSTEM_PARAMETER_Charge2 {
    u16 ChargeLimitTemperature1;            // 充电限制温度1，高于此温度时，限制最大充电电流为0.5C,单位1degC
    u16 ChargeLimitTemperature2;            // 充电限制温度2，高于此温度时，限制最大充电电流为0.25C，单位1degC
    s16 ChargeLimitTemperature3;
    s16 ChargeLimitTemperature4;            // 充电限制温度4，低于此温度时，不允许输出
    u16 ChargeCurrent;                      //充电电流
    u16 rsvd[3];
};
//远程控制参数，0xA8-0xAF
struct SYSTEM_PARAMETER_Remote {
    u16 RemoteEnable;                           // 远程控制使能标志位
    u16 OffLineTime;                            // 网络掉线关闭延时，单位min，默认60min
    u16 rsvd[6];
};
    
// BMS 参数
struct SYSTEM_PARAMETER_BMS {
	struct SYSTEM_PARAMETER_Battery		        Battery;
	struct SYSTEM_PARAMETER_Output		        Output;
	struct SYSTEM_PARAMETER_Contactor	        Contactor;
	struct SYSTEM_PARAMETER_Warning		        Warning;
    struct SYSTEM_PARAMETER_BatteryType         BatteryType;
    struct SYSTEM_PARAMETER_Charge              Charge;
    struct SYSTEM_PARAMETER_Discharge           Discharge;
    struct SYSTEM_PARAMETER_CapacityCalibrate   CapacityCalibrate;
    struct SYSTEM_PARAMETER_Fan                 Fan;
    struct SYSTEM_PARAMETER_CAN_Protocal_60AH   CANProtocal60AH;
    struct SYSTEM_PARAMETER_Parallel            Parallel;
    struct SYSTEM_PARAMETER_CurrentSensor       CurrentSensor;    
    struct SYSTEM_PARAMETER_Protect             Protect;
    struct SYSTEM_PARAMETER_Charge_GB           ChargeGB;           // 国标充电协议，异口充电相关参数
    struct SYSTEM_PARAMETER_Charge2             Charge2;   
    struct SYSTEM_PARAMETER_Remote              Remote;
    u16    rsvd[80];        
};

// 系统参数结构体
typedef struct {
	struct SYSTEM_PARAMETER_System		System;
	struct SYSTEM_PARAMETER_BMS			BMS;
} SYSTEM_PARAMETER_structDef;


// End of system_parameter.h


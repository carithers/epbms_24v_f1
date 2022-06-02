/* ==================================================================================

 File name:     system_parameter.h
 Originator:    BLJ
 Description:   ϵͳ�����ṹ��ͷ�ļ���ϵͳ������������EEPROM��

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-28-2015     Version 0.0.1           ���Թ���
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                     // STM32�����Ĵ�������ͷ�ļ�


// ϵͳ��Ϣ 0x00-0x0F
struct SYSTEM_PARAMETER_Information {
	u16	DeviceName[8];						// �豸���ͱ��
	u16	SerialNumber1;						// ���к�1
	u16	SerialNumber2;						// ���к�2	
	u16	ManufactureData;					// ��������	��/���ڣ���1350����13���50��
	u16	HardwareVersion;					// Ӳ���汾 ����1.01���洢ֵ��101
	u16	SoftwareVersion;					// ����汾 ����1.01���洢ֵ��101
	u16	PrototalVersion;					// ͨ��Э��汾 ����1.01���洢ֵ��101
	u16	ParameterVersion;					// �����汾 ����1.01���洢ֵ��101
	u16	rsvd[1];
};

// У׼���� 0x10-0x17
struct SYSTEM_PARAMETER_Calibration {
	u16	CurrentSensorMaxCurrent;			// �����������Ŵ������Ĭ��100A
	u16 CurrentSensorMaxVoltage;			// �����������Ŵ������Ĭ��100mV����1mŷķ��������
	s16	CurrentSensorOffset;				// ����������ƫ��	һ��-255��255֮��
	s16	VdcSensorGain;						// ֱ����ѹ�������Ŵ����
	s16	VdcSensorOffset;					// ֱ����ѹ������ƫ��
	s16	InnerDriveSensorGain;			    // �ڲ�������ѹ�������Ŵ��������λ0.1kOhm
	s16	InnerDriveSensorOffset;				// �ڲ�������ѹ������ƫ�ã���λ0.1V
	s16	BMSIdleCurrent;						// BMS������ĵ�������λmA	
};

// �����ò�������д��EEPROM�� 0x18-0x1F
struct SYSTEM_PARAMETER_Test {
	u16	test_BalancePointer;				// ǿ��ָ���������󣬲�����
    u16	test_Shutdown;						// �ֶ�ǿ�Ƶ�ؽ�������ģʽ
    u16 test_ClearFaultHistory;             // ������ϼ�¼
    u16 test_ClearSOC;
    u16 test_ChargeReqVol;
	u16	rsvd[3];
};

// ϵͳ��Ϣ
struct SYSTEM_PARAMETER_System {
	struct SYSTEM_PARAMETER_Information 	Information;
	struct SYSTEM_PARAMETER_Calibration		Calibration;
    struct SYSTEM_PARAMETER_Test            Test;
};

// ��ز��� 0x20-0x2F
struct SYSTEM_PARAMETER_Battery {
	u16	AFESelect;							// AFEоƬѡ��	0:����ѡ��1��BQ76920��2��BQ76930��3��BQ76940��4��BQ76925
	u16 SeriesNumber;						// ����鴮������
	u16	FullCapacity;						// ���ʵ����������λ0.1AH
	u16	DesignCapacity;						// ����������������λ0.1AH
	u16	CellOverVoltage;					// �����ѹ����ֵ����λmV
	u16	CellUnderVoltage;					// ����Ƿѹ����ֵ����λmV
	u16	ShortCutDownCurrent;				// ��ض�·����,��λA
	u16	ShortCutDownDelay;					// ��ض�·��ʱ����λus
	u16	OverCutDownCurrent;					// ��ع�����������λA
	u16	OverCutDownDelay;					// ��ع�����ʱ����λms
	u16	CellChargeStopVoltage;				// �������ֹ��ѹ����λmV
	u16	ChargeCurrentLimit;					// ��س�����Ƶ�������λA
	u16	BatteryTemperatureCheckMode;		// ����¶ȼ��ʹ��
	u16	OverTemperature;					// ����¶ȹ������ƣ���λ���϶�
	s16	UnderTemperature;					// ����¶ȹ������ƣ���λ���϶�
	u16 ChargeStopDelay;                    // �������ӳ٣���λs
};



// ������Ʋ��� 0x30-0x37
struct SYSTEM_PARAMETER_Output {
	s16	AutoCutoffCurrent;					// ����Զ��жϵ�������λmA
	u16	AutoCutoffDelay;					// ����Զ��ж���ʱ����λs
	u16	SleepDelay;							// BMS����͹���״̬ǰ��ʱ����λs
	u16	OutputDelay;						// ���������ʱ����λms
	u16	PreDischargeTime;					// Ԥ�ŵ�ʱ�䣬��λms	
    u16 SleepDelayLong;                     // BMS����͹���״̬ǰ��ʱ����ʱ����λh����SleepDelay�ۼ�
	u16	rsvd[2];
};

// ���Ӵ���������ز�����0x38-0x3F
struct SYSTEM_PARAMETER_Contactor {
	u16 ContactorBaseFrequency;				// ���Ӵ���PWMƵ�ʣ���λHz
	u16 ContactorFullPercentTime;			// �Ӵ�������ʱȫռ�ձȳ���ʱ�䣬��λms
	u16	ContactorLongLastPercent;			// �Ӵ�����ʱ�乤��ʱռ�ձȣ���λ0.1%
	u16 rsvd[5];
};


// ���ⱨ����Ϣ���ã�0x40-0x47
struct SYSTEM_PARAMETER_Warning {
	u16	BDILowLevel1Percent;				// ��ص����ͱ�����Lv1����Ӧ��������λ%
	u16	BDILowLevel2Percent;				// ��ص������ع��ͱ�����Lv2����Ӧ��������λ%
	u16	BDILowLimitLift;					// �͵��������ƾ�������Ӧ������λ1%
	u16	BDILowLimitSpeed;					// �͵��������Ƴ��٣���Ӧ������λ1%
	u16	BMSOTemperatureCheckEnable;			// BMS������¶ȼ��ʹ��
	u16 BMSOverTemperature;              // BMS���������¶ȣ��ݶ�Ϊ80���϶�
    u16 LowSOCBeep;                         // ���ڴ��¶�ʱ��������ʼ����
	u16 rsvd[1];
};


// ��������л���0x48-0x4F
struct SYSTEM_PARAMETER_BatteryType {
	u16	BatteryType;				        // ������ͣ�0����Ԫ��1���������
    u16	BatteryTypeReal;				    // ʵ�ʵ�����ͣ������жϵ�������Ƿ��޸ģ�0����Ԫ��1���������
	u16	rsvd[6];
};

// �������ƣ�0x50-0x57
struct SYSTEM_PARAMETER_Charge {
    u16 ChargeVoltageStep1;                 // �������׶�1��������ߵ�ѹ���ڴ˵�ѹ�����������1C��Ϊ0.5C����λ1mV
    u16 ChargeVoltageStep2;                 // �������׶�2��������ߵ�ѹ���ڴ˵�ѹ�����������0.5C��Ϊ0.25C����λ1mV
    u16 ChargeVoltageStep3;                 // �������׶�3��������ߵ�ѹ���ڴ˵�ѹ�����������0.25C��Ϊ0.125C����λ1mV
    u16 ChargeForceStopVoltage;             // ���ǿ�ƽ�����ѹ����λmV
    u16 ChargeFinishMinVoltage;             // ��������С�����ѹ����λmV
    u16 ChargeOverCurrent;                  // ��籣����������λ1A
    u16 ChargeOverCurrentDelay;             // ��������ʱ����λ1s
	u16	ChargerCommEnable;                  // CAN���߳���ͨ��ʹ��
};

// ��ŵ���� ��0x58-0x5F
struct SYSTEM_PARAMETER_Discharge {
    u16 DischargeForceStopVoltage;          // ����ŵ�ǿ�ƽ�����ѹ
    u16 DischargeStopVoltage;               // ����ŵ������ѹ������Ϊ�ŵ�������ҳ�������5s�������ŵ�
	// cc_flg-17
	u16 CalibrateCapacity;
    u16 dsg_cc_low_k;
    u16 dsg_tmp_low_k;
    u16 rsvd[3];
	// cc_flg-17
    
};

// ��ص���У׼��0x60-0x6F
struct SYSTEM_PARAMETER_CapacityCalibrate {
    u16 CalibrateVoltage1;                  // У׼��ѹ1����Ӧ����1%
    u16 CalibrateVoltageLowLimit1;          // У׼��ѹ1����С����ֵ
    u16 CalibrateVoltage2;                  // У׼��ѹ2����Ӧ����3.3%
    u16 CalibrateVoltageLowLimit2;          // У׼��ѹ2����С����ֵ
    u16 CalibrateVoltage3;                  // У׼��ѹ3����Ӧ����10%
    u16 CalibrateVoltageLowLimit3;          // У׼��ѹ3����С����ֵ
    u16 CalibrateVoltage4;                  // У׼��ѹ4����Ӧ����20%
    u16 CalibrateVoltageLowLimit4;          // У׼��ѹ4����С����ֵ
    u16 rsvd[8];
};

// ���ȿ��ƣ�0x70-0x77
struct SYSTEM_PARAMETER_Fan {
	u16 FanBaseFrequency;				    // ����PWMƵ�ʣ���λHz
	u16 FanFullPercentTime;			        // ��������ʱȫռ�ձȳ���ʱ�䣬��λms
	u16	FanLongLastPercent;			        // ���ȳ�ʱ�乤��ʱռ�ձȣ���λ0.1%
    u16 FanEnableTemperature;               // ����ʹ���¶ȣ�������¶ȸ��ڴ��¶�ʱ����������
    s16	FanCurrent;		        			// �������е�������λmA
	u16 FanFullPercent;                     // ���������ռ�ձ�
    u16 FanMode;                            // ���ȹ���ģʽ��0�������¶�ʹ�ܣ�1������Keyʹ�ܣ�2���ڵ͵�������
    u16 OutputClsoseDelay;                   // �����ʱ������ģʽ1��������
};

// 48V60AH�����ͨ��Э�����,0x78-0x7F
struct SYSTEM_PARAMETER_CAN_Protocal_60AH {
    u16 CANFrameEnable;                     // ������֡����ʹ��
	u16	BDILowStopLift;					    // �͵��������ƾ�������Ӧ������λ1%
	u16	BDILowSpeedRedution1;			    // �͵��������Ƴ���1����Ӧ������λ1%
    u16	BDILowSpeedRedution2;			    // �͵��������Ƴ���1����Ӧ������λ1%
    u16	BDILowOpenMC;			            // �͵��������Ƴ���1����Ӧ������λ1%
    u16 rsvd[3];
};

// ��������ò�����0x80-0x87
struct SYSTEM_PARAMETER_Parallel {
    u16 ParallelEnable;                     // ��ز�������ʹ��
    u16 rsvd[7];
};

// ����������,0x88-0x8F
struct SYSTEM_PARAMETER_CurrentSensor {
    u16 CurrentSensorSelect;                // ����������ѡ��0��BQ769оƬ��ӷ�������1����ӵ��Ż�������������,2�����˫�Ż�������������,
    u16 rsvd[7];
};

// ��ر�����ز�����0x90-0x97
struct SYSTEM_PARAMETER_Protect {
    u16 CellSoftOverVoltage;                    // ���������ѹ��ѹ����λ1mV
    u16 CellSoftOverVoltageDelay;               // ���������ѹ��ʱ����λ1s
    u16 CellHardwareOverVoltageDelay;           // ����Ӳ����ѹ��ʱ����λ1s
    u16 ShortCurrentTimeFactor;                 // ������������·��������ֵ�Ŵ�������������Ķ�·����������ʱ = ShortCutDownDelay * ShortCurrentTimeFactor    
    u16 CellUnderVoltageDelay;                  // ���Ƿѹ��ʱ����λs
    u16 rsvd[3];
};

// ��ڳ����ز���,0x98-0x9F
struct SYSTEM_PARAMETER_Charge_GB {
    u16 ChargeMode;                             // 0��ͬ�ڳ�ŵ磬1����ڳ�ŵ磬Э�鲻�䣬2����ڳ�ŵ磬������Э��
    u16 rsvd[7];
};
// �����¶ȿ��ƣ�0xA0-0xA7
struct SYSTEM_PARAMETER_Charge2 {
    u16 ChargeLimitTemperature1;            // ��������¶�1�����ڴ��¶�ʱ��������������Ϊ0.5C,��λ1degC
    u16 ChargeLimitTemperature2;            // ��������¶�2�����ڴ��¶�ʱ��������������Ϊ0.25C����λ1degC
    s16 ChargeLimitTemperature3;
    s16 ChargeLimitTemperature4;            // ��������¶�4�����ڴ��¶�ʱ�����������
    u16 ChargeCurrent;                      //������
    u16 rsvd[3];
};
//Զ�̿��Ʋ�����0xA8-0xAF
struct SYSTEM_PARAMETER_Remote {
    u16 RemoteEnable;                           // Զ�̿���ʹ�ܱ�־λ
    u16 OffLineTime;                            // ������߹ر���ʱ����λmin��Ĭ��60min
    u16 rsvd[6];
};
    
// BMS ����
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
    struct SYSTEM_PARAMETER_Charge_GB           ChargeGB;           // ������Э�飬��ڳ����ز���
    struct SYSTEM_PARAMETER_Charge2             Charge2;   
    struct SYSTEM_PARAMETER_Remote              Remote;
    u16    rsvd[80];        
};

// ϵͳ�����ṹ��
typedef struct {
	struct SYSTEM_PARAMETER_System		System;
	struct SYSTEM_PARAMETER_BMS			BMS;
} SYSTEM_PARAMETER_structDef;


// End of system_parameter.h


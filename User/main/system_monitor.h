/* ==================================================================================

 File name:     system_monitor.h
 Originator:    BLJ
 Description:   ϵͳ����ֵ�������ṹ��

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-15-2016     Version 0.0.1           ���Թ���
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                      // STM32�����Ĵ�������ͷ�ļ�


// ��ز�����Ϣ 0x00-0x0F
struct SYSTEM_MONITOR_Battery {
	s16	Voltage;							// ������ܵ�ѹ����λ0.1V  // //g_SystemMonitor.BMS.Battery.Temperature
	s16	Current;							// ������ܵ�������λ0.1A
	s16	CurrentCali;						// ������ܵ���У׼����λ0.001A����Χ-0.099-0.099A
	s16	Capacity;							// ���ʣ���������λ0.1AH
	s16	CapacityCali;						// ���ʣ�����У׼����λ0.001AH����Χ0-0.099AH
	s16	SOC;								// ���ʣ������ٷֱȣ���λ0.1%
	s16	CellMaxVoltage;						// ������ߵ�ѹ����λ0.001V
	s16	CellMaxVoltagePointer;				// ������ߵ�ѹ��Ӧ�ڼ��ڵ�أ���ײ�һ��Ϊ��һ��
	s16	CellMinVoltage;						// ������͵�ѹ����λ0.001V
	s16	CellMinVoltagePointer;				// ������͵�ѹ��Ӧ�ڼ��ڵ�أ���ײ�һ��Ϊ��һ��
	s16	Temperature[3];						// ����¶�1����λ0.1���϶�
	u16	CellBalanceEnable;					// �������ʹ��
	u16	CellBalancing;						// ����ʹ���б�־λ
	u16	CellBalancePointer;					// ��������Ӧ�ڼ��ڵ��
};

// ������Ϣ 0x10-0x1F
struct SYSTEM_MONITOR_Cell {
	s16	CellVoltage[15];					// �����ص�ѹ����λ0.001V
	s16	rsvd;
};

// AFEоƬ��Ϣ 0x20-0x27
struct SYSTEM_MONITOR_AFE {
	s16	InnerTemperature[3];				// AFEоƬ�ڲ��¶Ȳ�������λ1���϶�
	s16 BatteryTemperature[4];              // ����ڲ��¶ȣ���λ0.1���϶�
	u16	rsvd;
};

// ������Ϣ 0x28-0x2F
struct SYSTEM_MONITOR_Manage_bits {
	u16	DischargeEnable:1;					// �ŵ翪��ʹ�ܱ�־λ��ʵ���г�ŵ翪��ͬʱ�򿪹ر�
	u16	ChargeEnable:1;						// ��翪��ʹ�ܱ�־λ
	u16	DischargeState:1;					// �ŵ翪��ʵ��״̬��0���رգ�1��ʹ��
	u16	ChargeState:1;						// ��翪��ʵ��״̬��0���رգ�1��ʹ��
	u16	DischargeIOState:1;					// BQ769xоƬ�ŵ翪�ؿ���IO�ڵ�ƽ��0���رգ�1���ߵ�ƽ��ʹ�ܷŵ�
	u16	ChargeIOState:1;					// BQ769xоƬ��翪�ؿ���IO�ڵ�ƽ��0���رգ�1���ߵ�ƽ��ʹ�ܳ��
	u16	KEYState:1;							// KEY�����ź�
	u16	ExtraBalance:1;						// ���������ʹ��
	u16	rsvd:8;	
};

union SYSTEM_MONITOR_Manage_union {
	u16									all;
	struct SYSTEM_MONITOR_Manage_bits	bit;
};

struct SYSTEM_MONITOR_Manage {
	union SYSTEM_MONITOR_Manage_union 	Manage_bits;
	u16	ExtraBalanceLastTime;				// ��������ѳ���ʱ�䣬��λmin
	u16	ChargeCurrentLimit;					// ��������������λ0.1A
	s16	BMSTemperature[2];					// BMS������¶ȣ���λ0.1���϶�
    u16 DriveVoltage;                       // �ڲ�������ѹ����λ0.1V
    u16 BatteryPackageVoltage;              // ��ذ�����ܵ�ѹ��ADC�����õ�����λ0.1V
	u16	rsvd;
};


// ������Ϣ 0x30-0x37
struct SYSTEM_MONITOR_Fault_bits {
	u16	ShortCurrent:1;						// ��ض�·
	u16	OverCurrent:1;						// ��ع���
	u16	CellOverVoltage:1;					// �����ѹ����
	u16	CellUnderVoltage:1;					// �����ѹ����
	u16	OverTemperature:1;					// ����¶ȹ���
	u16	UnderTemperature:1;					// ��ѹ�¶ȹ���
	u16	rsvd:10;
};

union SYSTEM_MONITOR_Fault_union {
	u16									all;
	struct SYSTEM_MONITOR_Fault_bits	bit;
};

struct SYSTEM_MONITOR_Fault {
	u16	FaultCodeLv0;
	u16	FaultCodeLv1;
	union SYSTEM_MONITOR_Fault_union	FaultInf_bits;
	u16	rsvd[5];
};

//����ʱ�� 
struct SYSTEM_MONITOR_Timer {
    u16 RunTimeS;                           // �ϵ�����ʱ�䣬��λs
    u16 DischargeTimeS;                     // ���ηŵ��ۼ�ʱ�䣬��λs
    u16 rsvd[3];
    u16 I2CInterruptCnt;                    // I2C�жϴ���
    u16 MainInterruptCnt;                   // 1ms��ʱ�жϴ���
    u16 MainLoopCnt;                        // ��ѭ������
};

//������¶� 
struct SYSTEM_MONITOR_BMS_Temperature {
    s16 BMSTemperature[4];                  // BMS������¶ȣ���λ0.1���϶�
    u16 rsvd[4];
};

// �ֱ��Ӧ�ۺ��������һ·���ڶ�·
struct SYSTEM_MONITOR_Current {    
	s16	Current;							// ������ܵ�������λ0.1A
	s16	CurrentCali;						// ������ܵ���У׼����λ0.001A����Χ-0.099-0.099A
	s16	Current1;							// ������ܵ���1����λ0.1A
	s16	Current1Cali;						// ������ܵ���У׼1����λ0.001A����Χ-0.099-0.099A
	s16	Current2;							// ������ܵ���2����λ0.1A
	s16	Current2Cali;						// ������ܵ���У׼2����λ0.001A����Χ-0.099-0.099A
    u16 rsvd[2];
};

// ---------------- BMS��Ϣ��0x00-0x67 --------------------
struct SYSTEM_MONITOR_BMS {
	struct SYSTEM_MONITOR_Battery	                Battery;        // ��ز�����Ϣ 0x00-0x0F 
	struct SYSTEM_MONITOR_Cell		                Cell;           // ������Ϣ 0x10-0x1F
	struct SYSTEM_MONITOR_AFE		                AFE;            // AFEоƬ��Ϣ 0x20-0x27
	struct SYSTEM_MONITOR_Manage	                Manage;         // ������Ϣ 0x28-0x2F
	struct SYSTEM_MONITOR_Fault	                    Fault;          // ������Ϣ 0x30-0x37
    struct SYSTEM_MONITOR_Timer                     Timer;          // ����ʱ�� 0x38-0x3F
    struct SYSTEM_MONITOR_Cell		                Cell2;          // ������Ϣ��λ 0x40-0x4F
    struct SYSTEM_MONITOR_AFE		                Temperature2;   // ��Ӧ��λAFE������ڲ��¶�5-8     ,0x50-0x57
    struct SYSTEM_MONITOR_BMS_Temperature           BMSTemperature; // BMS�����¶ȴ�����, 0x58-0x5F
    struct SYSTEM_MONITOR_Current                   Current;        // �ۺ������0x60-0x67
    u16    rsvd[16];

};

// ϵͳԤ��������Ϣ�����ڲ��ԣ�0x38-0x47
// ϵͳԤ��������Ϣ2�����ڲ��ԣ�0x48-0x57
struct SYSTEM_MONITOR_Reverse {
	s16	Data[16];
};

// --------------- ϵͳ����ֵ�������� ----------------------
struct SYSTEM_MONITOR_System {
	struct SYSTEM_MONITOR_Reverse	Reverse;
	struct SYSTEM_MONITOR_Reverse	Reverse2;
	struct SYSTEM_MONITOR_Reverse	Reverse3;
};


// ------------------------------ ϵͳ�����ṹ�� -------------------------------
typedef struct {
	struct SYSTEM_MONITOR_BMS		BMS;
	struct SYSTEM_MONITOR_System	System;
} SYSTEM_MONITOR_structDef;

// End of system_monitor.h


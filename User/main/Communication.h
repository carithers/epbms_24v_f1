/* ==================================================================================

 File name:     communication.h
 Originator:    BLJ
 Description:   �豸֮��ͨ���������ݽṹ��ͷ�ļ�.

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 11-28-2014     Version 0.0.1          
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"

// ------------------------------ ģ��ṹ��---------------------------------
struct Comm_NMT {
	u8	ControlCommond;
	u8	rsvd[7];
};

/*
// ǣ��������ͨ����������
struct Comm_Traction1_Input_bit {
    u8 Input1:1;
    u8 Input2:1;
    u8 Input3:1;
    u8 Input4:1;
    u8 Input5:1;
    u8 Input6:1;
    u8 Input7:1;
    u8 Input8:1;
};

union Comm_Traction1_Input_union {
    u8                                 all;
    struct Comm_Traction1_Input_bit     bit;
};

struct Comm_Traction1 {
    union Comm_Traction1_Input_union    Input;      // ��������
    u8 FaultCode;              // ���Ϸ���
    u8 SpeedHzHi;              // �ٶȷ�����λ
    u8 SpeedHzLow;             // �ٶȷ�����λ
};
*/

struct Comm_Traction1_Input_bit {
    u8 Input0:1;
    u8 Input1:1;
    u8 Input2:1;
    u8 Input3:1;
    u8 Input4:1;
	u8 Input5:1;
    u8 Input6:1;
    u8 Input7:1;
};

union Comm_Traction1_Input_union {
    u8                                 all;
    struct Comm_Traction1_Input_bit     bit;
};

struct Comm_Traction1 {
	u8	rsvd;
	u8	BDI;
	u8 	SpeedHzHi;              // 速度反馈�?位，单位为频�?
    u8 	SpeedHzLow;             // 速度反馈�?位，单位为频�?
	s8	Steer;
    union Comm_Traction1_Input_union    Input;      // 数字输入�?
	u8	rsvd2;
	u8	rsvd3;
};

struct Comm_Traction2_Input_bit {
    u8 Seat:1;
    u8 Forward:1;
    u8 Input3:1;
    u8 Input4:1;
    u8 Input5:1;
	u8 Input6:1;
    u8 Input7:1;
    u8 Input8:1;
};

union Comm_Traction2_Input_union {
    u8                                 all;
    struct Comm_Traction2_Input_bit     bit;
};

struct Comm_Traction2 {
    u8 rsvd1;
    u8 rsvd2;
    u8 rsvd3;
    u8 rsvd4;
    u8 rsvd5;	
	union Comm_Traction2_Input_union    Input;      // 数字输入�?
    u8 rsvd[2];
};

struct Comm_Fault {
    u8 rsvd1;
    u8 rsvd2;
    u8 FaultCode;
    u8 rsvd[5];
};



struct Comm_SmartDisplay_Input_bits {
    u8 bit0:1;				// ��ɲ
    u8 bit1:1;
    u8 bit2:1;				// ���̾���
    u8 bit3:1;
	u8 bit4:1;				// ���̺���
    u8 bit5:1;				// ����ƽ��	
    u8 bit6:1;				// �û��ٶ�	
    u8 bit7:1;	
};

union Comm_SmartDisplay_Input_union {
    u8                                 		all;
    struct Comm_SmartDisplay_Input_bits     bit;
};

struct Comm_SmartDisplay {
	union Comm_SmartDisplay_Input_union	Input;
    u8 rsvd[7];
};

struct Comm_SmartSpeedLimit {
	u8	rsvd1;
	u8	rsvd2;
	u8	SpeedLimit;
	u8	rsvd[5];
};

struct Comm_TractionTimeAsk {
	u8	TimeAsk;
	u8 rsvdp[7];
};

struct Comm_TractionTime {
	u8	rsvd1;
	u8	rsvd2;
	u8	TimeHi;
	u8	TimeLow;
	u8	rsvd[4];
};

struct Comm_MDI_CAN_DIN1 {
	u8	BYTE0;
	u8	BYTE1;
	u8	rsvd[6];
};

struct Comm_BMS_Data_Test {
	u8	CapacityByte3;
	u8	CapacityByte2;
	u8	CapacityByte1;
	u8	CapacityByte;
	u8	VoltageHi;
	u8	VoltageLow;
	u8	CurrentHi;
	u8	CurrentLow;
};

struct Comm_BMS_Control_Test {
	u8	DSGControl;
	u8	CHGControl;
	u8	rsvd[6];
};

struct Comm_Test {
	u8	Byte[8];
};


struct Comm_BMS_Data_BatteryState_bits {
    u8 Charge:1;						// 0��δ���ߣ������
    u8 UnderTemperature:1;				// ����¶ȹ���
    u8 OverTemperature:1;				// ����¶ȹ���
    u8 ShortCurrent:1;					// ��ض�·
	u8 OverCurrent:1;					// ��ع���
    u8 UnderVoltage:1;					// ����Ƿѹ	
    u8 OverVoltage:1;					// �����ѹ
	u8 BMSFail:1;						// BMS����
};


union Comm_BMS_Data_BatteryState_union {
    u8                                 			all;
    struct Comm_BMS_Data_BatteryState_bits     	bit;
};


struct Comm_BMS_Data1 {
	u8	BatteryVoltageHi;				// ����ܵ�ѹ����λ0.1V����8ʹ
	u8	BatteryVoltageLow;
	u8	BatteryCurrentHi;				// ����ܵ��������Ϊ����������ʹ.001A����8ʹ
	u8	BatteryCurrentLow;
	u8	CapacityHi;						// ���ʣ���������ʹ.001AH����8ʹ
	u8	CapacityLow;
	u8	SOC;							// ���ʣ������ٷֱȣ�1%
	union Comm_BMS_Data_BatteryState_union	BatteryState;
};


struct Comm_BMS_Data2 {
	u8	SingleMaxVoltageHi;				// ������ߵ�ѹ����λ0.001V����8ʹ
	u8	SingleMaxVoltageLow;
	u8	SingleMaxPointer;				// ��ߵ�ѹ�����Ӧ�ڼ���
	u8	SingleMinVoltageHi;				// ������͵�ѹ����λ0.001V����8ʹ
	u8	SingleMinVoltageLow;			
	u8	SingleMinPointer;				// ��͵�ѹ�����Ӧ�ڼ���
	u8	BatteryTemperature;				// ����¶ȣ��з��ţ���ʹ���ώ�
	u8	BatteryTemperature2;			// ����¶ȣ��з��ţ���ʹ���ώ�
};

struct Comm_BMS_Data3 {
	u8	BDILowLevel;					// ������ߵ�ѹ����λ0.001V����8ʹ
	u8	rsvd[7];
};

struct Comm_Curtis1212C_Ask {
	u8	rsvd[8];
};

struct Comm_BMS_LowSOC_bits {
    u8 LiftLimit:1;						//
	u8 SpeedLimit:1;
	u8 rsvd:6;
};

union Comm_BMS_LowSOC_union {
    u8                         		all;
    struct Comm_BMS_LowSOC_bits    	bit;
};

struct Comm_Curtis1212C_Response {
	union Comm_BMS_LowSOC_union		LowSOC;
	u8	rsvd[7];
};


struct Comm_Charger {
	u8	VoltageHi;
	u8	VoltageLow;
	u8	CurrentHi;
	u8	CurrentLow;
    u8  ChargerState;
	u8	rsvd[2];
	u8  ChargeFaultCode;	
};

struct Comm_BMS_ChargeCommand {
	u8	ChargeVoltageLimitHi;
	u8	ChargeVoltageLimitLow;
	u8	ChargeCurrentLimitHi;
	u8	ChargeCurrentLimitLow;
	u8	ChargeAllow;					// ��������־λ��1����ֹ��磬0�������m
// ���״̬���������ֵΪ0x00�������ֹ��磬���������£�0x01 �ѳ�M 0x02 ����¶ȹ��� 0x03 ����¶ȹ��� 0x04 �����ѹ���� 0x05 
// �����ѹ���� 0x06 �����ѹ���� 0x07 �����ѹ���� 0x08 ��ص����ѹ������0x09 ������  ...(�����Ƿ������Ƴ���ԭ������
    u8  BMSState; 
    u8  SOCFullFlag;                    // ��س�����־ʹ
	u8	rsvd;
};

struct Comm_FaultCode {
	u8	FaultCodeLv1;					// 1�����ϣ�ֱ���ж����
	u8	WarningCodeLv2;					// 2�����������������
	u8	FaultCodeLv3;
	u8	FaultCodeLv4;
	u8	rsvd[3];
    u8  BMSTemperature;
};

struct Comm_BMSFault_Data {
	u8      BYTE0;
	u8		BYTE1;
	u8		BYTE2;
	u8		BYTE3;
	u8		BYTE4;
	u8		BYTE5;
	u8		BYTE6;
	u8		BYTE7;
};


// �豸��ͨ�����ݽṹʹ
typedef struct {
	struct Comm_NMT						NMT;						// 0x00
    struct Comm_Traction1   			TractionInf;				// 0x388
	struct Comm_Traction2				TractionInf2;				// 0x18C
    struct Comm_Fault   				FaultInf1;					// 0x288		// Node 3, Traction
	struct Comm_Fault   				FaultInf2;					// 0x28C		// Node 5, Pump
	struct Comm_Fault   				FaultInf3;					// 0x290		// Node 4, Traction2
	struct Comm_Fault					FaultInf4;					// 0x294		// Node 7, Pump2
	struct Comm_Fault					FaultInf5;					// 0x298		// Node 6, Eps
	struct Comm_SmartDisplay			SmartDispaly;				// 0x3C0
	struct Comm_SmartSpeedLimit			SmartSpeedLimit;			// 0x60C
	struct Comm_TractionTimeAsk			TractionTimeAsk;			// 0x608
	struct Comm_TractionTime			TractionTime;				// 0x588
	struct Comm_MDI_CAN_DIN1			MDI_CAN_DIN1;				// 0x240
	struct Comm_BMS_Data_Test			BMSDataTest;				// 0x600
	struct Comm_BMS_Control_Test		BMSControlTest;				// 0x681
	struct Comm_Test					Test1;						// 0x682
	struct Comm_FaultCode				FaultCode;					// 0x683
	struct Comm_BMS_Data1				BMSData1;					// 0x2F0
	struct Comm_BMS_Data2				BMSData2;					// 0x2F1
	struct Comm_BMS_Data3				BMSData3;					// 0x2F2
	struct Comm_Curtis1212C_Ask			Curtis1212CAskBMS;			// 0x27C
	struct Comm_Curtis1212C_Response	Curtis1212CResponseBMS;		// 0x1FC
	struct Comm_Charger					ChargerState;				// 0x0F5
	struct Comm_BMS_ChargeCommand 		BMSChargeCommand;			// 0x0F4
	
	struct Comm_BMSFault_Data           BootCMD;                    // 0x797��λ���������ֽ���
	struct Comm_BMSFault_Data           BootRev;                    // 0x7AB�豸�ظ���λ��
	
	u8	ReceiveFrame[8];
	u8	SendFrame[8];
} Comm_structDef;












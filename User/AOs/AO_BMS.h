/* ==================================================================================

 File name:     AO_BMS.c
 Originator:    BLJ
 Description:   ��ع���ϵͳ״̬��
 Take care�� ����״̬����󣬶�ʱ��1���������Զ�ʱ

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 12-20-2015     Version 0.0.1           ��������
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                     		// STM32�����Ĵ�������ͷ�ļ�
#include "qpn_port.h"                           	// ״̬��ͷ�ļ�

// ------------------------------- ȫ�ֺ궨�� ---------------------------------------
#define	BMS_COMMAND_IDLE			0				// ���д���ָ��
#define	BMS_COMMAND_DISCHARGE		1				// �ŵ�ָ��
#define	BMS_COMMAND_CHARGE			2				// ���ָ��



// -------------------------- ��ʾģ������ṹ�� ---------------------------------
struct AO_BMS_State {
	u16	BatteryExtraBalance;				// ��ض������ʹ�ܣ����ڵ�ؿ���״̬���������10��Сʱ�����뿪����״̬����ȡ��
	u16	ChipResetAsk;						// оƬ�������󣬵����������ú󣬻���λ�˱�־λ
	u16	CANCommEnable;						// CANͨ��ʹ��
    u16 OutputAllow;                        // ��������ʹ�ܣ���λ������λ�����������
};

struct AO_BMS_Parameter {
	s32	BatteryFullCapacity;				// �����ʵ����������λmAH
	s32	BatteryDesignCapacity;				// ����������������λmAH
	s32	BMSIdleCurrent;						// BMS�����ص�������λmA�����ڲ����������
    s32 FanCurrent;                         // �������е�������λmA�����ڲ����������
    u16 LowSOCCutOffTime;                   // �͵������ڽӴ����ж�ʱ��
};

struct AO_BMS_Variable {
	s32	BatteryCapacityTemp;				// �����������ʱ��������λmAS
	s32	ChargeStartCapacity;				// ������״̬ʱ��ʣ�������������ʱ�Ƚϵó����γ�������������ѭ������
	u32	ReadValueTimeOld;					// ���ݶ�ȡʱ�䣬��λ1ms
	u16	ExtraBallanceCount;					// ��ض�����⣬������ÿ1H��1
	u16	ShutdownDelay;						// ��ѹǷѹ��ʱ
	u16	FaultSleepDelay;					// ����ʱ��ʱ�رռ�����
	u16	FaultNoRecoverDelay;				// ����δִ�лָ�������
	u32	AutoCutoffDelay;					// ����Զ��ж���ʱ����λs
	u16	BatteryExtraBalanceDelay;			// ��ض������ʹ����ʱ
	u16	BatteryOutputDelay;					// ������������ʱ
	u16 LowCapCutoffCnt;                    // ���ڵ͵����жϼ�ʱ
    u16 DischargeFilter;
    u16 ChargeCheckCnt;                     // ���״̬�ж���ʱ
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
    s32 BatteryCurrent;                     // ��ص�������λmA����Ϊ��磬��Ϊ�ŵ�
    s32 BatteryCurrentFld;                  // ��ص�������ͨ�˲��󣬵�λmA����Ϊ��磬��Ϊ�ŵ�
	s32	BatteryCapacity;					// �����������λmAH
	s32	FullCapacity;						// ���ʵ�ʿɳ����������λmAH��BMSģ�鸳ֵ
	s32	ChargeOrDischargeCapacity;			// ���γ�ŵ��������λmAH����ֵΪ��磬��ֵΪ�ŵ�
	s32	BatteryTemperature[8];				// ����¶ȣ���λ0.1���϶�
    s32 BatteryTemperatureHi;               // ��ذ�����¶ȣ���λ0.1���϶�
    s32 BatteryTemperatureLow;              // ��ذ�����¶ȣ���λ0.1���϶�    
	s32	BMSTemperature[4];					// BMS���ư壬��λ0.1���϶�
	s32 BMSTemperatureHi;                   // BMS���ư�����¶ȣ���λ0.1���϶�
	u16	CircleNumberAdd;					// ѭ���������Ӵ�������λ0.1��
	u16	LifeCalibrateCircleNumberAdd;		// ���ѭ������У׼��������λ0.1��
	u16	SOC;								// ��ص����ٷֱȣ���λ0.1%
	u16	SingleMinVoltagePointer;			// ������ߵ�ѹ��Ӧ�ڼ��ڣ�1-7
	s16	ChargeCurrentLimit;					// ������Ƶ���
    u16 DischargeTimeOnce;                  // ���ηŵ��ۼ�ʱ��
    u16 StartSOC;                           // С��15%ʱ���ڱ��SOC���˺����ÿ����2%�ж�һ�����
};

typedef struct AO_BMSTag {                      
    QActive super;							 			// derive from QActive
	struct 	AO_BMS_State		State;				// ģ��״̬
	struct 	AO_BMS_Parameter	Parameter;			// BQ769x0 оƬ����
	struct	AO_BMS_Variable		Variable;			// ģ����ʱ����
	struct 	AO_BMS_Output		Output;				// �������ֵ
} AO_BMS;

// ------------------------------ ����������� ---------------------------------
void AO_BMS_ctor(AO_BMS * const me);

// ------------------------------ �����״̬ ---------------------------------
static QState AO_BMS_initial(AO_BMS * const me);							// ��ʼ״̬
static QState AO_BMS_StartWait(AO_BMS * const me);						// 1.�����ȴ�״̬
static QState AO_BMS_Normal(AO_BMS * const me);							// 2.����״̬
static QState AO_BMS_Idle(AO_BMS * const me);						// 3.����״̬
static QState AO_BMS_ExtraBalance(AO_BMS * const me);			// 32.�������״̬
static QState AO_BMS_SleepDelay(AO_BMS * const me);				// 33.˯��ǰ��ʱ״̬
static QState AO_BMS_OnDelay(AO_BMS * const me);                // 34.���ǰ��ʱ״̬
static QState AO_BMS_On(AO_BMS * const me);							// 31.�������״̬
static QState AO_BMS_Fault(AO_BMS * const me);							// 100.����״̬
static QState AO_BMS_LowPower(AO_BMS * const me);                 // �͵��������߼�
static QState AO_BMS_EnterBootloader(AO_BMS * const me);				// 200.����Bootloader��ʱ״̬
// End of AO_BMS.h

/* ==================================================================================

 File name:     system_state.h
 Originator:    BLJ
 Description:   ϵͳ״̬���ݽṹ�壬��¼ϵͳ״̬��ȫ���������

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 01-03-2015     Version 0.0.1           ���Թ���
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                      // STM32�����Ĵ�������ͷ�ļ�

// ȫ��ϵͳ״̬������ʾϵͳ״̬
struct SYSTEM_STATE_State_bits {
	u16	EEPROMReadFinishFlag:1;				// EEPROM��Ϣ��ȡ�ɹ���־����ʱ����
	u16	IWDGReloadEnable:1;					// 0����ֹι����1:����ι��
    u16 KEYState:1;                         // �ⲿKEY����ʹ�ܱ�־λ��0��KEY�Ͽ���δʹ�ܣ�1��KEY�պϣ�ʹ��
    u16 ChargerFaultFlag:1;                 // �������ϱ�־λ
    u16 TestLED:1;                           
    u16 ChargeOnFlag:1;                     //��翪����־
    u16 ChargeTempError:1;                  //����¶ȱ���
	u16 EnterBootloaderAsk:1;               // ����bootloader�������˱�־λ��λ�����ͽ���Bootloader�¼���������ñ�־λ
	u16	rsvd:8;
};

union SYSTEM_STATE_union {
	u16								all;
	struct SYSTEM_STATE_State_bits	bit;
};

struct SYSTEM_STATE_Variable {
	u32	MCUIDNumber[3];						// оƬID number
	u16 EEPROMCheckValue[6];				// EEPROM�е�У����Ϣ
	u16 EEPROMCheckValue2[6];				// EEPROM�е�У����Ϣ
	u32	SystemMainLoopCountOld;				// ��ѭ����������
	u16	g_systemMainLoopCountSameCount;		// ��ѭ��δִ�д�����������1msʱ���ж�֮�䣬��ѭ��δ��ִ��
	u16	ParameterWriteAsk;					// ϵͳ����д�����󣬵���λ��λʱ����ʼ��ʱ����ʱ����������д��
	u16	ParameterWriteDelay;				// ϵͳ����д��֮ǰ��ʱ1s����ֹƵ��д��
    u16 ChargeCurrentLimitDivision;         // ���������Ʒ�Ƶ������2����0.5C
};

struct SYSTEM_STATE_Output {
	u32	System1msInterruptCount;			// 1msʱ���жϼ���
	u32	SystemMainLoopCount;				// ��ѭ������
	u32	SystemStartTime;					// ϵͳ����ʱ�䣬�����м�¼��TotalTimeΪ׼����ǰʱ���ȥ����ʱ�伴����������ʱ��
};

// ϵͳ״̬�ṹ��
typedef struct {
	union 	SYSTEM_STATE_union		State;
	struct 	SYSTEM_STATE_Variable	Variable;
	struct 	SYSTEM_STATE_Output		Output;
} SYSTEM_STATE_structDef;

// End of system_state.h


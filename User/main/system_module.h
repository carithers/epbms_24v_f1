/* ==================================================================================

 File name:     system_module.h
 Originator:    BLJ
 Description:   ϵͳ������ģ��

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 04-1-2015     Version 0.0.1           ���Թ���
-----------------------------------------------------------------------------------*/



// --------------------------- �ⲿ�ɵ��ú��� ------------------------------------
void System_LED_Update(void);					// LED������£�������1ms�ж���
void I2C_BQ769x0_SIG_Update(void);				// BQ769x0 I2C ͨ������¼�����
void flash_EEPROM_SIG_Update(void);				// flashģ��EEPROM ����¼�����
void SystemMonitor_Update(void);				// ϵͳ״̬�������º�����1s����1��
void SystemParameter_Init(void);				// ϵͳ������ʼ������ֵĬ��ֵ
//void SystemComm_Update(void);					// ϵͳͨ�Ÿ��º�������������ѭ����
void UsartCommand_Update(void);                 // ϵͳ����ͨѶ���º�������������ѭ����
void Debuger_Update(void);						// ϵͳ�����ô���������������ѭ����
void System_ParameterSet(void);					// ϵͳģ���������EEPROM������ʼ��
void ChargerCommand_Update(void);               // ���³�������ָ�� 

int get_temp_current_v(short t, int c);

// End of system_module.h


/* ==================================================================================

 File name:     AO_BQ769x0.h
 Originator:    BLJ
 Description:   BQ769x0ͨ�ż��������״̬������Ҫ����I2Cͨ�ż��Ĵ�����д

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 07-14-2016     Version 0.9.1           ����CRCУ�����������֤��
 07-01-2016     Version 0.9.0           ����������ɣ����ֹ��ܿ����Ż��������ڲ���
 01-02-2015     Version 0.0.1           ���Թ���ͨ��
-----------------------------------------------------------------------------------*/
#ifndef __AO_SH36730X0_H
#define __AO_SH36730X0_H
#include "stm32f0xx.h"                                         // STM32�����Ĵ�������ͷ�ļ�
#include "qpn_port.h"                                           // ״̬��ͷ�ļ�
                                                                
// ------------------------------- ȫ�ֺ궨�� ---------------------------------------
//#define BQ769X0_CLEAR_SYS_STAT          1                     // BQ769x0 оƬϵͳ״̬���ָ��
//#define BQ769X0_CONTROL_ON              2                     // BQ769x0 оƬ���³�ŵ��·ָ��,������ŵ��·
//#define BQ769X0_CONTROL_OFF             3                     // BQ769x0 оƬ���³�ŵ��·ָ��,�رճ�ŵ��·

#define SH36730x0_RECORD_LENGTH           16                    // BQ769x0״̬����ת��¼����

// -------------------------- ��ʾģ������ṹ�� ---------------------------------
struct AO_SH36730x0_State {
    u32 SampleUpdateCount;                                      // �������´���
    u16 EnterSHIPModeAsk;                                       // ����͹���ģʽ�����ⲿ��λ
    u16 BatteryBalanceEnable;                                   // ��ؾ���ʹ��
    u16 BatteryBalancing;                                       // ��ؾ����У�0���޵�ؾ��⣬1��������
    u16 BatteryBalancePointer;                                  // �ڼ��ڵ���ھ���
    u16 Timer1sUpdateAsk;                                       // 1s��ʱ���������־λ�����յ���ʱ��������ʱ����ִ��ͨ�ţ���˱�־λ��λ���ص�Idleʱ����ִ�и���ָ��
    u16 SH36730x0ControlAsk;                                    // SH36730x0оƬ����ָ����յ�ָ��ʱ����ִ��ͨ�ţ������������Ϊ����ָ��ص�Idleʱ����ִ��ָ��
    u16 DSGControl;                                             // �ŵ���ƿ���״̬��0���رգ�1������
    u16 CHGControl;                                             // �����ƿ���״̬��0���رգ�1������   
    u16 CANSendEnable;                                          // CANͨ�ŷ���ʹ��
	u16	test_ForceBatteryBalancePointer;						// �����ã�ǿ��ָ����������
};

struct AO_SH36730x0_Parameter {
    s32 ADCGain;                                                // ��ѹ�Ŵ��������λuV
    s32 ADCOffset;                                              // ��ѹ������̬ƫ���λmV
    s32 SingleOverVoltage;                                      // �����ع�ѹ
    s32 SingleUnderVoltage;                                     // ������Ƿѹ
    s32 CCGain;                                                 // �����������Ŵ������ֱ�ӱ�����������
    s32 CCOffset;                                               // ����������ƫ�ã�ֱ�������ڲ��������
    s32 BallanceErrVoltage;                                     // ��ؾ�������������λmV
    u16 SH36730x0_Type;                                         // BQ769x0оƬѡ�ͣ�1:SH367303,2:SH367305,3:SH36736
    u16 CellNumber;                                             // �ܹ���ش�����
    u8  CellSelect[10];                                         // ��������ѡ��
};

struct AO_SH36730x0_Variable_bits {
    u32 Block:4;                                                // Variable: EEPROM has five blocks for information, parameter, record ......
    u32 BlockStack1:4;                                          // Variable: EEPROM block in stack
    u32 BlockStack2:4;                                          // Variable: EEPROM block in stack
    u32 StackCount:4;                                           // Variable: block numbers in stack
    u32 SystemRecordPageFoundFlag:1;                            // ϵͳ���м�¼����ҳ���ҵ�
    u32 SystemRecordPage:2;                                     // ϵͳ���м�¼����ҳ
    u32 SystemRecordFoundFlag:1;                                // Variable: The last vehicle record has been found flag
    u32 WriteParameterAllow:1;                                  // �յ�EEPROMģ�鷢����ϵͳ����������ȡ�¼�����������SH36730x0оƬ����
    u32 rsvd:10;
};

struct AO_SH36730x0_Variable_union {
    u32                                   all;
    struct AO_SH36730x0_Variable_bits     bit;
};

struct AO_SH36730x0_Variable {
    struct  AO_SH36730x0_Variable_union   Variable_bits;        //
    u32 TotalTimeRecord;                                        // Variable: use for total time record in first read //��ʱ���¼����
    u32 mileageOld;                                             // Variable: Old mileage�� use for record as mileage 100m once//��̼�¼
    u16 SampleUpdateCount;                                      // ������������һ�β�����ȡ���ܳ�������֮
    u16 EEPROMFaultInf;                                         // Variable: EEPROM fault information             //������Ϣ
    u16 RecordWriteDelay;                                       // Variable: ���м�¼д��EEPROM��ʱ��������λs
    u16 EEPROMWriteBlock;                                       // Variable: EEPROMд��ָ������
    u16 EEPROMWriteBlockWait;                                   // �ȴ�д������
    u16 SystemRecordCircleNumber;                               // ϵͳ���м�¼�����ţ�0-63  *2
    u16 SystemRecordRetryCount;                                 // ϵͳ���м�¼��ȡ���Դ���������ȡ�����м�¼LRCУ��������ȡ��һ�μ�¼���˱���++���������Դ���
    u16 CellNumberPointer;                                      // �Ѽ��㵥���ѹ����
    u16 ClearSystemStateCount;                                  // ϵͳ���ϻָ����Դ���
    u16 ParameterWriteNumber;                                   // ϵͳ����д�����
    u16 UpdateCount;                                            // ������ݸ��¼�������ÿ60s����һ�ε�ؾ�������ŵ��·����
    u16 EnterSHIPStep;                                          // ����SHIPģʽ�跢��ָ�����ݣ�����������
    u16 ReadValueCount;                                         // ���ݶ�ȡ����
    u16 CCReadedFlag;                                           // ��������ֵ�Ѷ�ȡ��־λ
    u16 I2CFaultRetryCount;                                     // I2CͨѶʧ�����Դ�����Ĭ���������5��
    u16 CRCCheckRetryCount;                                     // BSH36730оƬͨ��CRCУ��ʧ�����Դ������������Ϊ3�Σ��յ���ȷ��������ü�����
	u16 FaultEnterSHIPDelay;									// SHоƬ����ʱ������͹���ģʽǰ��ʱ
    u16 I2CTryRecoverFlag;                                      // I2C���߳��������޸�ʱ��λ���޸�������㡣
};

struct AO_SH36730x0_Output {                                    //��������
    s32 SingleVoltage[15];                                      // ��ص����ѹ����λmV�����10��
    s32 BatteryVoltage;                                         // ����ܵ�ѹ����λmV
    s32 BatteryCurrent;                                         // ��ص�������λmA��
    s32 InnerTemperature[3];                                    // BQ769x0�¶ȴ������������ڲ��¶ȣ���λ0.1���϶�
    s32 ExternalTemperature[3];                                 // BQ769x0�¶ȴ������������ⲿ�¶ȣ���λ0.1���϶�
    s32 SingleMaxVoltage;                                       // ������ߵ�ѹ
    s32 SingleMinVoltage;                                       // ������͵�ѹ
    u32 ReadValueTime;                                          // ��ȡ����ʱ����ʱ��飬��1ms��ʱ������ֵ
    u16 SingleMaxVoltagePointer;                                // ������ߵ�ѹ��Ӧ�ڼ��ڣ�1-7
    u16 SingleMinVoltagePointer;                                // ������ߵ�ѹ��Ӧ�ڼ��ڣ�1-7
};

// -------------------------------------------------- �SH36730x0�Ĵ����ṹ�� -------------------------------------------------------------------------------------
// 0x00��SYS_STAT ϵͳ��־�Ĵ���   ��λֵ��0 
struct SH36730x0_FLAG1_bits {
    u8  TWI:1;                                                  // 1��TWIͨѶ���� Timeout
    u8  WDT:1;                                                  // 1�����Ź����
    u8  OV:1;                                                   // 1������Ӳ������籣��
    u8  SC:1;                                                   // 1������Ӳ����·����
	u8  rsvd:4;                                                 //��λֵ
                                           
};

union SH36730x0_FLAG1_union {                                   //�������б��Ա����һ����ַ
    u8                              all;
    struct  SH36730x0_FLAG1_bits   bit;
};



// 0x01��FLAG2  ��־λ�Ĵ���2  ��λֵ;0

struct SH36730x0_FLAG2_bits {
    u8  VADC:1;   //  1��VADC �жϷ�����־λ  ��ȡ��Ӳ�����
    u8  CADC:1;   //  1��CADC �жϷ�����־λ  ��ȡ��Ӳ�����
    u8  RST:1;    //  1��V33  Reset������־   ��ȡ��Ӳ�����
	u8  rsvd:5;

};

union SH36730x0_FLAG2_union {
    u8                              all;
    struct  SH36730x0_FLAG2_bits   bit;
};

// 0x02��BSTATUS ״̬�Ĵ���  ��λֵ;0
struct SH36730x0_BSTATUS_bits {
    u8  CHGR:1;     // 1�����������״̬λ
    u8  LOAD:1;     // 1����������״̬λ
    u8  CHGING:1;   // 1�����״̬λ
    u8  DSGING:1;   // 1: �ŵ�״̬λ
    u8  CHG:1;      // 1�����MOS����״̬λ
    u8  DSG:1;      // 1���ŵ�MOS����״̬λ
	u8  rsvd:2;
};

union SH36730x0_BSTATUS_union {
    u8                              all;
    struct  SH36730x0_BSTATUS_bits   bit;
};

// 0x03��INT_EN  ALARM ���ƼĴ���
struct SH36730x0_INT_EN_bits {
    u8  TWI_INT:1;      //1��TWI Timeout�ж�ʹ��λ 
    u8  WDT_INT:1;      //1: ���Ź�WDT�ж�ʹ��Ϊ
    u8  VADC_INT:1;     //1: VADCת���ж�ʹ�ܣ�ת����ɺ�ALARM�ܽ�����͵�ƽ���壩
    u8  CADC_INT:1;     //1: CADCת���ж�ʹ�ܣ�ת����ɺ�ALARM�ܽ�����͵�ƽ����
    u8  CD_INT:1;       //1: ��ŵ�״̬�ж�ʹ��λ ��⵽��ŵ�� ALARM�ܽ���������ź�
    u8  OV_INT:1;       //1: Ӳ�����䱣���ж�ʹ��λ
	u8  SC_INT:1;       //1: Ӳ����·�����ж�ʹ��λ
	u8  rsvd:1;
};

union SH36730x0_INT_EN_union {
    u8                              all;
    struct  SH36730x0_INT_EN_bits   bit;
};

// 0x04: SCONF1  ϵͳ���ƼĴ���1
struct SH36730x0_SCONF1_bits {
    u8  CHGR_EN:1;        //1����������ʹ��λ
    u8  LOAD_EN:1;        //1�����ؼ��ʹ�ܿ���λ
    u8  OV_EN:1;          //1��Ӳ�����䱣��ʹ�ܿ���λ
    u8  SC_EN:1;          //1��Ӳ����·����ʹ�ܿ���λ
    u8  WDT_EN:1;         //1: ���Ź�ʹ�ܿ���λ
    u8  PD_EN:1;          //1���͹���״̬����λ
    u8  CTLD_EN:1;        //1: CTLD���ܿ���λ  (ʹ��ʱCTLD�ܽ����ȿ���DSG(�ŵ�MOS)�ܽŹ���)
	u8  LTCLR:1;          //1: LTCLR �����־�Ĵ�����FLAG1��
};

union SH36730x0_SCONF1_union {
    u8                              all;
    struct  SH36730x0_SCONF1_bits  bit;
};

// 0x05: SCONF2 ϵͳ���ƼĴ���2
struct SH36730x0_SCONF2_bits {
    u8  CHG_C:1;          // 1�����MOS����λ
    u8  DSG_C:1;          // 1���ŵ�MOS����λ
    u8  ALARM_C:1;        // 1: ALARM���ѡ��λ ��0����͵�ƽ���� 1 ����͵�ƽ�� 
    u8  RESETorPF:1;      // 1����λ�ⲿMCU/��������ѡ��λ
	u8  rsvd:4;
                         
};

union SH36730x0_SCONF2_union {
    u8                              all;
    struct  SH36730x0_SCONF2_bits  bit;
};

// 0x06: SCONF3 ϵͳ���ƼĴ���3
struct SH36730x0_SCONF3_bits {
    u8  SCAN_C:3;         // VADCת������ѡ��λ  �ֲ�46ҳ
    u8  VADC_C:1;         // 1��VADC������ʽѡ��λ 0������ѹ�ɼ� 1������ѹ���¶Ȳɼ�
    u8  VADC_EN:1;        // 1��VADCʹ�ܿ���λ
    u8  CBIT_C:1;         // 1��CADC����ѡ��λ 0->10 bit  1->13 bit
    u8  CADC_M:1;         // 1: CADC������ʽѡ��λ 0���βɼ���ɺ�ENλ�Զ�����  �����ɼ�
    u8  CADC_EN:1; 	      // 1: CADCʹ�ܿ���λ
};

union SH36730x0_SCONF3_union {
    u8                              all;
    struct  SH36730x0_SCONF3_bits   bit;
};

// 0x07: SCONF4  
struct SH36730x0_SCONF4_bits {
    u8  CB6:1;            //1:ƽ���·����λ 6 
    u8  CB7:1;            //1:ƽ���·����λ 7         
    u8  CB8:1;            //1:ƽ���·����λ 8
	u8  CB9:1;            //1:ƽ���·����λ 9
	u8  CB10:1;           //1:ƽ���·����λ 10
	u8  rsvd:3;
};

union SH36730x0_SCONF4_union {
    u8                              all;
    struct  SH36730x0_SCONF4_bits   bit;
};

// 0x08: SCONF5
struct SH36730x0_SCONF5_bits {
    u8  CB1:1;            //1:ƽ���·����λ 1 
    u8  CB2:1;            //1:ƽ���·����λ 2         
    u8  CB3:1;            //1:ƽ���·����λ 3
	u8  CB4:1;            //1:ƽ���·����λ 4
	u8  CB5:1;            //1:ƽ���·����λ 5                                         // 0-3��Ƿѹ��ʱ
	u8  rsvd:3;
};

union SH36730x0_SCONF5_union {
    u8                              all;
    struct  SH36730x0_SCONF5_bits   bit;
};

// 0x09: SCONF6
struct SH36730x0_SCONF6_bits {
    u8  SCTn:2;            //[0:1]Ӳ����·������ʱ����ѡ��λ
    u8  SCVn:2;            //[2:3]Ӳ����·������ѹ����ѡ��λ         
    u8  RSTn:2;            //[4:5]��λ�ⲿMCU������ѡ��λ
	u8  RSNSn:2;           //[6:7]CADC�ɼ���Χѡ��λ              
};

union SH36730x0_SCONF6_union {
    u8                              all;
    struct  SH36730x0_SCONF6_bits   bit;
};

// 0x0A: SCONF7
struct SH36730x0_SCONF7_bits {
    u8  WDTtn:2;            //[0:1]���Ź����ʱ��ѡ��λ
    u8  SHSn:2;             //[2:3]��ŵ�״̬�����ֵѡ��λ         
    u8  OVTn:3;             //[4:6]Ӳ������籣����ʱѡ��λ  ����⵽�����ҳ���ʱ�䳬������ʱ�俪����������
	u8  rsvd:1;
};

union SH36730x0_SCONF7_union {
    u8                              all;
    struct  SH36730x0_SCONF7_bits   bit;
};


// 0x0B: SCONF8
struct SH36730x0_SCONF8_bits {
    u8  OVD8:1;             //Ӳ�����䱣����ֵ   ���㷽ʽ���Ĵ���ֵ*5.86mv  //*ע�� *ע�� *ע��SCONF8/9�Ĵ�����λֵΪ1
    u8  OVD9:1;             //Ӳ�����䱣����ֵ      
    u8  rsvd:6;	
};

union SH36730x0_SCONF8_union {
    u8                              all;
    struct  SH36730x0_SCONF8_bits   bit;
};

// 0x0C: SCONF9
struct SH36730x0_SCONF9_bits {
    u8  OVD0:1;             
    u8  OVD1:1;
    u8  OVD2:1;             
    u8  OVD3:1; 
    u8  OVD4:1;             
    u8  OVD5:1; 	
    u8  OVD6:1;             
    u8  OVD7:1; 
};

// 0x0C: SCONF9
union SH36730x0_SCONF9_union {
    u8                              all;
    struct  SH36730x0_SCONF9_bits   bit;
};



// 0x0D: SCONF10
struct SH36730x0_SCONF10 {
    u8  PINn;            //���͹��Ŀ���λ ��д��PIN[7:0] =0x33 ��д��PD_EN=1 AFE�ر�LDO ����͹���
};



// 0x0E~0x21: CELLn   10���Ĵ������ֵ����ѹ
struct SH36730x0_CELLn {
    u8  CELLnH;            //[0:3]
    u8  CELLnL;            //[0:7]
};


// 0x22~0x25: TSn   �¶ȼĴ���   2���Ĵ���    ������ת����ɺ����ݸ���ΪTSn�ܽ��ϵĵ�ѹ��ѹ�ȶ�Ӧ��ֵ
struct SH36730x0_TSn {
    u8  TSnH;            //[0:3]
    u8  TSnL;            //[0:7]
};

// 0x26~0x29: TEMPn   �¶ȼĴ���   2���Ĵ���   ������ת����ɺ����ݸ���Ϊ�ڲ��¶ȼ��ֵ
struct SH36730x0_TEMPn {
    u8  TEMPnH;            //[0:3]
    u8  TEMPnL;            //[0:7]
};

// 0x2A~0x2B: CUR   �����Ĵ���     ������ת����ɺ����ݸ��£�RS2-RS1�����ɼ��ܽţ���ѹ��Ӧ����ֵ
struct SH36730x0_CUR  {
    u8  CURH;            //[0:4]  //CUR[12]Ϊ����λ 1��ʾ�ŵ� 0��ʾ���
    u8  CURL;            //[0:7]
};


//------------------------------------------------------------------------------------

struct SH36730x0_Register {
    union SH36730x0_FLAG1_union       FLAG1;                   // 0x00: ϵͳ��־�Ĵ���
    union SH36730x0_FLAG2_union       FLAG2;                   // 0x01��ϵͳ��־�Ĵ���
    union SH36730x0_BSTATUS_union     BSTATUS;                 // 0x02  ״̬�Ĵ���
    union SH36730x0_INT_EN_union      INT_EN;                  // 0x03  �ж������־λ

    union SH36730x0_SCONF1_union      SCONF1;                  // 0x04  ϵͳ���ƼĴ���1
    union SH36730x0_SCONF2_union      SCONF2;                  // 0x05   
    union SH36730x0_SCONF3_union      SCONF3;                  // 0x06
    union SH36730x0_SCONF4_union      SCONF4;                  // 0x07
    union SH36730x0_SCONF5_union      SCONF5;                  // 0x08
    union SH36730x0_SCONF6_union      SCONF6;                  // 0x09
    union SH36730x0_SCONF7_union      SCONF7;                  // 0x0A
	union SH36730x0_SCONF8_union      SCONF8;                  // 0x0B
    union SH36730x0_SCONF9_union      SCONF9;                  // 0x0C 
    struct SH36730x0_SCONF10          SCONF10;                 // 0x0D   ϵͳ���ƼĴ���10

    struct SH36730x0_CELLn            CELL[10];                // 0x0E~0x21  �����о��ѹ�Ĵ���
    struct SH36730x0_TSn              TS[2];                   // 0x22~0x25  �¶ȼĴ���
    struct SH36730x0_TEMPn            TEMP[2];                 // 0x26~0x29  �¶ȼĴ���
    struct SH36730x0_CUR              CUR;                     // 0x2A~0x2B  �����Ĵ��� 
    
};



typedef struct AO_SH36730x0Tag {                      
    QActive super;                                                     // derive from QActive
    struct  AO_SH36730x0_State        State;                           // ģ��״̬
    struct  AO_SH36730x0_Parameter    Parameter;                       // SH36730x0 оƬ����
    struct  AO_SH36730x0_Variable     Variable;                        // ģ����ʱ����
    struct  AO_SH36730x0_Output       Output;                          // �������ֵ
    struct  SH36730x0_Register        SH36730x0Register;               // SH36730x0ϵ��AFEоƬ�Ĵ����ṹ��
    u16                               Record[SH36730x0_RECORD_LENGTH]; // ״̬��ת��¼
} AO_SH36730x0;


//extern AO_SH36730x0 g_AO_SH36730x0; 


// ------------------------------ ����������� ---------------------------------
    void AO_SH36730x0_ctor(AO_SH36730x0 * const me);
                                                  
// ------------------------------ �����״̬ ---------------------------------
	static QState AO_SH36730x0_initial(AO_SH36730x0 * const me);                         // ��ʼ״̬
	static QState AO_SH36730x0_StartWait(AO_SH36730x0 * const me);                       // 1.�����ȴ�״̬
	static QState AO_SH36730x0_Normal(AO_SH36730x0 * const me);                          // 2.����״̬��ͨ���޴���
	static QState AO_SH36730x0_ReadParameter(AO_SH36730x0 * const me);                   // 3.��ȡ�������ã���ȡ�Ĵ���0x00-0x0B
	static QState AO_SH36730x0_ClearSystemState(AO_SH36730x0 * const me);                // 301.���ϵͳ״̬������״̬SYS_STAT
	static QState AO_SH36730x0_ReadParameter2(AO_SH36730x0 * const me);                  // 31.��ȡ��������2����ȡ�Ĵ���0x50-0x51
	static QState AO_SH36730x0_ReadParameter3(AO_SH36730x0 * const me);                  // 32.��ȡ��������3����ȡ�Ĵ���0x59
	static QState AO_SH36730x0_WaitWriteParameter(AO_SH36730x0 * const me);              // 321.������ȡ��ɣ��ȴ�EEPROM��ȡϵͳ������ɣ����ɶ�BQ769оƬ��������
	static QState AO_SH36730x0_WriteParameter(AO_SH36730x0 * const me);                  // 33.����ϵͳ����
	static QState AO_SH36730x0_Idle(AO_SH36730x0 * const me);                            // 4.����״̬
	static QState AO_SH36730x0_ReadValue(AO_SH36730x0 * const me);                       // 9.��ȡ������Ϣ
	static QState AO_SH36730x0_UpdateControl(AO_SH36730x0 * const me);                   // 10.���¿���ֵ��������������س�ŵ��·
	static QState AO_SH36730x0_EnterSHIP(AO_SH36730x0 * const me);                       // 12.����͹���״̬
	static QState AO_SH36730x0_Fault(AO_SH36730x0 * const me); 
    static QState AO_SH36730x0_CloseBalance(AO_SH36730x0 * const me);	                 //��ȡ��ѹǰ�رվ���
    static QState AO_SH36730x0_ReadConfig(AO_SH36730x0 * const me);                     //��ȡ����״̬
   
// End 
#endif

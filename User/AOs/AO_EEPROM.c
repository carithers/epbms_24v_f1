/* ==================================================================================

 File name:     AO_EEPROM.c
 Originator:    BLJ
 Description:   EEPROM״̬������ȡд�������Ϣ���ڸ�һ�1�7?s��ʱ�����������м�¼
 Take care�1�7?	����״̬����󣬶�ʱ�1�7?���������Զ��1�7?

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 06-30-2016     Version 0.9.1       ���ڲ��԰汾
 04-12-2016     Version 0.9.0       ���Թ���ͨ��
 12-07-2014     Version 0.0.1       ���Թ���ͨ��
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                         // STM32�����Ĵ�������ͷ�ļ�
#include "string.h"                             // ����memset�����1�7?
#include "system_core.h"                        // ϵͳ����ͷ�Ą1�7?
#include "qpn_port.h"                           // ״̬��ͷ�Ą1�7?

// ---------------------- ״̬�����м�¼�������� -----------------------------
//#define   AO_DEBUG    1

//  ID


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

#endif              // ���Դ������


// -------------------------------- ���Ա��� ---------------------------------

// ------------------------------ ģ���ڲ��ɵ��ù��ܺ��1�7?--------------------------------
void SystemRecord_Update(AO_EEPROM * const me);                 // ϵͳ���м�¼���º����1�7?s����һ�1�7?

    
// ------------------------------ EEPROM�������� ---------------------------- 
// 64K flash MM32SPIN06 ȡ���5ҳ��ÿҳ1K����EEPROM   
// Page0�����й�����ʷ��¼�͹��ϴ�����¼
// Page1��ϵͳ��Ϣ��У��ֵ��Check Inf.)
// Pag:2-3: ���м�¼g_SystemRecord��ռ2ҳ������д��
// Page4: ϵͳ����

// EEPROMд���ַ                                                       1K                 * 2
const u32   EEPROM_Block_addr[] = { FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH * 2,     \
                                    FLASH_EEPROM_START_ADDRESS,     \
                                    FLASH_EEPROM_START_ADDRESS,     \
                                    FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH,     \
                                    FLASH_EEPROM_START_ADDRESS + FLASH_EEPROM_BLOCK_LENGTH * 4,     \
                                    0x0000};

// EEPROM������RAM�еĵ�ַ
/*
g_SystemRecord;// �豸���м�¼
g_SystemFaultHistory;// ϵͳ���ϼ�¼
g_SystemState;// ϵͳ״̬.EEPROM�е�У����Ϣ
g_SystemParameter;// ϵͳ����
g_SystemMonitor;// ϵͳ����ֵ	
g_SystemRecord;// �豸���м�¼									
*/															
u16*    EEPROM_Block_pBuf[] = {(u16*)&g_SystemRecord,       \
                               (u16*)&g_SystemFaultHistory,        \
                               (u16*)&g_SystemRecord,      \
                               (u16*)&g_SystemState.Variable.EEPROMCheckValue,   \
                               (u16*)&g_SystemParameter,       \
                               (u16*)&g_SystemRecord};

// ��������,��λΪ���֣�2BYTES
// Take care: ���鳤�ȱ�����ȷ����������
// Take care����0Ϊ���м�¼��Ĭ��ռ��30Byte����15�����֣���LRCУ�飬ռ1�����֣��ܳ�16������
// Take care��MCU ID ������Ĭ�ϳ���Ϊ6������
const u16   EEPROM_Block_length[] = {15,                \
                                    sizeof(g_SystemFaultHistory)/2,             \
                                    7,              \
                                    6,              \
                                    sizeof(g_SystemParameter)/2,                \
                                    1024};

// ��Ӧ����LRCУ��ʹ��λ
const u16   EEPROM_Block_LRCEnable[] = {1, 0, 0, 0, 1, 0};


// -------------------------------- ������� ------------------------------------
void AO_EEPROM_ctor(AO_EEPROM * const me) {
    QActive_ctor(&me->super, Q_STATE_CAST(&AO_EEPROM_initial));
}

// -------------------------------- ��ʼ״�1�7?------------------------------------
QState AO_EEPROM_initial(AO_EEPROM * const me) {
    return Q_TRAN(&AO_EEPROM_StartWait);
}

// ------------------------------- 1.�����ȴ���һ��ʱ�䣬�ٶ�ȡEEPROM ----------------------------------
QState AO_EEPROM_StartWait(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(1);
#endif
                
            QActive_armX((QActive *)me, 0U, 5U);      		// ������ʱ��0  5ms==================================================�������ȹر�EEPROM

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:                             		// ��ʱ�1�7?�����1�7?
        {            
            status = Q_TRAN(&AO_EEPROM_FirstRead);      		// ��ת��������ȡ״̬����ȡEEPROM����        
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// ��״̬Ϊtop״�1�7?      
        }
        break;
    }
    
    return status;
}

// ------------------------- 2.������ȡ״̬����ȡEEPROM���� ---------------------------
QState AO_EEPROM_FirstRead(AO_EEPROM * const me) {
    
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
            status = Q_TRAN(&AO_EEPROM_ReadCheckInf);   		// ��ת����״̬����ȡУ����Ϣ
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ
            
            status = Q_HANDLED();
        }
        break;
																
        case Q_TIMEOUT_SIG:                             		// ��ʱ������������ʱ
        {
			
            // ������ȡEEPROM��Ϣ������ʱ�¼�ʱ��ͳһ��AO_EEPROM_FirstRead״̬���1�7?
            
            // ����FAULT_SIG������ȫ�ֹ��ϱ�־����ת������״�1�7?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_OVERTIME);
            
            status = Q_TRAN(&AO_EEPROM_Fault);          		// ��ȡУ������Ϣ��ʱ����ת��EEPROM����״�1�7?
        }
        break;
        
        case EEPROM_FAIL_SIG:                         		    // EEPROM����ʧ�ܱ�־�1�7?
        {
			
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ�1�7?
            
            // ����FAULT_SIG������ȫ�ֹ��ϱ�־����ת������״�1�7?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_FAIL);
            
            status = Q_TRAN(&AO_EEPROM_Fault);          		// ��ȡУ������Ϣ��ʱ����ת��EEPROM����״�1�7?         
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// ��״̬Ϊtop״�1�7?
        }
        break;
    }
    
    return status;
}

u16 t_id[6];

// ----------------------------- 3.��ȡУ����Ϣ ------------------------------------
QState AO_EEPROM_ReadCheckInf(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(3);
#endif
            
            // ��ȡEEPROMУ������Ϣ   6��������Ϣ
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_CHECK], 
                                EEPROM_Block_addr[EEPROM_BLOCK_CHECK], 
                                EEPROM_Block_length[EEPROM_BLOCK_CHECK], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_CHECK]);
            
            QActive_armX((QActive *)me, 0U, 500U);      		// ������ʱ��0�1�7?00ms
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                         		// EEPROMУ������ȡ�Ʉ1�7?
        {
			
			
            //status = Q_HANDLED();//����ɾ��-------------------
            
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ�1�7?
            
            // ��ȡоƬID�ţ�96�1�7?
            g_SystemState.Variable.MCUIDNumber[0] = *(__IO uint32_t*)(MCU_ID_NUMBER);
            g_SystemState.Variable.MCUIDNumber[1] = *(__IO uint32_t*)(MCU_ID_NUMBER + 4);
            g_SystemState.Variable.MCUIDNumber[2] = *(__IO uint32_t*)(MCU_ID_NUMBER + 8);   

        /*  t_id[0] = ((((((g_SystemState.Variable.MCUIDNumber[0] & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[0] >> 8) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) << 8);
            t_id[1] = (((((((g_SystemState.Variable.MCUIDNumber[0] >> 16) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[0] >> 24) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) << 8);
            
            t_id[2] = ((((((g_SystemState.Variable.MCUIDNumber[2] & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[2] >> 8) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) << 8);
            t_id[3] = (((((((g_SystemState.Variable.MCUIDNumber[2] >> 16) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[2] >> 24) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) << 8);

            t_id[4] = ((((((g_SystemState.Variable.MCUIDNumber[1] & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[1] >> 8) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) << 8);
            t_id[5] = (((((((g_SystemState.Variable.MCUIDNumber[1] >> 16) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) 
                        + ((((((((g_SystemState.Variable.MCUIDNumber[1] >> 24) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) << 8);
        */
        
            // MCU id����֤ͨ����ת����һ���裬��ת��Ѱ���������м�¼״�1�7?
            me->Variable.Variable_bits.bit.SystemRecordPage = 0;
            me->Variable.SystemRecordCircleNumber = 0;
            me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 0;       // ϵͳ���м�¼����ҳ���ҵ���־λ����1�7?
            me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 0;           // ϵͳ���м�¼���ҵ���־λ����1�7?
            me->Variable.SystemRecordCircleNumber = 0;                          // ϵͳ���м�¼���������ţ�����
            
            status = Q_TRAN(&AO_EEPROM_SearchRecord);           
            
        /*  if ((g_SystemState.Variable.EEPROMCheckValue[0] == (u16)((((((g_SystemState.Variable.MCUIDNumber[0] & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[0] >> 8) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[1] == (u16)(((((((g_SystemState.Variable.MCUIDNumber[0] >> 16) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[0] >> 24) & 0x000000FF) + 0x15) * 0x09 - 0x21) & 0x5A) + 0x63) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[2] == (u16)((((((g_SystemState.Variable.MCUIDNumber[2] & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[2] >> 8) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[3] == (u16)(((((((g_SystemState.Variable.MCUIDNumber[2] >> 16) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[2] >> 24) & 0x000000FF) + 0x19) * 0x05 - 0x25) & 0x5F) + 0x69) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[4] == (u16)((((((g_SystemState.Variable.MCUIDNumber[1] & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[1] >> 8) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) << 8))
                && (g_SystemState.Variable.EEPROMCheckValue[5] == (u16)(((((((g_SystemState.Variable.MCUIDNumber[1] >> 16) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) 
                                                                    + ((((((((g_SystemState.Variable.MCUIDNumber[1] >> 24) & 0x000000FF) + 0x13) * 0x13 - 0x02) & 0x55) + 0x62) & 0x000000FF) << 8)))
            {
                // MCU id����֤ͨ����ת����һ���裬��ת��Ѱ���������м�¼״�1�7?
                me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 0;       // ϵͳ���м�¼����ҳ���ҵ���־λ����1�7?
                me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 0;           // ϵͳ���м�¼���ҵ���־λ����1�7?
                me->Variable.SystemRecordCircleNumber = 0;                          // ϵͳ���м�¼���������ţ�����
                status = Q_TRAN(&AO_EEPROM_SearchRecord);   
            }
            else                                        		// У����������ת��fault״�1�7?
            {
                // ����EEPROMУ���ȡ�����־λ��FAULT_EEPROM_FAILURE
                Protect_SetFaultCodeLv0(&g_Protect, FAULT_EEPROM_FAILURE);                  
                
                // ��ת��EEPROM��ʼ��״̬������EEPROM��Ȼ������д���ʼ���1�7?
                status = Q_TRAN(&AO_EEPROM_Initial);    

                // ��ת������״�1�7?
            //  status = Q_TRAN(&AO_EEPROM_Fault);
            //  status = Q_TRAN(&AO_EEPROM_SearchRecord);   
                
            }*/
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);         	// ��״̬ΪAO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 4.Ѱ�����µ��г���¼ ------------------------------------
// Page1-2��Ϊ���м�¼ʹ�ã�ÿһ����¼����Ϊ16BYTE��һҳ�ɴ��64���¼����ҳ����Ϊ���1�7?
// �����߼����ϵ�ʱ����������Ҫ��һҳ��ϵͳ����˯��ʱ��������Ҫ��һҳ�1�7?
// Take care�1�7?���޿�д������飬�����ǿ�Ʋ���һҳ�1�7?
QState AO_EEPROM_SearchRecord(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(4);
#endif
            
            // ��ȡEEPROM���м�¼һ�Σ��ӱ��1�7?-��1�7?
            // ��һ������ʱSystemRecordCircleNumber == 0
            // ��ȡ�����м�¼ֱ�Ӵ������м�¼���ݽṹ��
            // Take care��Ѱ���������м�¼����ȡ�1�7?Byte����������ʱ�䣬����LRCУ��   
            // flashģ��EEPROM��һҳ��64�����ݣ�0-63Ϊ��һҳ��64-127Ϊ���ڵĵڶ��1�7?
            // Take care����ָ�����ȡǰ2���������ݣ�����LRCУ��
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD],                          \
                                EEPROM_Block_addr[EEPROM_BLOCK_RECORD]                                          \
                                + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage   \
                                + 2 * RECORD_CIRCLE_LENGTH * me->Variable.SystemRecordCircleNumber,             \
                                2, 0);
																
            QActive_armX((QActive *)me, 0U, 500U);      		// ������ʱ��0�1�7?00ms 
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                         		// EEPROMУ������ȡ�Ʉ1�7?
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ�1�7?
           
			
            // ���ϴ��룬���޷��غ���������ת����״̬��Ԥ��QPϵͳ����
            status = Q_TRAN(&AO_EEPROM_Fault);
            
            // ���Ȳ���������������ҳ������Ҫ�������һ�1�7?
            // ��һ�ζ�ȡ��ַ0���ڶ��ζ�ȡ��ַ64���Ƚ��������1�7?
            // ���܄1�7?������һ������ֵ����һ��Ϊ0xFFFFFFFF������ֵһҳΪ����ҳ����һҳ�Ѳ���
            // ���܄1�7?������������ֵ�����һҳΪ����ҳ����һҳ��Ҫ���1�7?
            // ���܄1�7?���������1�7?xFFFFFFFF����һҳΪ����ҳ����һҳ�Ѳ���,���������ҵ�            
            
            if (me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag == 0)      // ���м�¼����ҳ��δ�ҵ�
            {
                if (me->Variable.Variable_bits.bit.SystemRecordPage == 0)           // ��һҳ��ʼ�����Ѷ�ȡ�����ȡ�ڶ��1�7?
                {
                    // ��¼��һҳ����ֵ����Ϊ����Ƚ�����
                    me->Variable.TotalTimeRecord = g_SystemRecord.TotalTime;
                    
                    // ���ҵڶ�ҳ��һ�����1�7?
                    me->Variable.Variable_bits.bit.SystemRecordPage = 1;
                    
                    status = Q_TRAN(&AO_EEPROM_SearchRecord);                       // ��ת��Ѱ���������м�¼״̬���ٴζ�ȡ���м�¼
                }
                else                    // �ڶ�ҳ��ʼ�����Ѷ�ȡ���Ƚ�������
                {
                    // ��һ�ζ�ȡ��ַ0���ڶ��ζ�ȡ��ַ64���Ƚ��������1�7?
                    // ���܄1�7?������һ������ֵ����һ��Ϊ0xFFFFFFFF������ֵһҳΪ����ҳ����һҳ�Ѳ���
                    // ���܄1�7?������������ֵ�����һҳΪ����ҳ����һҳ��Ҫ���1�7?
                    // ���܄1�7?���������1�7?xFFFFFFFF����һҳΪ����ҳ����һҳ�Ѳ���,EEPROM�����ݣ����³�ʼ�1�7?
                    // me->Variable.TotalTimeRecord�1�7?Ϊ��һҳ��ʼ��ַ����                 
                    // g_SystemRecord.TotalTime�1�7?Ϊ�ڶ�ҳ��ʼ��ַ����
                    if (g_SystemRecord.TotalTime == 0xFFFFFFFF && me->Variable.TotalTimeRecord == 0xFFFFFFFF)
                    {
                        // EEPROM�������ݣ�ִ�����ݳ�ʼ������
                        // �˴�ֱ����ת��AO_EEPROM_FixRecord��ִ�г�ʼEEPROM����д���޸�����
                       
                        // �����܄1�7?
                        me->Variable.Variable_bits.bit.SystemRecordPage = 0;
                        me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 1;       // ��λ���м�¼����ҳ���ҵ���־λ
                        
                        // ϵͳ���м�¼�������҄1�7?
                        me->Variable.SystemRecordCircleNumber = 0;
                        me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 1;           // ��λ���м�¼���ҵ���־λ
                        
                    //    status = Q_TRAN(&AO_EEPROM_ReadRecord);                           // ��ת����ȡ���м�¼״̬����ȡ���м�¼
                        
                        // ���ϵͳ���м�¼,��д�1�7?
                        memset(&g_SystemRecord, 0, sizeof(g_SystemRecord));                 // �豸���м�¼
                        
                        // ���м�¼�޷���ȡʱ��Ĭ�����õ�ص����1�7?,��һ�γ��������Զ����õ���
                        g_AO_BMS.Output.BatteryCapacity = 0;
                        
                        // ��ת���޸����м�¼״̬����������м�¼�����־λ
                        me->Variable.Variable_bits.bit.LastRecordLRCFail = 0;
                        
                        status = Q_TRAN(&AO_EEPROM_FixRecord);      // ��ת���޸����м�¼״�1�7?                     
                    }
                    else if (g_SystemRecord.TotalTime != 0xFFFFFFFF && me->Variable.TotalTimeRecord == 0xFFFFFFFF)
                    {
                        // �����܄1�7?
                        me->Variable.Variable_bits.bit.SystemRecordPage = 1;
                        me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 1;       // ��λ���м�¼����ҳ���ҵ���־λ     

                        status = Q_TRAN(&AO_EEPROM_SearchRecord);                   // ��ת��Ѱ���������м�¼״̬���ٴζ�ȡ���м�¼                       
                    }
                    else if (g_SystemRecord.TotalTime == 0xFFFFFFFF && me->Variable.TotalTimeRecord != 0xFFFFFFFF)
                    {
                        // �����܄1�7?
                        me->Variable.Variable_bits.bit.SystemRecordPage = 0;
                        me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 1;       // ��λ���м�¼����ҳ���ҵ���־λ

                        status = Q_TRAN(&AO_EEPROM_SearchRecord);                   // ��ת��Ѱ���������м�¼״̬���ٴζ�ȡ���м�¼
                    }       
                    else
                    {
                        // ���܄1�7?,������һ�1�7?
                        if (g_SystemRecord.TotalTime > me->Variable.TotalTimeRecord)
                        {
                            // �1�7?ҳ��ֵ���ڵ�1ҳ��ֵ�������1�7?ҳ���ݣ��˴������mcu 20ms��ͣ
                            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_RECORD]);

                            me->Variable.Variable_bits.bit.SystemRecordPage = 1;            // �������ݴ���ڄ1�7?ҳ�У����1�7?                              
                        }
                        else
                        {
                            // �1�7?ҳ��ֵ���ڵ�2ҳ��ֵ�������1�7?ҳ���ݣ��˴������mcu 20ms��ͣ
                            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_RECORD] + FLASH_EEPROM_BLOCK_LENGTH);

                            me->Variable.Variable_bits.bit.SystemRecordPage = 0;            // �������ݴ���ڄ1�7?ҳ�У����1�7?   
                        }
                        
                        me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag = 1;       // ��λ���м�¼����ҳ���ҵ���־λ
                        
                        status = Q_TRAN(&AO_EEPROM_SearchRecord);                           // ��ת��Ѱ���������м�¼״̬���ٴζ�ȡ���м�¼
                    }
                }
            }
            else                // ���м�¼����ҳ���҄1�7?me->Variable.Variable_bits.bit.SystemRecordPageFoundFlag == 1
            {   
                // ��β����������1�7?        
                // �������м�¼��3�ֿ��ܣ��о�����
                // ���܄1�7?���±������ʱ��1�7?xFFFFFFFF
                // ���܄1�7?���ѱ���ȫ�����м�¼�������һ�����м�¼�����¼Ǆ1�7?
                
                if (me->Variable.SystemRecordCircleNumber == 0)                 // ��ȡ���0��ֵ��ɣ����������1�7?�������������ϲ��ܱȄ1�7?
                {
                    me->Variable.SystemRecordCircleNumber++;                    // ��ȡ���1����
                    
                    status = Q_TRAN(&AO_EEPROM_SearchRecord);                   // ��ת��Ѱ���������м�¼״̬���ٴζ�ȡ���м�¼                   
                }
                else                                                            // ���1-63���ݶ�ȡ��ɣ�����һ�����ݽ��бȄ1�7?
                {                    
					if (g_SystemRecord.TotalTime == 0xFFFFFFFF)
                    {
                        // ���¶�ȡ���݄1�7?xFFFFFFFF������һ������Ϊ�������1�7?
                        
                        // Take care���˴�SystemRecordCircleNumber������1��������ָ����һ������λ�1�7?
                        me->Variable.SystemRecordCircleNumber--;                
                        me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 1;   // ��λ���м�¼���ҵ���־λ
                        
                        status = Q_TRAN(&AO_EEPROM_ReadRecord);                     // ��ת����ȡ���м�¼״̬����ȡ���м�¼
                    }
                    else
                    {
                        // ���¶�ȡ����ֵ���1�7?xFFFFFFFF���������ȡ��һ�����1�7?
                        me->Variable.SystemRecordCircleNumber++;                    
                        if (me->Variable.SystemRecordCircleNumber < RECORD_CIRCLE_NUMBER)       // ��Ҫ��ȡ����1�7?
                        {
                            status = Q_TRAN(&AO_EEPROM_SearchRecord);                   // ��ת��Ѱ���������м�¼״̬���ٴζ�ȡ���м�¼
                        }
                        else                            
                        {
                            // �����һ�������Բ�Ϊ0xFFFFFFFF�������һ������Ϊ�������1�7?
                            // Take care�1�7?���м�¼���Ϊѭ�����ȼ�1����Ϊ��Ŵ�0��ʼ���1�7?
                            me->Variable.SystemRecordCircleNumber = RECORD_CIRCLE_NUMBER - 1;
                            
                            me->Variable.Variable_bits.bit.SystemRecordFoundFlag = 1;   // ��λ���м�¼���ҵ���־λ
                             
                            status = Q_TRAN(&AO_EEPROM_ReadRecord);                     // ��ת����ȡ���м�¼״̬����ȡ���м�¼
                        }
                    }
                }
            }
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // ��״̬ΪAO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 5.��ȡ�г���¼ ------------------------------------
QState AO_EEPROM_ReadRecord(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(5);
#endif        
            
            // ��ȡEEPROM���м�¼һ�1�7?
            // ��ȡ�����м�¼ֱ�Ӵ������м�¼���ݽṹ��
            // Take care����ָ����LRCУ��
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD], \
                                EEPROM_Block_addr[EEPROM_BLOCK_RECORD] 
                                + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage 
                                + 2 * RECORD_CIRCLE_LENGTH * me->Variable.SystemRecordCircleNumber, \
                                EEPROM_Block_length[EEPROM_BLOCK_RECORD], 1);
                
            QActive_armX((QActive *)me, 0U, 500U);                  // ������ʱ��0�1�7?00ms 
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROM��ȡ�ɹ�,��У�����1�7?
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            // ���м�¼��ȡ��ɣ������м�¼���ݸ����ģ��
            g_AO_BMS.Output.BatteryCapacity = g_SystemRecord.BatteryCapacity;
            g_AO_BMS.Output.SOC = g_SystemRecord.SOC;
//            if(g_AO_BMS .Output.SOC <= 120)
//            {
//                if(g_AO_BMS.Output.SOC == 0)
//                {
//                    g_AO_BMS.Output.StartSOC = 1;
//                }
//                else
//                {
//                    g_AO_BMS.Output.StartSOC = g_SystemRecord.SOC;
//                }
//            }
            // �����м�¼������ʱ����Ϊϵͳ����ʱ�䣬������ȡ����ǰʱ���ȥ����ʱ�䣬��ϵͳ����������ʱ�1�7?
            // �����ݿ���ΪOEM����ʱ�����Ƅ1�7?
            g_SystemState.Output.SystemStartTime = g_SystemRecord.TotalTime;

            if (me->Variable.Variable_bits.bit.LastRecordLRCFail == 0)      // ���һ�����м�¼LRCУ��ʧ�ܱ�־λΪ0���������1�7?
            {
                status = Q_TRAN(&AO_EEPROM_ReadParameter);                  // EEPROM��Ϣ��ȡ���,��ת��AO_EEPROM_ReadParameter״�1�7?
            }
            else
            {       
                // ���һ�����м�¼У��������ѻ����һ����ȷ�����м�¼����Ҫ�޸��������򣬷��������Ժ����ݻ���
                // ��ת���޸����м�¼״̬����������м�¼�����־λ
                me->Variable.Variable_bits.bit.LastRecordLRCFail = 0;
                
                status = Q_TRAN(&AO_EEPROM_FixRecord);              // ��ת���޸����м�¼״�1�7?
            }
        }
        break;
        
        case EEPROM_LRC_FAIL_SIG:                                   // EEPROM��ȡ����LRCУ�����
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            // For test�����ڼ�¼����ʱ�����˶��ٴ����м�¼LRCУ�����
            me->Variable.RecordLRCFail++;                           // ���м�¼LRCУ��������
                
            // ���ϴ���
        //    status = Q_HANDLED();
            
            // ��λ���һ�����м�¼LRCУ������־�1�7?
            me->Variable.Variable_bits.bit.LastRecordLRCFail = 1;
            
            if (me->Variable.SystemRecordCircleNumber > 0)          // ϵͳ���м�¼��Ų�Ϊ0����������һ�����1�7?
            {
                me->Variable.SystemRecordCircleNumber--;
                
            //    me->Variable.SystemRecordRetryCount++;            // ϵͳ���м�¼���¶�ȡ����++
                
                status = Q_TRAN(&AO_EEPROM_ReadRecord);             // ��ת����ȡ���м�¼״̬����ȡ���м�¼               
            }
            else                        // �Ѳ�������������ݣ��Դ���������ϵͳ���м�¼�����м�¼���׷τ1�7?
            {       
                // ���ϵͳ���м�¼,��д�1�7?
                memset(&g_SystemRecord, 0, sizeof(g_SystemRecord));         // �豸���м�¼
                
                // ���м�¼�޷���ȡʱ��Ĭ�����õ�ص����1�7?,��һ�γ��������Զ����õ���
                g_AO_BMS.Output.BatteryCapacity = 0;
                g_AO_BMS.Output.SOC = 0;   
                
                // ϵͳ����ʱ��Ҳ��1�7?
                g_SystemState.Output.SystemStartTime = 0;
                
                // ��ת���޸����м�¼״̬����������м�¼�����־λ
                me->Variable.Variable_bits.bit.LastRecordLRCFail = 0;
                
                status = Q_TRAN(&AO_EEPROM_FixRecord);              // ��ת���޸����м�¼״�1�7?
            }
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // ��״̬ΪAO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ---------------------------- 51.���һ�����м�¼�����޸��1�7?---------------------------------
QState AO_EEPROM_FixRecord(AO_EEPROM * const me) {
    
    QState status;
    
    u16 l_cnt = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(51);
#endif
                  
            // ����Record��ʹ�ñ��1�7?�1�7?��ҳ��ҳ�����AO_EEPROM.c�1�7?0�1�7?
            // Take care: ����flashEEPROMΪ��ʱָ�����ʱ�����CPUֹͣ������ɾ�1�7?ҳflash��Լ40ms
            for (l_cnt = 0; l_cnt < 2; l_cnt++)
            {
                flash_EEPROM_Erase(&g_flash_EEPROM, (EEPROM_Block_addr[EEPROM_BLOCK_RECORD] + l_cnt * FLASH_EEPROM_BLOCK_LENGTH));
            }

            // ���record��Ȼ��д���һҳ����һ�1�7?
            me->Variable.Variable_bits.bit.SystemRecordPage = 0;
            me->Variable.SystemRecordCircleNumber = 0;

            // д�����м�¼��Ĭ��д���һҳ��һ�����������д���ַ
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD], 
                                EEPROM_Block_addr[EEPROM_BLOCK_RECORD], \
                                EEPROM_Block_length[EEPROM_BLOCK_RECORD], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_RECORD]);

            QActive_armX((QActive *)me, 0U, 500U);                  // ������ʱ��0�1�7?00ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:                
            // EEPROM�����ɹ�
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
    
            // �����м�¼������ʱ����Ϊϵͳ����ʱ�䣬������ȡ����ǰʱ���ȥ����ʱ�䣬��ϵͳ����������ʱ�1�7?
        //    g_SystemState.Output.SystemStartTime = g_SystemRecord.TotalTime;            
            
            status = Q_TRAN(&AO_EEPROM_ReadParameter);              // ��תAO_EEPROM_ReadParameter״�1�7?
        }
        break;      
		
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // ��״̬ΪAO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 6.��ȡ��������������Ϣ ------------------------------------
QState AO_EEPROM_ReadParameter(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(6);
#endif          
            
            // ��ȡEEPROM����һ�1�7?
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], \
                                EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], 1);
                    
            QActive_armX((QActive *)me, 0U, 1000U);                 // ������ʱ��0�1�7?000ms,�������ܽ϶࣬Ԥ������ʱ��ӄ1�7?
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROM��ȡ�ɹ�
        {
            QActive_disarmX((QActive *)me, 0U);                     

            // ������ϵͳ�����ĵ�һ�����豸���ƣ���Ϊ����ֵ�����޸��������п��ܲ���δ������ȡ�����δ��ʼ��
            if (g_SystemParameter.System.Information.DeviceName[0] == 0x0000
                || g_SystemParameter.System.Information.DeviceName[0] == 0xFFFF)
            {
                // Lv0: ����LRCУ��ͨ�������ǲ������ݴ��󣬿���δ����ʼ��������ȫΪ0
                Protect_SetFaultCodeLv0(&g_Protect, EEPROM_PARAMETER_WRONG);        //����LV0���Ϻ���
                
                status = Q_TRAN(&AO_EEPROM_FixParameter);           // ����δ����ʼ������ת���޸���������������Ϣ
            }
            else if (g_SystemParameter.System.Information.SoftwareVersion != CONST_SOFTWARE_VERSION)//�ж�����汾
			{
				status = Q_TRAN(&AO_EEPROM_UpdateInformation);           // ��ת�����²���״
			}
			else
            {
                // ϵͳ��������������ʼ������ģ   ����EEPROM����������ϵͳģ�����
                System_ParameterSet();
                
                // ��SH36730x0ģ�鷢�����������¼�����������SH36730x0оƬ
                // Take care�������EEPROM��ȡ�����ɹ����ٷ���BQ769x0�����START�¼�������SH36730x0оƬ�������ò���
                //QACTIVE_POST((QActive *)&g_AO_SH36730x0, START_SIG, 0);   
                	
                status = Q_TRAN(&AO_EEPROM_ReadFaultHistory);       // ��תAO_EEPROM_ReadFaultHistory 
            }
        }
        break;
        
        case EEPROM_LRC_FAIL_SIG:                                   // EEPROM��ȡ����LRCУ�����
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?   
            
            // Lv0: ����LRCУ�����
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_PARAMETER_LRC_FAIL);
            
            status = Q_TRAN(&AO_EEPROM_FixParameter);               // ����LRCУ��ʧ�ܣ���ת���޸�����״�1�7? 
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // ��״̬ΪAO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 61.�޸���������������Ϣ ------------------------------------
QState AO_EEPROM_FixParameter(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(61);
#endif
            
            // ϵͳ������ʼ��ΪĬ�τ1�7?
            memset(&g_SystemParameter, 0, sizeof(g_SystemParameter));       // �Ƚ�����ȫ������
            SystemParameter_Init();                                         // ��д��Ĭ�ϲ΄1�7?
            
            // ����������������flash���˲��������mcu ֹͣ�����1�7?0ms
            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER]);
                        
            // д��EEPROM����һ�1�7?
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], \
                                EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);
                    
            QActive_armX((QActive *)me, 0U, 1000U);     // ������ʱ��0�1�7?000ms���˲������ܺ�ʱ�ϳ�
                                
            status = Q_HANDLED();           
        }
        break;

        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROMд��ɹ�
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?

            // ϵͳ��������������ʼ������ģ�1�7?
            System_ParameterSet();
            
            // ��BQ769x0ģ�鷢�����������¼�����������BQ769x0оƬ
            // Take care�������EEPROM��ȡ�����ɹ����ٷ���BQ769x0�����START�¼�������BQ769x0оƬ�������ò���
        //    QACTIVE_POST((QActive *)&SH36730x0, START_SIG, 0);   
            
            status = Q_TRAN(&AO_EEPROM_ReadFaultHistory);           // ��תAO_EEPROM_ReadFaultHistory״�1�7?      
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // ��״̬ΪAO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 63.���²�������������Ϣ������汾���1�7?------------------------------------
QState AO_EEPROM_UpdateInformation(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(63);
#endif
            
            // ����汾���º��Զ�������ϼ�¼
            // ע�⣺flashд��ǰ�����ҳ���˲���Ϊ��ʱ���������ᵼ��cpu��ͣ����
            // ������������PWMʹ��ʱ���Ͻ�����flash���ᵼ��PWM��ͣ����
            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY]);             
            
            // ���²�����������汾���1�7?
            g_SystemParameter.System.Information.SoftwareVersion = CONST_SOFTWARE_VERSION;
			
            
            
            // ����������������flash���˲��������mcu ֹͣ�����1�7?0ms
            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER]);
                        
            // д��EEPROM����һ�1�7?
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], \
                                EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);
                    
            QActive_armX((QActive *)me, 0U, 1000U);     // ������ʱ��0�1�7?000ms���˲������ܺ�ʱ�ϳ�
                                
            status = Q_HANDLED();           
        }
        break;

        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROMд��ɹ�
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?

            // ϵͳ��������������ʼ������ģ�1�7?
            System_ParameterSet();
            
            // ��BQ769x0ģ�鷢�����������¼�����������BQ769x0оƬ
            // Take care�������EEPROM��ȡ�����ɹ����ٷ���BQ769x0�����START�¼�������BQ769x0оƬ�������ò���
        //    QACTIVE_POST((QActive *)&SH36730x0, START_SIG, 0);   
            
            status = Q_TRAN(&AO_EEPROM_ReadFaultHistory);           // ��תAO_EEPROM_ReadFaultHistory״�1�7?      
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // ��״̬ΪAO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 62.��ȡ������ʷ��¼ ------------------------------------
QState AO_EEPROM_ReadFaultHistory(AO_EEPROM * const me) {
    
    QState status;
    
    s16 i = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(62);
#endif
            
            // ��ȡ������ʷ��¼һ�1�7?
            flash_EEPROM_Read(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_FAULT_HISTORY], \
                                EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY], 
                                EEPROM_Block_length[EEPROM_BLOCK_FAULT_HISTORY], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_FAULT_HISTORY]);
                    
            QActive_armX((QActive *)me, 0U, 1000U);                 // ������ʱ��0�1�7?000ms,�˲������ܺ�ʱ�ϳ�
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_FINISH_SIG:                                     // EEPROM��ȡ�ɹ�
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ��0

            // To be delete
            // Take care�1�7?�ɰ�����1�7?.2.2.0002��֮ǰ���������°�ʱ���账��0xFF���ݣ����֮
            // �����ϴ���Ϊ0xFF���������ڣ�����������Ǆ1�7?
            for (i = FAULT_HISTORY_LENGTH; i >= 0; i--)
            {
                if (g_SystemFaultHistory.FaultHistory[i].bit.FaultCode == 0xFF)
                {
                    g_SystemFaultHistory.FaultHistory[i].all = 0;
                }
            }
            
            for (i = FAULT_COUNT_LENGTH; i >= 0; i--)
            {
                if (g_SystemFaultHistory.FaultCount[i].bit.FaultCode == 0xFF)
                {
                    g_SystemFaultHistory.FaultCount[i].all = 0;
                }
            }   
			
            // ��BQ769x0ģ�鷢�����������¼�����������BQ769x0оƬ
            // Take care�������EEPROM��ȡ�����ɹ����ٷ���BQ769x0�����START�¼�������BQ769x0оƬ�������ò���


			QACTIVE_POST((QActive *)&g_AO_SH36730x0, START_SIG, 0);   //����SH36730  �����ź�
	
	
            
            status = Q_TRAN(&AO_EEPROM_Normal);                     // ��ת��AO_EEPROM_Normal״�1�7?       
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_FirstRead);                 // ��״̬ΪAO_EEPROM_FirstRead
        }
        break;
    }
    
    return status;
}

// ----------------------------- 7.EEPROM��������״̬���ɽ���д���ȡָ�1�7?------------------------------------
QState AO_EEPROM_Normal(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(7);
#endif
            
            QActive_armX((QActive *)me, 2U, 1000U);                 // ������ʱ��2�1�7?000ms�����ڶ�ʱˢ�1�7?
                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_Idle);                       // Ĭ����ת��AO_EEPROM_Idle
        }
        break;
        
        case Q_EXIT_SIG:
        {   
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            QActive_disarmX((QActive *)me, 2U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;      
        
        case Q_TIMEOUT_SIG:                                         // ��ʱ�1�7?����������ʱ
        {
            // ��������EEPROM��Ϣ������ʱ�¼�ʱ��ͳһ��AO_EEPROM_Normal״̬���1�7?
            
            // ����FAULT_SIG������ȫ�ֹ��ϱ�־����ת������״�1�7?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_OVERTIME);
            
            status = Q_TRAN(&AO_EEPROM_Fault);                      // ��ת��EEPROM����״�1�7?
        }
        break;      

        case Q_TIMEOUT2_SIG:                                        // ��ʱ�1�7?����������ʱ
        {
            QActive_armX((QActive *)me, 2U, 1000U);                 // ������ʱ��2�1�7?000ms�����ڶ�ʱˢ�1�7?
            
            // 1s��ʱ���º�������ʱˢ�����мǄ1�7?
            SystemRecord_Update(me);
            
            status = Q_HANDLED(); 
        }
        break;
        
        case EEPROM_FAIL_SIG:
        {
            // EEPROM����ʧ���¼���ͳһ��AO_EEPROM_Normal״̬���1�7?
            
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            // ����FAULT_SIG������ȫ�ֹ��ϱ�־����ת������״�1�7?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_FAIL);
            
            status = Q_TRAN(&AO_EEPROM_Fault);                      // ��ȡУ������Ϣ��ʱ����ת��EEPROM����״�1�7?         
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                            // ��״̬Ϊtop״�1�7?
        }
        break;
    }
    
    return status;
}

// ----------------------------- 8.EEPROM����״̬���ɽ���д���ȡָ�1�7?------------------------------------
QState AO_EEPROM_Idle(AO_EEPROM * const me) {
    
    QState status;
    
    u16 i = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(8);
#endif
            
            // �˴������л�����ָ�������ٴη��1�7?
            if (me->Variable.EEPROMWriteBlockPointer > 0)           // ����ָ�������������1�7?
            {
                // ����EEPROMд��ָ���������һ��ָ������Ƚ��ȳ�
                QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, me->Variable.EEPROMWriteBlockWait[0]);
                
                // ���л��泤��--�����л�����ǰ�ƽ�һλ���������һ��ָ�����ָ����ǰ�ƶ�һ�1�7?
                me->Variable.EEPROMWriteBlockPointer--;         
                for (i = 0; i < me->Variable.EEPROMWriteBlockPointer; i++)
                {
                    me->Variable.EEPROMWriteBlockWait[i] = me->Variable.EEPROMWriteBlockWait[i + 1];                    
                }
				
				me->Variable.EEPROMWriteBlockWait[me->Variable.EEPROMWriteBlockPointer] = 0;
            }
                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {           
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_WRITE_SIG:                                      // EEPROMдָ�1�7?
        {
            me->Variable.EEPROMWriteBlock = (u16)Q_PAR(me);         // ��ȡд��ָ�������Ϊ��д�����1�7?
        
            status = Q_TRAN(&AO_EEPROM_Write);                      // ��ת��EEPROMд��״�1�7?   
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Normal);                    // ��״̬ΪAO_EEPROM_Normal״�1�7?
        }
        break;
    }
    
    return status;
}

s32 t_EEPROM[5];

// ------------------------- 9.EEPROMд��״̬���ɽ���д���ȡָ�1�7?--------------------------
QState AO_EEPROM_Write(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(9);
#endif
            
            switch (me->Variable.EEPROMWriteBlock)
            {
                case EEPROM_BLOCK_RECORD:
                {
                    // ���м�¼������ҳflash����д��ķ�ʽ��������Ӧ���1�7?
                    me->Variable.SystemRecordCircleNumber++;    // ��ҳ����д�����м�¼����++������flash��д���λ��     
                    if (me->Variable.SystemRecordCircleNumber >= RECORD_CIRCLE_NUMBER)      // ��׼������ҳ���Ƴ���
                    {       
                        // ��ǰҳд��󣬸�������һҳ����д�1�7?
                        if (me->Variable.Variable_bits.bit.SystemRecordPage == 0)
                        {
                            me->Variable.Variable_bits.bit.SystemRecordPage = 1;
                        }
                        else
                        {
                            me->Variable.Variable_bits.bit.SystemRecordPage = 0;
                        }
                            
                        // ��ҳ����д�����м�¼��������
                        me->Variable.SystemRecordCircleNumber = 0;
                        
                        // Take care: ����д���ҳ��ʼ��ַ���ݲ�Ϊ0xFFFFFFFF������������ǰ�����м�¼������Ҫ�Ȳ�����ҳ
                        // �Ǳ��ʱ��Ҫ�󲻸ߵĲ�Ʒ�����������в���flash,ϵͳ��ͣ�1�7?0ms
                        // ������������PWMʹ��ʱ���Ͻ�����flash���ᵼ��PWM��ͣ����
                        if (*(__IO uint32_t*)(EEPROM_Block_addr[EEPROM_BLOCK_RECORD] + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage) != 0xFFFFFFFF)
                        {
                            flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_RECORD] + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage);
                        }
                    }                   
                    
                    // д�����м�¼��ָ��λ�ã�һҳд����ɺ�Ż����
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD],                                 \
                                        EEPROM_Block_addr[EEPROM_BLOCK_RECORD]                                                  \
                                        + FLASH_EEPROM_BLOCK_LENGTH * me->Variable.Variable_bits.bit.SystemRecordPage           \
                                        + 2 * RECORD_CIRCLE_LENGTH * me->Variable.SystemRecordCircleNumber,                     \
                                        EEPROM_Block_length[EEPROM_BLOCK_RECORD], EEPROM_Block_LRCEnable[EEPROM_BLOCK_RECORD]);                         
                }
				break;
                
                case EEPROM_BLOCK_PARAMETER:        // ����д�룬�������ڿ��л��������ʱд�1�7?
                {
                    // ע�⣺flashд��ǰ�����ҳ���˲���Ϊ��ʱ���������ᵼ��cpu��ͣ����
                    // ������������PWMʹ��ʱ���Ͻ�����flash���ᵼ��PWM��ͣ����
                    flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER]);     
                    
                    // д��EEPROMһ�1�7?
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER],      \
                                        EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER],                      \
                                        EEPROM_Block_length[EEPROM_BLOCK_PARAMETER],                    \
                                        EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);    
                  
                }
                break;
                
                case EEPROM_BLOCK_FAULT_HISTORY:
                {
                    
                    // ע�⣺flashд��ǰ�����ҳ���˲���Ϊ��ʱ���������ᵼ��cpu��ͣ����
                    // ������������PWMʹ��ʱ���Ͻ�����flash���ᵼ��PWM��ͣ����
                    flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY]);     
                    
                    // д��EEPROMһ�1�7?
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_FAULT_HISTORY],      \
                                        EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY],                      \
                                        EEPROM_Block_length[EEPROM_BLOCK_FAULT_HISTORY],                    \
                                        EEPROM_Block_LRCEnable[EEPROM_BLOCK_FAULT_HISTORY]);                        
                }
                break;
            }       
    
            // ͳһ����һ�1�7?s��ʱ�1�7?
            QActive_armX((QActive *)me, 0U, 5000U);                 // ������ʱ��0�1�7?000ms
                                
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {   
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_WRITE_SIG:                                      // EEPROMдָ�1�7?
        {
			
            // ��ִ��д�����ʱ�յ�д��ָ�����ָ��������
            if (me->Variable.EEPROMWriteBlockPointer < EEPROM_WRITE_COMMAND_LENGTH)
            {
                me->Variable.EEPROMWriteBlockWait[me->Variable.EEPROMWriteBlockPointer] = (u16)Q_PAR(me);           // ��ȡд��ָ�������Ϊ��д�����1�7?
            
                me->Variable.EEPROMWriteBlockPointer++;
				
				t_EEPROM[0]++;
				
				if (t_EEPROM[1] < me->Variable.EEPROMWriteBlockPointer)
				{
					t_EEPROM[1] = me->Variable.EEPROMWriteBlockPointer;
				}
            }
            else
            {
                // EEPROMϣ��ָ����л�������������������������ⲻ����EEPROM�����������ⲿϵͳ����Ƶ������д�����
                Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_OVER_FLOW);
            }
            
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?          
            
            // ����д����ɺ���λоƬ���������־�1�7?���´ν���BMS_Idle����״̬ʱ������mcu����
            // To be update�1�7?�˴�����Ӧ����Ч�ˣ���ȷ�1�7?
            if (me->Variable.EEPROMWriteBlock == EEPROM_BLOCK_PARAMETER)
            {
                g_AO_BMS.State.ChipResetAsk = 1;
            }
			
			status = Q_TRAN(&AO_EEPROM_Idle);                       // EEPROM��Ϣ��ȡ���,��ת��AO_EEPROM_Idle״�1�7?
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Normal);                    // ��״̬ΪAO_EEPROM_Normal״�1�7?
        }
        break;
    }
    
    return status;
}

// ------------------------- 100.EEPROM���ϣ�����������1�7?---------------------------
QState AO_EEPROM_Fault(AO_EEPROM * const me) {
    
    QState status;
    
    u16 l_cnt = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(100);
#endif
   
            // ��λLV0������ϣ���������ΪEEPROM��дʧ��
            // �˲���Ϊ����������������״̬ǰδ���þ��������Ϣ����ͳһ��EEPROM��дʧ��
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_EEPROM_FAILURE);
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;
        
        case EEPROM_WRITE_SIG:                                      // EEPROMдָ�1�7?
        {
            // EEPROM����״̬ʱ����������ղ���д��ָ��
            switch ((u16)Q_PAR(me))
            {
                case EEPROM_BLOCK_CHECK:                            // д��У������
                {   
                    // ����Check��Ϣ��Ӧflash����ʱָ������CPU��ͣ�1�7?0ms
                    flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_CHECK]);
                    
                    // д��У������
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_CHECK], EEPROM_Block_addr[EEPROM_BLOCK_CHECK], \
                                        EEPROM_Block_length[EEPROM_BLOCK_CHECK], EEPROM_Block_LRCEnable[EEPROM_BLOCK_CHECK]);                   
                }
                break;
                
                case EEPROM_BLOCK_PARAMETER:                        // д��ϵͳ����
                {
                    // ע�⣺flashд��ǰ�����ҳ���˲���Ϊ��ʱ���������ᵼ��cpu��ͣ����
                    flash_EEPROM_Erase(&g_flash_EEPROM, EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER]);     
                    
                    // д��ϵͳ����
                    flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], 
                                        EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], \
                                        EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);                               
                }
                break;
                
                case EEPROM_BLOCK_ALL:                              // ����EEPROMȫ������
                {                       
                    // Take care: ����flashEEPROMΪ��ʱָ�����ʱ�����CPUֹͣ������ɾ�1�7?ҳflash��Լ100ms
                    for (l_cnt = 0; l_cnt < FLASH_EEPROM_BLOCK_NUMBER; l_cnt++)
                    {
                        flash_EEPROM_Erase(&g_flash_EEPROM, FLASH_EEPROM_START_ADDRESS + l_cnt * FLASH_EEPROM_BLOCK_LENGTH);
                    }           
                }
                break;
            }

            status = Q_HANDLED();
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                            // ��״̬Ϊtop״�1�7?
        }
        break;
    }
    
    return status;
}

// ---------------------------------- 20.EEPROM��ʼ�1�7?-----------------------------------------
QState AO_EEPROM_Initial(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(20);
#endif

            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_Erase);                      // ��תAO_EEPROM_Erase״�1�7?     
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            status = Q_HANDLED();
        }
        break;      
        
        case EEPROM_FAIL_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                     // �رն�ʱ�1�7?
            
            // ����FAULT_SIG������ȫ�ֹ��ϱ�־����ת������״�1�7?
            Protect_SetFaultCodeLv0(&g_Protect, EEPROM_COMMAND_FAIL);
            
            status = Q_TRAN(&AO_EEPROM_Fault);                      // ��ȡУ������Ϣ��ʱ����ת��EEPROM����״�1�7?         
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_Fault);                      // ������ʱ����ת��EEPROM����״�1�7?
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                            // ��״̬Ϊtop״�1�7?
        }
        break;
    }
    
    return status;
}

// ---------------------------------- 21.����EEPROM -----------------------------------------
QState AO_EEPROM_Erase(AO_EEPROM * const me) {
    
    QState status;
    
    u16 l_cnt = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(21);
#endif

            // ����EEPROMȫ������
            // Take care: ����flashEEPROMΪ��ʱָ�����ʱ�����CPUֹͣ����
            for (l_cnt = 0; l_cnt < FLASH_EEPROM_BLOCK_NUMBER; l_cnt++)
            {
                // Take care����ֹ����CPU ID�������飺
            //    if ((FLASH_EEPROM_START_ADDRESS + l_cnt * FLASH_EEPROM_BLOCK_LENGTH) != 0x0800E000)   
                if ((FLASH_EEPROM_START_ADDRESS + l_cnt * FLASH_EEPROM_BLOCK_LENGTH) != EEPROM_Block_addr[EEPROM_BLOCK_CHECK])                          
                {
                    flash_EEPROM_Erase(&g_flash_EEPROM, FLASH_EEPROM_START_ADDRESS + l_cnt * FLASH_EEPROM_BLOCK_LENGTH);
                }
            }

            QActive_armX((QActive *)me, 0U, 100U);                  // ������ʱ��0�1�7?00ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {
            status = Q_TRAN(&AO_EEPROM_InitialRecord);              // ��תAO_EEPROM_InitialRecord״�1�7?     
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Initial);                   // ��״̬ΪAO_EEPROM_Initial״�1�7?
        }
        break;
    }
    
    return status;
}

// ---------------------------- 22.��ʼ�����мǄ1�7?---------------------------------
QState AO_EEPROM_InitialRecord(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(22);
#endif
            
            memset(&g_SystemRecord, 0, sizeof(g_SystemRecord));         // �豸���м�¼����
            
            // д�����м�¼��Ĭ��д���һҳ��һ�����������д���ַ
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_RECORD], 
                                EEPROM_Block_addr[EEPROM_BLOCK_RECORD], \
                                EEPROM_Block_length[EEPROM_BLOCK_RECORD], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_RECORD]);
            
            QActive_armX((QActive *)me, 0U, 1000U);                     // ������ʱ��0�1�7?000ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:                                         // EEPROM�����ɹ�
        {
            QActive_disarmX((QActive *)me, 0U);                         // �رն�ʱ�1�7?

            status = Q_TRAN(&AO_EEPROM_InitialFaultHistory);            // ��תAO_EEPROM_InitialFaultHistory״�1�7?       
        }
        break;          
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Initial);                       // ��״̬ΪAO_EEPROM_Initial״�1�7?
        }
        break;
    }
    
    return status;
}

// ---------------------------- 23.��ʼ��������ʷ�Ǆ1�7?---------------------------------
QState AO_EEPROM_InitialFaultHistory(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(23);
#endif
            
            // ���������ʷ��¼
            memset(&g_SystemFaultHistory, 0, sizeof(g_SystemFaultHistory)); 
            
            // д����ϼ�¼
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_FAULT_HISTORY], 
                                EEPROM_Block_addr[EEPROM_BLOCK_FAULT_HISTORY], \
                                EEPROM_Block_length[EEPROM_BLOCK_FAULT_HISTORY], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_FAULT_HISTORY]);
            
            QActive_armX((QActive *)me, 0U, 1000U);     // ������ʱ��0�1�7?000ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:                                         // EEPROM�����ɹ�
        {
            QActive_disarmX((QActive *)me, 0U);                         // �رն�ʱ�1�7?

            status = Q_TRAN(&AO_EEPROM_InitialParameter);               // ��תAO_EEPROM_InitialParameter״�1�7?      
        }
        break;          
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Initial);                       // ��״̬ΪAO_EEPROM_Initial״�1�7?
        }
        break;
    }
    return status;
}

// ---------------------------- 25.��ʼ��ϵͳ�΄1�7?---------------------------------
QState AO_EEPROM_InitialParameter(AO_EEPROM * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(25);
#endif
            
            // ϵͳ������ʼ��ΪĬ�τ1�7?
            memset(&g_SystemParameter, 0, sizeof(g_SystemParameter));   // �Ƚ����в�����1�7?
            SystemParameter_Init();                                     // �ٸ�ֵ��ʼ�1�7?
            
            // д��ϵͳ����
            flash_EEPROM_Write(&g_flash_EEPROM, EEPROM_Block_pBuf[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_addr[EEPROM_BLOCK_PARAMETER], \
                                EEPROM_Block_length[EEPROM_BLOCK_PARAMETER], 
                                EEPROM_Block_LRCEnable[EEPROM_BLOCK_PARAMETER]);
            
            QActive_armX((QActive *)me, 0U, 1000U);                     // ������ʱ��0�1�7?000ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        { 
            status = Q_HANDLED();
        }
        break;  
        
        case EEPROM_FINISH_SIG:                                         // EEPROM�����ɹ�
        {
            QActive_disarmX((QActive *)me, 0U);                         // �رն�ʱ�1�7?
            
            // To be update�1�7?�˴���ת��EEPROM�������ʼ״̬��ͬʱ�������ٴν���Initial״̬����EEPROM��ʼ��ʧ�ܣ�������Fault״�1�7?
            status = Q_TRAN(&AO_EEPROM_Fault);                          // ��תAO_EEPROM_Fault״�1�7?����ӦУ��ֵд��ָ�1�7?  
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_EEPROM_Initial);                       // ��״̬ΪAO_EEPROM_Initial״�1�7?
        }
        break;
    }
    
    return status;
}


// ------------------------------- ����Ϊ�ڲ����ù��ܺ��1�7?-----------------------------------------



// ---------------------------------- ���м�¼��ʱ���º�����ʱ�1�7?s ---------------------------------
void SystemRecord_Update(AO_EEPROM * const me)
{       
    // ����ʱ��++
    g_SystemRecord.TotalTime++;
    
    // ��ص�������Ϣ���ݸ����м�¼
    g_SystemRecord.BatteryCapacity = g_AO_BMS.Output.BatteryCapacity;
    
    // ����س��������仯ʱ������֮
    if (g_SystemRecord.BatteryFullCapacity != g_AO_BMS.Output.FullCapacity 
        && g_AO_BMS.Output.FullCapacity > 0)
    {
        g_SystemRecord.BatteryFullCapacity = g_AO_BMS.Output.FullCapacity;
    }
    
    // ���µ����ٷք1�7?
    g_SystemRecord.SOC = g_AO_BMS.Output.SOC;   
    
    // ���ѭ����������
    if (g_AO_BMS.Output.CircleNumberAdd > 0)
    {
        g_SystemRecord.CircleNumber += g_AO_BMS.Output.CircleNumberAdd;
        g_AO_BMS.Output.CircleNumberAdd = 0;
        
        // ���Ƶ��ѭ���������Ϊ6000�Σ��ﲻ����
        if (g_SystemRecord.CircleNumber > 60000)                        //  CircleNumber:���ѭ�����������1�7?.1�1�7?bit
        {
            g_SystemRecord.CircleNumber = 60000;
        }
    }
    
    // ��ز���������������
    if (g_AO_BMS.Output.LifeCalibrateCircleNumberAdd > 0)
    {
        g_SystemRecord.LifeCalibrateCircleNumber += g_AO_BMS.Output.LifeCalibrateCircleNumberAdd / 10;
        g_AO_BMS.Output.LifeCalibrateCircleNumberAdd = 0;    
        
        // ����ބ1�7?000�Σ��1�7?0%����
        if (g_SystemRecord.LifeCalibrateCircleNumber > 20000)           //  LifeCalibrateCircleNumber:���ѭ�����������1�7?.1�1�7?bit
        {
            g_SystemRecord.LifeCalibrateCircleNumber = 20000;
        }      
    }
    
	// ��¼�����ʷ����1�7?
	if (g_AO_BMS.Output.BatteryTemperature[0] > g_SystemRecord.MaxBatteryTemperature)
	{
		g_SystemRecord.MaxBatteryTemperature = g_AO_BMS.Output.BatteryTemperature[0];
	}

	if (g_AO_BMS.Output.BatteryTemperature[1] > g_SystemRecord.MaxBatteryTemperature)
	{
		g_SystemRecord.MaxBatteryTemperature = g_AO_BMS.Output.BatteryTemperature[1];
	}
    // ��¼BMS��ʷ����1�7?
    if(g_AO_BMS.Output.BMSTemperatureHi > g_SystemRecord.BMSMaxTemp)
    {
        g_SystemRecord.BMSMaxTemp = g_AO_BMS.Output.BMSTemperatureHi;
    }
	
    // Ĭ��60sд��һ�����м�¼������д������
    if (g_AO_BMS.State.OutputAllow == 0)        // ʹ�����ʱ��ֹд�����мǄ1�7?
    {
        me->Variable.RecordWriteDelay++;    
        if (me->Variable.RecordWriteDelay >= RECORD_WRITE_TIMER
            || me->State.RecordWriteAsk > 0)            
        {   
            // ����EEPROMд���¼�,д�����м�¼
            QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, EEPROM_BLOCK_RECORD);   
            
            me->Variable.RecordWriteDelay = 0;
            me->State.RecordWriteAsk = 0;
        }
    }
}


// End of AO_EEPROM.c


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


// ---------------------------- ��ص������㷽�� --------------------------------
// To be update�� ����

/* BQ769x״̬���ᷢ�����ݸ����¼���BMS״̬���յ����¼��󣬴������ݣ���������ʣ�������
1.�������㷽������AH��������һ��������
2.���ʱĬ��Ϊ0.5C��磬��׼AH�����������������������Ƶ���ʱֹͣ��硣���ص�����ߵ�ѹ����4.2V��
  ��ʱ��ؼ������С����Ƶ������洢�˲���Ϊ���ʵ�ʵ����������ʵ�ʵ���С�����ֵ������ʵ�ʵ���Ϊ׼������SOC��
3.�ŵ�ʱĬ��0.5C������͵����ѹ�ϵ�ʱ�����ݵ�������������ʣ��������˵����Դ���ʵ��ֵ��������������
  �����������ڹ���ʣ�����������һ�����ʽ��ͼ������������Խ�ͣ�����Խ�ߡ�
*/


#include "stm32f0xx.h"                         // STM32�����Ĵ�������ͷ�ļ�
#include "string.h"                             // ����memset������
#include "system_core.h"                        // ϵͳ����ͷ�ļ�
#include "qpn_port.h"                           // ״̬��ͷ�ļ�
 

// ---------------------- ״̬�����м�¼�������� -----------------------------
#define AO_DEBUG    1

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
//u16 ChargeCnt = 0;            // ���ڸ����жϳ�ŵ�״̬
//u16 CutOffDelay = 0;

// ------------------------------ �ڲ��ɵ��ù��ܺ��� --------------------------------
void BatteryCapacity_Update(AO_BMS * const me);                 // ��ص�������
void BatteryCapacity_Calibrate(AO_BMS * const me);				// ��ص���������1s����һ��


// -------------------------------- ������� ------------------------------------
void AO_BMS_ctor(AO_BMS * const me) {
    QActive_ctor(&me->super, Q_STATE_CAST(&AO_BMS_initial));
}

// -------------------------------- ��ʼ״̬ ------------------------------------
QState AO_BMS_initial(AO_BMS * const me) {
    return Q_TRAN(&AO_BMS_StartWait);
}

// ------------------------------- 1.�����ȴ���һ��ʱ�䣬�ٶ�ȡEEPROM ----------------------------------
QState AO_BMS_StartWait(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(1);
#endif
            
            // ������ʱ���ⷢ��CAN���ݹ���
            me->State.CANCommEnable = 1;						// ����ʼCAN����ͨѶ
            
            g_AO_SH36730x0.State.BatteryBalanceEnable = 0;        // �رվ��� 
            
			// ������ȴ�2sʱ�䣬����ʱ����Ϊϵͳ����ʧ�ܣ�BMS�޷���������
            QActive_armX((QActive *)me, 0U, 2000U);        	// ������ʱ��0��2s

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {
            // ���ù�����Ϣ��BMS����ʧ��
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_START_FAIL);
            
            status = Q_TRAN(&AO_BMS_Fault);             		// ��ת��BMS����״̬
        }
        break;
        
        case START_SIG:                                 		// �����¼���BQ769x0оƬ��ʼ����ɣ��޴�����
        {			
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ��0       
            	
            // ����������ǰ��������һ�ε�����ɳ�������������м�¼����ȡ
            if (g_SystemRecord.BatteryFullCapacity == 0)		// ��һ������ʱ�����м�¼�п����޵�س�����������ֵΪ��Ƶ���
            {
                me->Output.FullCapacity = me->Parameter.BatteryDesignCapacity;
            }
            else
            {
				// ������������λ1mAH
                me->Output.FullCapacity = g_SystemRecord.BatteryFullCapacity;			//��-------
            }
                    
            // ������Idle״̬ʱ��KEY�ѱպϣ�����DIN_SIG����ؽ������״̬
            if (g_Input.Output.DIN_bits.bit.KEY > 0)
            {
                QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 1);                        //��-------
            }   

            // KEYʹ�ܣ��ҳ������ϱ�־λδ��λ
            if (g_Input.Output.DIN_bits.bit.KEY > 0 && g_SystemState.State.bit.ChargerFaultFlag == 0)
            {		
                QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 1);	//�����¼������¼���Կ�׻����������״̬
                
                g_SystemState.State.bit.KEYState = g_Input.Output.DIN_bits.bit.KEY;	//	����ϵͳ״̬��Կ�ױպ�
            }                   
            
            status = Q_TRAN(&AO_BMS_Normal);            		// ��ת��BMS��������״̬
        }
        break;
		
        case ENTER_BOOTLOADER_SIG:                      		// ����Bootloader�¼�
        {
            status = Q_TRAN(&AO_BMS_EnterBootloader);  
        }
        break;
        
        case FAULT_SIG:
        {
            status = Q_TRAN(&AO_BMS_Fault);  
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// ��״̬Ϊtop״̬       
        }
        break;
    }
	
    return status;
}

// ------------------------------- 2.BMS��������״̬ ----------------------------------
QState AO_BMS_Normal(AO_BMS * const me) {
    
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
            // Ĭ����ת������״̬
            status = Q_TRAN(&AO_BMS_Idle);      
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case FAULT_SIG:     //CANCommEnable                                  //�����¼�
        {
            status = Q_TRAN(&AO_BMS_Fault);  
        }
        break;
        
        case ENTER_BOOTLOADER_SIG:                      		// ����Bootloader�¼�
        {
            status = Q_TRAN(&AO_BMS_EnterBootloader);  
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// ��״̬Ϊtop״̬       
        }
        break;
    }
    
    return status;
}

// ------------------------------- 3.BMS����״̬���ر���� ----------------------------------
QState AO_BMS_Idle(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(3);
#endif
            
            me->State.OutputAllow = 0;                          // �������ܿ�����������������Ʊ�־
            
            // ���³�ŵ�mosfet����        
            g_AO_SH36730x0.State.CHGControl = 0;          		// BQ769x0�رճ��MOSFET
            g_AO_SH36730x0.State.DSGControl = 0;          		// BQ769x0�رշŵ�MOSFET
            
            // ÿ�ν������״̬ʱ������Ƿ���mcu�����������У�������֮
            // �����޸Ĳ�д����ɺ�ᴥ��һ��mcu��������
			// To be update�� �ٴ�ȷ�ϴ˲��ִ����Ƿ����ã�
            if (g_AO_BMS.State.ChipResetAsk > 0)                // �յ�оƬ������������mcu
            {
                g_AO_BMS.State.ChipResetAsk = 0;
                
                NVIC_SystemReset();                             //�����λ   
            } 

			
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG:
        {
//            // �������ؾ��ⱻ����ʱ����������4H���˳�����״̬ʱȡ��
//            if (me->State.BatteryExtraBalance == 1)
//            {
//                me->State.BatteryExtraBalance = 0;
//                
//                status = Q_TRAN(&AO_BMS_ExtraBalance);      	// ��ת�����ж������״̬
////                status = Q_TRAN(&AO_BMS_SleepDelay);            // ��ת������˯��״̬
//            }
//            else
//            {
                status = Q_TRAN(&AO_BMS_SleepDelay);            // ��ת������˯��״̬                            
//            }       
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case DIN_SIG:                                         //�����ź������¼�
        {
            switch ((u16)Q_PAR(me))    //#define Q_PARAM_SIZE 
            {
                case 1:         								// ������ŵ��·
                {   
                    //status = Q_TRAN(&AO_BMS_On);            	// ��ת��BMS����״̬
                    status=Q_TRAN(&AO_BMS_OnDelay);             //��ת��BMS�ӳ�         //��
                }
                break;
                
                default:
                {                 
                    status = Q_HANDLED();
                }
                break;
            }
        }
        break;
        
        case BMS_UPDATE_SIG:                					// BQ769оƬ����ֵ�����¼�
        {                   
            // ���µ���
            BatteryCapacity_Update(me); //��--------û������
            
            status = Q_HANDLED();
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_BMS_Normal);               	// ��״̬ΪNormal״̬       
        }
        break;
    }
    
    return status;
}

// ------------------------------- 32.BMS����ʱ�������״̬ ----------------------------------
QState AO_BMS_ExtraBalance(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(32);
#endif
            
            g_AO_SH36730x0.Parameter.BallanceErrVoltage = 5;      // ����5mV�������⿪��
			
            g_AO_SH36730x0.State.BatteryBalanceEnable = 1;        // ��������             
            
            me->Variable.ExtraBallanceCount = 0;				  // ���������������
            
			// ��ʱ��3ʱ��Ϊ1s��ע����������ʱ������
			// ���������Ƴ���4H��ʱ��ϳ���ÿ�ζ�ʱ���趨Ϊ60s������ִ��240��
            QActive_armX((QActive *)me, 3U, 60U);       		// ������ʱ��3��60s

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 3U);         		// �رն�ʱ��3
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT3_SIG:                        			// ���������ʱ��ʱ����60s����һ��
        {
            status = Q_HANDLED();
            
            // ��ض���������ʱ�䣬Ĭ��4H
            me->Variable.ExtraBallanceCount++;
            
            if (me->Variable.ExtraBallanceCount < 240)			// ����ʱ��δ����4H
            {                               
                // �������ʱ��δ������ⵥ���ѹ���С��5mV���˳��������
                // ����͵����ѹ����3.3V��ǿ��ֹͣ�������
                if ((g_AO_SH36730x0.Output.SingleMaxVoltage - g_AO_SH36730x0.Output.SingleMinVoltage) <= g_AO_SH36730x0.Parameter.BallanceErrVoltage
                    || g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4)
                {
                    status = Q_TRAN(&AO_BMS_SleepDelay);        // ��ת������˯��״̬
                }
                else
                {
                    QActive_armX((QActive *)me, 3U, 60U);       // ������ʱ��0��60s
                    
                    status = Q_HANDLED();
                }               
            }
            else
            {
				// ����������4H��ǿ�ƽ�������
                status = Q_TRAN(&AO_BMS_SleepDelay);            // ��ת������˯��״̬
            }
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_BMS_Idle);                 	// ��״̬ΪIdle״̬       
        }
        break;
    }
	
    return status;
}

// ------------------------------- 33. ����˯��״̬ ----------------------------------
// ��״̬Ϊ��͹���״̬���ɿ����ڴ����Ӵ�������
QState AO_BMS_SleepDelay(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(33); 
#endif
            
            g_AO_SH36730x0.State.BatteryBalanceEnable = 0;        // �رվ��� 
//=========================================================ʱ������е���֣�SleepDlayLongʼ��Ϊ0================================================================			
                                                           //�����Ļ��ͻ���ɣ�����й�������⣬���뵽��ʱ������������ʱ��3����͹���
			                                               //����ͻ���ʱ600s�����͹���
			//�����4Сʱ �������ÿ60s��1 ���240��     //����͹���ǰ��ʱ����ʱ ��λH
            if (me->Variable.ExtraBallanceCount > g_SystemParameter.BMS.Output.SleepDelayLong * 60) //g_SystemParameter.BMS.Output.SleepDelay
            {
                  //QActive_armX((QActive *)me, 3U, 30);//����
                  QActive_armX((QActive *)me, 3U, g_SystemParameter.BMS.Output.SleepDelay); //������ʱ��3 ����ʱ����͹���
            }
            else
            {    
                 //QActive_armX((QActive *)me, 3U, 10);//����
                // Ĭ����ʱ600s������˯��                                                                                                                    //�������else˵��ExtraBallanceCount=0 ʱ���ǹ̶���600s
                QActive_armX((QActive *)me, 3U, (g_SystemParameter.BMS.Output.SleepDelay + (u32)(g_SystemParameter.BMS.Output.SleepDelayLong) * 3600 - (u32)(me->Variable.ExtraBallanceCount) * 60));       // ������ʱ��3��Ĭ��600s                    
            }
//=============================================================================================================================================================
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 3U);         		// �رն�ʱ��3
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT3_SIG:                                	// ��ʱ��3����������������SHIP
        {   
        //  QActive_armX((QActive *)me, 3U, 1U);        		// ������ʱ��3��1s
            
            g_AO_SH36730x0.State.EnterSHIPModeAsk = 1;        	// �������SHIP�͹���ģʽ 
            
            status = Q_HANDLED();
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_BMS_Idle);                 	// ��״̬ΪIdle״̬       
        }
        break;
    }
    
    return status;
}

// ------------------------- 34.�������ǰ��ʱ״̬���ر���� ----------------------------
QState AO_BMS_OnDelay(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(34);
#endif
            
            g_SystemMonitor.System.Reverse3.Data[15]++;
            
            me->State.OutputAllow = 0;                          // �������ܿ������
            
            // ���³�ŵ�mosfet����        
            g_AO_SH36730x0.State.CHGControl = 0;          		// BQ769x0�رճ��MOSFET
            g_AO_SH36730x0.State.DSGControl = 0;          		// BQ769x0�رշŵ�MOSFET
            
           // me->Output.DischargeTimeOnce = 0;                   // ������ηŵ��ۼ�ʱ��
            
            // �������ʱΪ0s�����ֶ����ö�ʱ��ʱ��Ϊ1��1ms�󴥷�������ʱ��ʱ������Ϊ0���޷�������ʱ����
            if (g_SystemParameter.BMS.Output.OutputDelay > 0)
            {            
                QActive_armX((QActive *)me, 0U, g_SystemParameter.BMS.Output.OutputDelay);     		// ������ʱ��0��1s
			}
            else
            {
                QActive_armX((QActive *)me, 0U, 1U);     		// ������ʱ��0��1ms,���̴���
            }
            
            status = Q_HANDLED();           
        }
        break;       
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {           
            status = Q_TRAN(&AO_BMS_On);            	// ��ת��BMS����״̬
        }
        break;
        
        case DIN_SIG:                                  //������������¼�
        {
            switch ((u16)Q_PAR(me))//Q_PAR(me) ����  QACTIVE_POST((QActive *)&g_AO_BMS, DIN_SIG, 1);	���һ������
            {
                case 0:         								// ������ŵ��·
                {   
                    status = Q_TRAN(&AO_BMS_Idle);            	// ��ת��BMS����״̬//Կ�׶Ͽ������ս�����������ߵȴ�˯��
                }
                break;
                
              /*  case 2:         								// ��������·
                {   
                //    status = Q_TRAN(&AO_BMS_On);            	// ��ת��BMS����״̬
                    status = Q_TRAN(&AO_BMS_OnChargeDelay);     // ��ת��BMS���ǰ��ʱ״̬
                }
                break;   */             
                
                default:
                {                 
                    status = Q_HANDLED();
                }
                break;
            }
        }
        break;
        
        case BMS_UPDATE_SIG:                					// BQ769оƬ����ֵ�����¼�
        {                   
            // ���µ���
            BatteryCapacity_Update(me);      //��-----------
            
            status = Q_HANDLED();
        }
        break;      
        
        default: 
        {
            status = Q_SUPER(&AO_BMS_Normal);               	// ��״̬ΪNormal״̬       
        }
        break;
    }
    
    return status;
}


// ------------------------------- 31.���״̬ ----------------------------------
// �յ�Կ�׿����źţ������ŵ��·���ȴ�һ��ʱ�䣬����ͨ�����ֳɹ�����ر������
// �����ֳɹ�������������Ϣ�жϿ��������ǳ�����������Ӧ����ģʽ
// ��CANͨ��ģʽʱ����״̬����������״̬
// BMS_ON��������״̬����״̬�ɽ�һ��ϸ�ֶ����״̬������Ԥ�ŵ磬�Ӵ����պϣ���磬�ŵ��
u8 Restart_balancing=0;//
extern u16 UV_RECHARGE;
//u8 uv_flag=0;//Ƿѹ��־λ
QState AO_BMS_On(AO_BMS * const me) {
    
    QState status;

    s32 l_data = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(31);
#endif          
           
            me->State.OutputAllow = 1;                          // �������ܿ���ʹ��
            g_AO_BMS.Output.StartSOC = g_AO_BMS.Output.SOC;
            me->Variable.AutoCutoffDelay = 0;					// �Զ��ж������ʱ����
            
            // ������ʱ���ȼ�¼ʣ���������Ϊ���γ�ŵ����������
            me->Variable.ChargeStartCapacity = me->Output.BatteryCapacity;
            
#if (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2) // ��꿻���BMS��Ԥ�ŵ繦��
            
			Output_PreDischarge_Update(&g_Output, ENABLE);  	// ����Ԥ�ŵ�
		
			g_AO_BQ769x0.State.CHGControl = 0;          		// BQ769x0�رճ��MOSFET
			g_AO_BQ769x0.State.DSGControl = 0;          		// BQ769x0�رշŵ�MOSFET            
		
			// ������ʱ��0��Ĭ��1s����ʱ��ر�Ԥ�ŵ磬������·
			QActive_armX((QActive *)me, 0U, g_SystemParameter.BMS.Output.PreDischargeTime);     
		
#else                       									// ����Ԥ�ŵ繦�ܣ�ֱ�Ӵ򿪳�ŵ��·
            
			// ���³�ŵ�mosfet����        
			g_AO_SH36730x0.State.CHGControl = 1;          		// BQ769x0�������MOSFET
			g_AO_SH36730x0.State.DSGControl = 1;          		// BQ769x0�����ŵ�MOSFET
			
#endif

			// To be update���˲�������ͨ����������
            g_AO_SH36730x0.Parameter.BallanceErrVoltage = 2000;     	// ��ŵ�ʱ����8mV�������⿪��
            g_AO_SH36730x0.State.BatteryBalanceEnable = 1;        // ��������
     
     
            me->Variable.dsg_limit_cnt = 0;
            me->Variable.dsg_cnt = 0;
            me->Variable.chg_cnt = 0;

            // ��ʱ��1��Ϊ1s��ʱʱ����
            QActive_armX((QActive *)me, 1U, 1000U);     		// ������ʱ��1��1s
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ��0
            QActive_disarmX((QActive *)me, 1U);         		// �رն�ʱ��1

#if (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)          // ��꿻���BMS��Ԥ�ŵ繦��
        			
            Output_PreDischarge_Update(&g_Output, DISABLE);     // �ر�Ԥ�ŵ�

#endif
            
            me->State.OutputAllow = 0;                          // �������ܿ������
            
            // �˳����ʱ�����㱾�γ�ŵ������������ѭ������
			// ��ֵΪ��磬��ֵΪ�ŵ�
			// To be update���˱�������monitor
            me->Output.ChargeOrDischargeCapacity = me->Output.BatteryCapacity - me->Variable.ChargeStartCapacity;
            
            if (me->Output.ChargeOrDischargeCapacity > (me->Parameter.BatteryDesignCapacity / 2))
            {
                me->Output.CircleNumberAdd = 10;        		// ѭ��������1����λΪ0.1��
            }
            else if (me->Output.ChargeOrDischargeCapacity > (me->Parameter.BatteryDesignCapacity / 5))
            {
                me->Output.CircleNumberAdd = 5;     			// ѭ��������0.5����λΪ0.1��
            }
            
            g_AO_BMS.Variable.LowCapCutoffCnt = 0;
            g_AO_BMS.Variable.DischargeFilter = 0;
            
            status = Q_HANDLED();
        }
        break;
 
#if (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)          // ��꿻���BMS��Ԥ�ŵ繦��
            		
        case Q_TIMEOUT_SIG:         // ��ʱ��0����Ԥ�ŵ�����߼�
        {
            // Ԥ�ŵ���Ʒ�Ϊ2����
            // Step1������Ԥ�ŵ磬��ʱ1s��涨ʱ�������ŵ��·
            // Step2������ʱ1s���ر�Ԥ�ŵ�
            if (g_AO_BQ769x0.State.DSGControl == 0)
            {
                // Step1������Ԥ�ŵ磬��ʱ�涨ʱ�������ŵ��·
                QActive_armX((QActive *)me, 0U, 1000U);     	// ������ʱ��0��1s����ʱ��ر�Ԥ�ŵ磬������·

                // ���³�ŵ�mosfet����        
                g_AO_BQ769x0.State.CHGControl = 1;          	// BQ769x0�������MOSFET
                g_AO_BQ769x0.State.DSGControl = 1;          	// BQ769x0�����ŵ�MOSFET                    
            }
            else
            {
                // Step2������ʱ1s���ر�Ԥ�ŵ�                
                Output_PreDischarge_Update(&g_Output, DISABLE); 

				// ���³���CANģ���ʼ��
				CAN_Config(&g_CAN);
            }

            status = Q_HANDLED();           
        }
        break;
 
#endif						// (CONTROLLER_TARGET == BMS_XINXIN_A1 || CONTROLLER_TARGET == BMS_XINXIN_A2)
		
        case Q_TIMEOUT1_SIG:            						// ��ʱ��1Ϊ1sʱ����ʱ��
        {
            status = Q_HANDLED(); 
            
            QActive_armX((QActive *)me, 1U, 1000U);     		// ������ʱ��1��1s
			
//=================================================================================================================			
//			if(Restart_balancing++>10 && (g_AO_SH36730x0.Output.BatteryCurrent) >-2000)//�ŵ��������2A���������⣬�ŵ�����������50�η����������������־
//			{	
//			Restart_balancing=0;
//            g_AO_SH36730x0.State.BatteryBalanceEnable = 1;        // ��������
//            }
//			else
//			{
//			Restart_balancing=0;
//			}
			
//===================================================================================================================           
            // ���������״̬���ҵ���������100mAʱ����طŵ��ۼ�ʱ��++
            if (g_AO_SH36730x0.Output.BatteryCurrent < 100)
            {
                g_SystemRecord.DischargeTime++;                 // �ۼƷŵ�ʱ��+1s����λ1s
            }      
			
//			if (g_AO_SH36730x0.Output.BatteryCurrent > 100
//				&& g_AO_SH36730x0.Output.SingleMaxVoltage > 4050)
//			{
//				// ���ӽ����ս׶�ʱ����������ʹ�ܵ�ѹǿ�Ʊ�С��
//				// �ж���������ش��ڳ��׶Σ���������ѹ����4.05V
//				g_AO_SH36730x0.Parameter.BallanceErrVoltage = 3;     	// ��ŵ�ʱ����3mV�������⿪��
//			}
//			else
//			{
//				// ��������£���������������ѹΪ8mV�������鳬��10mV
//				g_AO_SH36730x0.Parameter.BallanceErrVoltage = 8;     	// ��ŵ�ʱ����8mV�������⿪��
//			}
            //����⣬ÿ1s���һ��
            if(g_AO_SH36730x0.Output.BatteryCurrent > 1500 || g_SystemState.State.bit.ChargeTempError > 0)//����¶�����
            {
                
                if(g_AO_BMS.Variable.ChargeCheckCnt > 20)        //����ж�ʱ���ݶ�Ϊ5s
                {
                    g_SystemState.State.bit.ChargeOnFlag = 1;
                } else g_AO_BMS.Variable.ChargeCheckCnt++;
            }
            else
            {
                if(g_AO_BMS.Variable.ChargeCheckCnt)g_AO_BMS.Variable.ChargeCheckCnt--;
                else g_SystemState.State.bit.ChargeOnFlag = 0;
            }
			
            if(g_SystemState.State.bit.ChargeTempError > 0)
            {
                g_AO_SH36730x0.State.CHGControl = 0;          		// BQ769x0�������MOSFET
                g_AO_SH36730x0.State.DSGControl = 0;          		// BQ769x0�����ŵ�MOSFET
            }
//==========================================================================================================================
//			else if(uv_flag==1)//Ƿѹ���ʱ�ر� �ŵ�MOS�򿪳��MOS
//			{
//			    g_AO_SH36730x0.State.CHGControl = 1;          		//		
//                g_AO_SH36730x0.State.DSGControl = 0;          		// 
//			}
//==========================================================================================================================
            else
            {
                g_AO_SH36730x0.State.CHGControl = 1;          		// BQ769x0�����ŵ�MOSFET		
                g_AO_SH36730x0.State.DSGControl = 1;          		// BQ769x0�����ŵ�MOSFET
            }
            
            me->Variable.tc_voltage = get_temp_current_v(g_AO_BMS.Output.BatteryTemperatureHi, g_AO_SH36730x0.Output.BatteryCurrent);
            
            
            if(me->Variable.dsg_limit_flg && g_AO_SH36730x0.Output.BatteryCurrent < -2000)
            {
                me->Variable.dsg_limit_cnt2++;
                if(me->Variable.dsg_limit_cnt2 > 1)
                {
                    me->Variable.dsg_limit_cnt2 = 0;
                    me->Output.SOC = 0;
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            }
            
            /// ��ŵ�ֹͣ����
            if(g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage - me->Variable.tc_voltage) /// 2.8
            {
                me->Variable.dsg_cnt++;
                me->Variable.dsg_limit_cnt++;
                if(me->Variable.dsg_limit_cnt > ((g_SystemState.State.bit.ChargeOnFlag || g_AO_SH36730x0.Output.BatteryCurrent > -1000)?300:10))
                {
                    me->Variable.dsg_limit_cnt = 0;
                    me->Output.SOC = 50;
                    me->Variable.dsg_limit_flg = 1;
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            } else if (g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.Discharge.DischargeStopVoltage - me->Variable.tc_voltage && g_SystemState.State.bit.ChargeOnFlag == 0) /// 3.1
            {
                if(me->Variable.dsg_limit_cnt)me->Variable.dsg_limit_cnt--;
                me->Variable.dsg_cnt++;
                if(me->Variable.dsg_cnt > 60)
                {
                    me->Variable.dsg_cnt = 0;
                    me->Output.SOC = 150;
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            } else {
                me->Variable.dsg_limit_cnt = 0;
                if(me->Variable.dsg_cnt)me->Variable.dsg_cnt--;
                me->Variable.dsg_limit_flg = 0;
            }
            
            
            if (g_AO_SH36730x0.Output.SingleMaxVoltage > g_SystemParameter.BMS.Battery.CellChargeStopVoltage) /// 3.6
            {
                me->Variable.chg_cnt++;
                if(me->Variable.chg_cnt > 10)
                {
                    me->Output.SOC = 900;
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            } else {
                me->Variable.chg_cnt = 0;
            }
            
            
            /// �Զ��ػ�
            // ���ŵ����С���趨ֵ�ҳ����趨ʱ�䣬ǿ�ƽ����ŵ�  ��ŵ����С��600ma ѭ��900�ν������
            // �����������Զ��жϵ�����Ϊ0ʱ����������
            if (g_SystemParameter.BMS.Output.AutoCutoffCurrent > 0
                && g_AO_SH36730x0.Output.BatteryCurrent < g_SystemParameter.BMS.Output.AutoCutoffCurrent
                && g_AO_SH36730x0.Output.BatteryCurrent > -g_SystemParameter.BMS.Output.AutoCutoffCurrent)
            {
                me->Variable.AutoCutoffDelay++;
                
                if (me->Variable.AutoCutoffDelay >= g_SystemParameter.BMS.Output.AutoCutoffDelay
                    && g_SystemState.State.bit.ChargeOnFlag == 0)
                {                           
                    status = Q_TRAN(&AO_BMS_Idle); 		// ��ת��BMS����״̬                           
                }
                else if(me->Variable.AutoCutoffDelay >= g_SystemParameter.BMS.Output.AutoCutoffDelay * 8)
                {
                    status = Q_TRAN(&AO_BMS_Idle); 		// ��ת��BMS����״̬  
                }
            }
            else
            {
                me->Variable.AutoCutoffDelay = 0;
            }
            
            
            
            
            
            
			/*
//			if(g_AO_SH36730x0.Output.SingleMinVoltage>g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage)//Ƿѹ��絽2.55�رձ�־
//			{
//               uv_flag=0;
//			
//			}
				
			// Take care�����¼������Ϊ���ӣ�����������Ĭ����ת����
//            if(g_SystemState.State.bit.ChargeOnFlag > 0)
//            {
//                g_AO_BMS.Output.StartSOC = 0;
//                g_AO_BMS.Variable.LowCapCutoffCnt = 0;
//            }
//            else
//            {                 
//                if(g_AO_BMS.Output.StartSOC == 0)
//                {
//                    if(g_AO_BMS.Output.SOC > 110)
//                    {
//                        g_AO_BMS.Output.StartSOC = 0;
//                    }
//                    else if(g_AO_BMS.Output.SOC >= 50)
//                    {
//                        g_AO_BMS.Output.StartSOC = g_AO_BMS.Output.SOC;
//                        g_AO_BMS.Parameter.LowSOCCutOffTime = (70 - (110-g_AO_BMS.Output.SOC));
//                    }
//                    else if(g_AO_BMS.Output.SOC > 0)
//                    {
//                        g_AO_BMS.Output.StartSOC = g_AO_BMS.Output.SOC;;
//                        g_AO_BMS.Parameter.LowSOCCutOffTime = 20;
//                    }
//                    else
//                    {
//                        g_AO_BMS.Output.StartSOC = 1;
//                        g_AO_BMS.Parameter.LowSOCCutOffTime = 20;
//                    }
//                    g_AO_BMS.Variable.LowCapCutoffCnt = 0;
//                }
//                else
//                {
//                    g_AO_BMS.Variable.LowCapCutoffCnt ++;                    
//                    if(((g_AO_BMS.Output.StartSOC - g_AO_BMS.Output.SOC) > 10) 
//                        || (g_AO_BMS.Variable.LowCapCutoffCnt > g_AO_BMS.Parameter.LowSOCCutOffTime))
//                    {
//                        g_AO_BMS.Output.StartSOC = 0;  //��Ҫ���¸���ԭʼSOC����Ϊһ��BMS����ϵ�����
//                        g_AO_BMS.Variable.LowCapCutoffCnt = 0;             //���ʱ���ʱ����
//                        me->State.OutputAllow = 0;                          // �������ܿ������            
//                        status = Q_TRAN(AO_BMS_LowPower);
//                    }
//                }
//            }
            if(g_AO_BMS.Output.SOC > g_AO_BMS.Output.StartSOC) g_AO_BMS.Output.StartSOC = g_AO_BMS.Output.SOC;//С��15%ÿ�½�2%�Ͽ�һ�����
            
            if(g_AO_BMS.Output.SOC <= 150 && !g_SystemState.State.bit.ChargeOnFlag)
            {
                g_AO_BMS.Variable.LowCapCutoffCnt++;
                if(g_AO_BMS.Output.StartSOC >= g_AO_BMS.Output.SOC + 10 || g_AO_BMS.Variable.LowCapCutoffCnt > 300)
                {
                    status = Q_TRAN(&AO_BMS_Idle);
                }
            } 
			else g_AO_BMS.Variable.LowCapCutoffCnt = 0;
            
            if(g_AO_BMS.Output.SOC < 100  && g_AO_SH36730x0.Output.BatteryCurrent <= -2000)//SOCС��10%��ֹ�ŵ�������״̬
            {
                g_AO_BMS.Variable.DischargeFilter++;
                if(g_AO_BMS.Variable.DischargeFilter)
                    status = Q_TRAN(AO_BMS_Idle);
            } else g_AO_BMS.Variable.DischargeFilter = 0;
            
            

            // ���ŵ�ʱ������ѹ������15mV�������������
            // Take care: ��������5s���Żᴥ��
            if ((g_AO_SH36730x0.Output.SingleMaxVoltage - g_AO_SH36730x0.Output.SingleMinVoltage) > 15)//15�޸�Ϊ20����========================
            {
                me->Variable.BatteryExtraBalanceDelay++;
                if (me->Variable.BatteryExtraBalanceDelay >= 5)
                {
                    me->Variable.BatteryExtraBalanceDelay = 0;
                    me->State.BatteryExtraBalance = 1;   		// ʹ�ܶ���������ڽ������״̬ʱ���Զ��������
                }
            }
            else
            {
                me->Variable.BatteryExtraBalanceDelay = 0;
            }
            
            // ��ص���У׼����Ҫ����ŵ�������f�ϵ�ʱ����������ƫ������
            BatteryCapacity_Calibrate(me);

			// ��ŵ���������ж�
			// 1��������͵�ѹ����2.55V�ҳ���10s����,��δ���ڳ��״̬
			// 2��������͵�ѹ����3.0V�ҷŵ��������100mA�ҳ���5s����
			// 3��������ߵ�ѹ����4.21V
			// 4��������ߵ�ѹ���ڳ�������ѹ��Ĭ��4.18V�����ҳ���������100mA�ҳ���5s����
			// 5�������������������忴�������                                                 //2.55
            if (g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.Discharge.DischargeForceStopVoltage 
                && g_SystemState.State.bit.ChargeOnFlag  == 0 && g_AO_SH36730x0.Output.BatteryCurrent >-1000) 	// �ŵ��ֹ��ѹΪ2.6V   ChargeOnFlag ��翪����־ 1s���һ�γ���������1A�г���ʱ��5s
            {
                // �����ѹ����2.55V���ҳ���10s���ϣ���Ϊ����޵磬ֱ�ӹر�
                // ��״̬���޷��ٿ���������Ե�ؽ��г������ʹ��
                // ����ѹ����2.55V�������������������зŵ�����������ϳ����ٿ�������������Ϊ��ز��������
				
				if(g_AO_SH36730x0.Output.BatteryCurrent <1500)
			{
                me->Variable.ShutdownDelay++;
                if (me->Variable.ShutdownDelay >= 60)     		// ����3�ε�ѹ����2.7V����Ϊ��ѹȷʵ���ͣ�Լ10s
                {
                    me->Variable.ShutdownDelay = 0;
                    
                    me->Output.BatteryCapacity = 0;     		// �����ѹ����2.7V����Ϊ����Ϊ0                    
																
                 //   g_SystemMonitor.System.Reverse2.Data[2]++;
    
                    status = Q_TRAN(&AO_BMS_Idle);          	// ��ת��BMS����״̬  
					//uv_flag=1;					
                }
				
			}
			
            }
            else if (g_AO_SH36730x0.Output.SingleMinVoltage < g_SystemParameter.BMS.Discharge.DischargeStopVoltage)	// �ŵ��ֹ��ѹΪ2.7V
            {
                if ((g_AO_SH36730x0.Output.BatteryCurrent <-1000))      // �Ҵ���1A
                {
                    me->Variable.ShutdownDelay++;
                    
                    if (me->Variable.ShutdownDelay >= 15)    	// ����5�ε�ѹ����3.0V����Ϊ��ѹȷʵ���ͣ�Լ15s
                    {
                        me->Variable.ShutdownDelay = 0;
                        
                        // �˴����Ǹ����㷨������Ƿѹʱ��ֱ�ӽ��������㲢������
                        // ���������·ŵ磬����ʣ��϶�ʱ����������Ƿѹ����ʱʣ��������޷��ų�������û�С�
                        me->Output.BatteryCapacity = 0;     	// �����ѹ����3.0V����Ϊ����Ϊ0                    
                        
                    //    g_SystemMonitor.System.Reverse2.Data[3]++;
                        
						
                      status = Q_TRAN(&AO_BMS_Idle);      	// ��ת��BMS����״̬
                    }
                }
                else
                {
                    me->Variable.ShutdownDelay = 0;            //��ѹǷѹ��ʱ
                }
            }
            else if (g_AO_SH36730x0.Output.SingleMaxVoltage > (g_SystemParameter.BMS.Battery.CellOverVoltage - 30))	// �����г���ֹ��ѹΪ3.7V
            {
                me->Variable.ShutdownDelay = 0;
                
                // ���1000��ѭ������������20%
                // g_SystemRecord.CircleNumber����ŵ��������λ0.1�Σ�LifeCalibrateCircleNumberΪ�����ó�ŵ��������λ0.1��
                l_data = (me->Parameter.BatteryFullCapacity - (me->Parameter.BatteryFullCapacity / 50 * (g_SystemRecord.CircleNumber + g_SystemRecord.LifeCalibrateCircleNumber) / 1000));
                                
                if (me->Output.BatteryCapacity < l_data)
                {       
                    // ��AH��ͳ�Ƶ�������Ƶ���С1%������10�Σ�����ʧ0.2%�ĵ���
                    // LifeCalibrateCircleNumberAdd��λ0.1��
                    // ���ȼ������ƫС��������λ1%
                    if (((l_data - me->Output.BatteryCapacity) * 100 / me->Output.BatteryCapacity) <= 5)            // ����ƫС5%
                    {
                        // ���1000��ѭ������������20%
                        // ��Ƶ���ƫС5%��������������5.0�Σ�������������0.1%
                        me->Output.LifeCalibrateCircleNumberAdd = (l_data - me->Output.BatteryCapacity) * 1000 / me->Output.BatteryCapacity;
                    }
                    else                    					// ����ƫС����5%����Ϊ5%
                    {
                        // ��Ƶ���ƫС5%��������������5�Σ�������������0.1%
                        me->Output.LifeCalibrateCircleNumberAdd = 50;
                    }   
                    
                    me->Output.BatteryCapacity = l_data;                                   
                }
				
				me->Output.FullCapacity = me->Output.BatteryCapacity;	// ��������Ϊʵ�ʵ���                 
                me->Output.SOC = 1000;									// SOC����Ϊ100.0%
                
                // �����ѹ����3V�����4.2V����ֹͣ�ŵ�
            //    g_SystemMonitor.System.Reverse2.Data[4]++;
                
                status = Q_TRAN(&AO_BMS_Idle);              	// ��ת��BMS����״̬
            }           
            else if (g_AO_SH36730x0.Output.SingleMaxVoltage > g_SystemParameter.BMS.Battery.CellChargeStopVoltage)        // �����г���ֹ��ѹΪ3.7V
            {
                // ע�������ʱ����������ƶ������ܵ��¶�ʱ�䳬����ֵ����Ӧ�ô����������ػ��߼�
                if (g_AO_SH36730x0.Output.BatteryCurrent > 100)   // ����������100mA
                {
                    me->Variable.ShutdownDelay++;
                    if (me->Variable.ShutdownDelay >= g_SystemParameter.BMS.Battery.ChargeStopDelay)     	// ��ʱ5s
                    {
                        me->Variable.ShutdownDelay = 0;                      
                        
                        // ���1000��ѭ������������20%
                        // g_SystemRecord.CircleNumber����ŵ��������λ0.1�Σ�LifeCalibrateCircleNumberΪ�����ó�ŵ��������λ0.1��
                        l_data = (me->Parameter.BatteryFullCapacity - (me->Parameter.BatteryFullCapacity / 50 * (g_SystemRecord.CircleNumber + g_SystemRecord.LifeCalibrateCircleNumber) / 1000));
                    
                        if (me->Output.BatteryCapacity < l_data)
                        {       
                            // ��AH��ͳ�Ƶ�������Ƶ���С1%������10�Σ�����ʧ0.2%�ĵ���
                            // LifeCalibrateCircleNumberAdd��λ0.1��
                            // ���ȼ������ƫС��������λ1%
                            if (((l_data - me->Output.BatteryCapacity) * 100 / me->Output.BatteryCapacity) <= 5)            // ����ƫС5%
                            {
                                // ���1000��ѭ������������20%
                                // ��Ƶ���ƫС5%��������������5�Σ�������������0.1%
                                me->Output.LifeCalibrateCircleNumberAdd = (l_data - me->Output.BatteryCapacity) * 1000 / me->Output.BatteryCapacity;
                            }
                            else                    			// ����ƫС����5%����Ϊ5%
                            {
                                // ��Ƶ���ƫС5%��������������5�Σ�������������0.1%
                                me->Output.LifeCalibrateCircleNumberAdd = 50;
                            }       
                            
                            me->Output.BatteryCapacity = l_data;
                        }

                        me->Output.FullCapacity = me->Output.BatteryCapacity;	// ��������Ϊʵ�ʵ���                    
                        me->Output.SOC = 1000;									// SOC����Ϊ100.0%
						
                   //     g_SystemMonitor.System.Reverse2.Data[5]++;  

                        status = Q_TRAN(&AO_BMS_Idle);   		// ��ת��BMS����״̬
                    }
                }
            }
            else                    // δ����Ƿѹ��ѹ�����������Ƿ���Ͻ������Ҫ��
            {
                // δ���ֵ����ѹǷѹ����������Ƿ����   
				// ������������Ƶ������ҳ���������100mA���ҵ�����ߵ�ѹ������4.1V������ʱ10s
                if (me->Output.BatteryCapacity >= me->Parameter.BatteryDesignCapacity
                    && g_AO_SH36730x0.Output.BatteryCurrent > 100
                    && g_AO_SH36730x0.Output.SingleMaxVoltage > g_SystemParameter.BMS.Charge.ChargeFinishMinVoltage)//��ߵ�ѹ����3.4V
                {               
                    me->Variable.ShutdownDelay++;                 
                    if (me->Variable.ShutdownDelay >= 10)
                    {
                        me->Variable.ShutdownDelay = 0;
				
						// ���Ե�������ﵽ��������������������ʱ���ȽϽ���ʱ����ѹ��Խ�ӽ���ֹ��ѹ����Խ�Ӵ����˥������
						
                        // ���1000��ѭ������������20%
                        // g_SystemRecord.CircleNumber����ŵ��������λ0.1�Σ�LifeCalibrateCircleNumberΪ�����ó�ŵ��������λ0.1��
                        l_data = (me->Parameter.BatteryFullCapacity - (me->Parameter.BatteryFullCapacity / 50 * (g_SystemRecord.CircleNumber + g_SystemRecord.LifeCalibrateCircleNumber) / 1000));
                    
						// �������ؿɳ���������С��ʵ�ʵ��������ٲ�����������ͷ�˰�������
						// ��������99%���Ͼ������ٳ��ȥһ����
                        if (me->Output.BatteryCapacity < (l_data * 99 / 100))
                        {       
							// ������ʱ��ѹԽ�ӽ�g_SystemParameter.BMS.Battery.CellChargeStopVoltage������Խ�����в�����ΧΪ-50mV��0V
							// ��ѹ���50mV,�����������1mV,����4.9�Σ�������������Լ0.1%
							if (g_SystemParameter.BMS.Battery.CellChargeStopVoltage - g_AO_SH36730x0.Output.SingleMaxVoltage > 0
								&& g_SystemParameter.BMS.Battery.CellChargeStopVoltage - g_AO_SH36730x0.Output.SingleMaxVoltage < 50)
							{
								me->Output.LifeCalibrateCircleNumberAdd = 50 - (g_SystemParameter.BMS.Battery.CellChargeStopVoltage - g_AO_SH36730x0.Output.SingleMaxVoltage);
							}							
						}
						
                        me->Output.FullCapacity = me->Output.BatteryCapacity;	// ��������Ϊʵ�ʵ���                    
                        me->Output.SOC = 1000;									// SOC����Ϊ100.0%
                        
                    //    g_SystemMonitor.System.Reverse2.Data[6]++;
                        
                        // ��ص����������ֵ����ֹͣ�ŵ�
                        status = Q_TRAN(&AO_BMS_Idle);      	// ��ת��BMS����״̬
                    }
                }
                else                        
                {					
                    me->Variable.ShutdownDelay = 0;				// δ��������ж��������ж���ʱ����
                    
                    // ���ŵ����С���趨ֵ�ҳ����趨ʱ�䣬ǿ�ƽ����ŵ�  ��ŵ����С��600ma ѭ��900�ν������
                    // �����������Զ��жϵ�����Ϊ0ʱ����������
                    if (g_SystemParameter.BMS.Output.AutoCutoffCurrent > 0
                        && g_AO_SH36730x0.Output.BatteryCurrent < g_SystemParameter.BMS.Output.AutoCutoffCurrent
                        && g_AO_SH36730x0.Output.BatteryCurrent > -g_SystemParameter.BMS.Output.AutoCutoffCurrent)
                    {
                        me->Variable.AutoCutoffDelay++;
						//me->Variable.AutoCutoffDelay+=100;
						
                        if (me->Variable.AutoCutoffDelay >= g_SystemParameter.BMS.Output.AutoCutoffDelay
                            && g_SystemState.State.bit.ChargeOnFlag == 0)
                        {                           
                        //    g_SystemMonitor.System.Reverse2.Data[7]++;
                            
                            status = Q_TRAN(&AO_BMS_Idle); 		// ��ת��BMS����״̬                           
                        }
                        else if(me->Variable.AutoCutoffDelay >= g_SystemParameter.BMS.Output.AutoCutoffDelay * 5)
                        {
                            status = Q_TRAN(&AO_BMS_Idle); 		// ��ת��BMS����״̬  
                        }
                    }
                    else
                    {
                        me->Variable.AutoCutoffDelay = 0;
                    }
                }
            }
            */
        }
        break;
        
        case DIN_SIG:
        {
            switch ((u16)Q_PAR(me))
            {
																
                case 1:         								// ������ŵ��·
                {
					// ����A2�濪ʼ�����״̬���յ���ť�����źţ���ر����
					
#if (CONTROLLER_TARGET == BMS_XINXIN_A2)            

                    status = Q_TRAN(&AO_BMS_Idle);              // ��ת��BMS����״̬
                    
#else
                    status = Q_HANDLED();
                    
#endif                      // CONTROLLER_TARGET == BMS_XINXIN_A2
					
                }
                break;
                
                default:
                {   
                    // ����A2�汾��ʼ����ť���º���������������ť�ɿ����ر������������ؿ���ʱ�Զ��ر����
					
#if (CONTROLLER_TARGET == BMS_XINXIN_A2)            

                    status = Q_HANDLED();
                    
#else
                    
                //    g_SystemMonitor.System.Reverse2.Data[8]++;
                    
                    status = Q_TRAN(&AO_BMS_Idle);              // ��ת��BMS����״̬
                    
#endif                  	// CONTROLLER_TARGET == BMS_XINXIN_A2
                    
                }
                break;
            }
        }
        break;
                        
        case BMS_UPDATE_SIG:                					// BMS�����¼���ÿ�����ݸ�����ɷ���
        {           
            // ���µ���
//            BatteryCapacity_Update(me); 
            
            status = Q_HANDLED();
        }
        break;
                
        default: 
        {
            status = Q_SUPER(&AO_BMS_Normal);               	// ��״̬ΪNormal״̬       
        }
        break;
    }
	
    return status;
}


// ------------------------------- 100. BMS����״̬����ͼ�ָ�֮ ----------------------------------
QState AO_BMS_Fault(AO_BMS * const me) {
    
    QState status;
	
    u16 i = 0;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(100);
#endif
			
            me->State.OutputAllow = 0;                          // �������ܿ������
            
            me->Variable.FaultSleepDelay = 0;					// ����״̬����͹�����ʱ����
			me->Variable.FaultNoRecoverDelay = 0;				// ����δ�ָ���ʱ����
			
            g_AO_SH36730x0.State.BatteryBalanceEnable = 0;        // �رվ��� 
//==============================================================================================            

			g_AO_SH36730x0.State.DSGControl = 0;          		    // BQ769x0�رշŵ�MOSFET   
			g_AO_SH36730x0.State.CHGControl = 0;
     
//==============================================================================================            
            // ------------------------------- ��¼���� ---------------------------------
            // ���ֹ��ϼ�¼��д��һ��0x00�����룬���޹��ϣ�ԭ�����
            if (Protect_GetFaultCode(&g_Protect) > 0)           // �������ϴ���ʱ���й��ϼ�¼
            {
                // �����й��ϼ�¼����һ�����������¹���
                for (i = 127; i > 0; i--)
                {
                    g_SystemFaultHistory.FaultHistory[i].all = g_SystemFaultHistory.FaultHistory[i - 1].all;
                }
                
                g_SystemFaultHistory.FaultHistory[0].bit.FaultCode = Protect_GetFaultCode(&g_Protect);
                g_SystemFaultHistory.FaultHistory[0].bit.TimeStamp = g_SystemRecord.TotalTime / 60;
                
                // ���ϴ�������Ѱ�Ҳ���1
                for (i = 0; i <= 127; i++)
                {
                    // ��������ƥ��������Ϊ0ʱ����������벢��1
                    if (g_SystemFaultHistory.FaultCount[i].bit.FaultCode == Protect_GetFaultCode(&g_Protect)
                        || g_SystemFaultHistory.FaultCount[i].bit.FaultCode == 0x00)
                    {
                        g_SystemFaultHistory.FaultCount[i].bit.FaultCode = Protect_GetFaultCode(&g_Protect);
                        g_SystemFaultHistory.FaultCount[i].bit.FaultCodeCount++;
                        
                        break;
                    }
                }
                
             //   g_SystemMonitor.System.Reverse2.Data[13] = 1;
                
                // ���͹��ϼ�¼д���¼�
                QACTIVE_POST((QActive *)&g_AO_EEPROM, EEPROM_WRITE_SIG, EEPROM_BLOCK_FAULT_HISTORY);
            }           
            
            QActive_armX((QActive *)me, 0U, 500U);     		// ������ʱ��0��0.5s

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {   
            status = Q_HANDLED();
            
            QActive_armX((QActive *)me, 0U, 100U);      		// ������ʱ��0��0.1s
            
            if (g_Input.Output.DIN_bits.bit.KEY == 0)           // �������ضϿ�
            {
                if (Protect_GetFaultCode(&g_Protect) == 0)
                {       
					// ���������������ص�����״̬�����ٴ��������
                //    g_SystemMonitor.System.Reverse2.Data[9]++;
                    
                    status = Q_TRAN(&AO_BMS_Idle);              // ��ת��BMS����״̬
                }
                else
                {
                    // ����δ��������������
                    if (Protect_ClearFault(&g_Protect) > 0)		// �������������0������ɹ���1��δ���
                    {  
                        g_AO_EEPROM.State.RecordWriteAsk = 1;
						// 1s�����δ�ָ��������͹���״̬
						if (me->Variable.FaultSleepDelay++ < 10) 	// 0.1s��10
						{
							status = Q_HANDLED();
						}
						else
						{
							me->Variable.FaultSleepDelay = 0;
							
							// ��ʱ1s���Զ�����͹���
							g_AO_SH36730x0.State.EnterSHIPModeAsk = 1;	// �������SHIP�͹���ģʽ
						}
								  
                    }
                    else
                    {
                        // ���������
                     //   g_SystemMonitor.System.Reverse2.Data[10]++;
                        
                        status = Q_TRAN(&AO_BMS_Idle);      	// ��ת��BMS����״̬
                    }
                }
            }
            else
            {
                me->Variable.FaultSleepDelay = 0;
                
                // ���Ϸ���5min����δ���и�λ�������͹���״̬
                // ���ԣ����������������ܲ��ܽ���͹��ģ��������ܲ���
                if (me->Variable.FaultNoRecoverDelay++ < 3000)  	// 0.1s��600    
                {
                    status = Q_HANDLED();
                }
                else
                {
                    me->Variable.FaultNoRecoverDelay = 0;
			
					g_AO_SH36730x0.State.EnterSHIPModeAsk = 1;	// �������SHIP�͹���ģʽ
		
                }
            }
        }
        break;
		
        case BMS_UPDATE_SIG:                					// BMS�����¼���ÿ�����ݸ�����ɷ���
        {           
            // ���µ���
            BatteryCapacity_Update(me); 
            
            status = Q_HANDLED();
        }
        break;		
        
        case ENTER_BOOTLOADER_SIG:                      		// ����Bootloader�¼�
        {
            status = Q_TRAN(&AO_BMS_EnterBootloader);  
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// ��״̬Ϊtop״̬       
        }
        break;
    }
	
    return status;
}

//--------------------------------�͵������Ѵ���״̬-------------------------
/*
QState AO_BMS_LowPower(AO_BMS * const me)
{
    QState status;
    switch(Q_SIG(me))
    {
        case Q_ENTRY_SIG:
        {
#ifdef AO_DEBUG
    t_Record(200);
#endif
            g_AO_BQ769x0.State.BatteryBalanceEnable = 0;        // �رվ��� 
            
            me->State.OutputAllow = 0;                          // �������ܿ������
            // ���³�ŵ�mosfet����        
            g_AO_BQ769x0.State.CHGControl = 0;                  // BQ769x0�رճ��MOSFET
            g_AO_BQ769x0.State.DSGControl = 0;                  // BQ769x0�رշŵ�MOSFET
            g_AO_EEPROM.State.RecordWriteAsk = 1;
        }
        break;
        case Q_INIT_SIG:
        {
            QActive_armX((QActive *)me, 0U, 10);  //ÿ10ms���һ��Կ�׿���״̬1000*60/10=6000
            QActive_armX((QActive *)me, 3U, 60); 
        }
        break;
        case Q_TIMEOUT_SIG:
        {
            QActive_armX((QActive *)me, 0U, 10);
            if(g_Input.Output.DIN_bits.bit.KEY == 0) 
            {
                g_AO_BQ769x0.State.EnterSHIPModeAsk = 1;            // �������SHIP�͹���ģʽ            
            }
            status = Q_HANDLED();
        }
        break;
        case Q_TIMEOUT3_SIG:
        {
            g_AO_BQ769x0.State.EnterSHIPModeAsk = 1;            // �������SHIP�͹���ģʽ  
        }
        break;  
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// ��״̬Ϊtop״̬       
        }
        break; 
    }
    return status;
}


*/
// ------------------------------- 200. ����Bootloader��ʱ״̬ ----------------------------------
//����bootloaderʱע��ر�AFE���Ź�
QState AO_BMS_EnterBootloader(AO_BMS * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            
#ifdef AO_DEBUG
            t_Record(200);
#endif

            g_AO_SH36730x0.State.BatteryBalanceEnable = 0;        // �رվ��� 
            
            // ���³�ŵ�mosfet����        
            g_AO_SH36730x0.State.CHGControl = 0;          		// BQ769x0�رճ��MOSFET
            g_AO_SH36730x0.State.DSGControl = 0;          		// BQ769x0�رշŵ�MOSFET
            
            QActive_armX((QActive *)me, 0U, 1000U);     		// ������ʱ��0��1s

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         		// �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:
        {   
            // ��ʱ1s������mcu           
          //  NVIC_SystemReset();   
		  g_AO_SH36730x0.State.EnterSHIPModeAsk =1;//����͹���״̬
          g_SystemState.State.bit.EnterBootloaderAsk =1;//
        
            
            status = Q_HANDLED();
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                		// ��״̬Ϊtop״̬       
        }
        break;
    }
    return status;
}
//int AH_count=0;
s32	t_test244[5];
s32 l_ContactorCurrent = 0;
s32 time_IIC=1;
//u8 value=0;
// ------------------------------- ��ص������� ---------------------------------
// ÿ��BQ769x0оƬ����ֵ���ݶ�ȡ��ɺ�ִ�У�Լ100ms����ִ��һ��
// ��������Ϊ1mA��ʱ�侫��Ϊ1ms����������Ϊ1mAH
void BatteryCapacity_Update(AO_BMS * const me)
{
    // --------------------- ��������һ�����ݸ���֮��ʱ��飬���㱾�α仯�ĵ�ص��� -------------------------
    l_ContactorCurrent = 0;
    if(g_Output.State.bit.FanEnable > 0)
    {
        l_ContactorCurrent += me->Parameter.FanCurrent;   //�������е���
    }
    // Take care�� BMSIdleCurrentΪBMS����ĵ����
	// ���� ReadValueTimeOld ����0ʱ���㣬������һ��ֵ�����ϵ��ʼ��Ӱ��
	
	time_IIC=g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld;//������
	
	if (me->Variable.ReadValueTimeOld >0)
	{
		if (g_AO_SH36730x0.Output.ReadValueTime > me->Variable.ReadValueTimeOld)				
		{
			// ��������ʱ��BQ769оƬ����ֵ���¼���������1000ms��ʵ��100-200ms����
			if ((g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld) < 1000)//����ʱ�����1000ms 
			{ 
				//AH_count++;//--------------------------------------------------------------------------------------------------����
				//��ʱ���� ��λmAS
				// Take care��BMSIdleCurrent��������BMS����ĵ����+��ص��Էŵ�                        //BMS�����ص���   ���ȵ���*��������ʱ��  
				me->Variable.BatteryCapacityTemp += (g_AO_SH36730x0.Output.BatteryCurrent - me->Parameter.BMSIdleCurrent - l_ContactorCurrent) * (g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld);
			}
			else//����ʱ�䳬��1000ms���ù���
			{
				t_test244[0]++;
				t_test244[1] = g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld;
				
				// Lv0��BQ769оƬ����ֵ���¼������1000ms
				Protect_SetFaultCodeLv0(&g_Protect, BQ769_UPDATE_TOO_SLOW);
				
				me->Variable.BatteryCapacityTemp += (g_AO_SH36730x0.Output.BatteryCurrent - me->Parameter.BMSIdleCurrent) * 500;
			}
		}
		else
		{
			// �������ڳ�ʱ�����к�ReadValueTime����������Լ����������49�죡															
			if (g_AO_SH36730x0.Output.ReadValueTime < 1000)			// ��ReadValueTime < 1000��˵�����������ˣ���������   //����:˫��666
			{
				me->Variable.BatteryCapacityTemp += (g_AO_SH36730x0.Output.BatteryCurrent - me->Parameter.BMSIdleCurrent) * g_AO_SH36730x0.Output.ReadValueTime;
			}
			else
			{
				t_test244[2]++;
				t_test244[3] = g_AO_SH36730x0.Output.ReadValueTime - me->Variable.ReadValueTimeOld;
				
				// �����������һ����1000ms��û���£�����ñ���
				// Lv0��BQ769оƬ����ֵ���¼������1000ms
				Protect_SetFaultCodeLv0(&g_Protect, BQ769_UPDATE_TOO_SLOW2);//=========
				
				me->Variable.BatteryCapacityTemp += (g_AO_SH36730x0.Output.BatteryCurrent - me->Parameter.BMSIdleCurrent) * 500;
			}
		}
	}
	
    me->Variable.ReadValueTimeOld = g_AO_SH36730x0.Output.ReadValueTime;			// ��ֵ��䣬ǧ�������      //  
    
    
    // ------------------------- �������仯����1mAHʱ�����µ�ص��� -------------------------------
	// BatteryCapacityTemp��������λΪ1mA*ms      
    if (me->Variable.BatteryCapacityTemp >= 3600000)
    {
        // �����ӵĵ�������3600mAS����1mAH
        me->Output.BatteryCapacity += me->Variable.BatteryCapacityTemp / 3600000;
        
        // ���Ƶ������ܳ������������
        if (me->Output.BatteryCapacity > me->Parameter.BatteryFullCapacity)
        {
            me->Output.BatteryCapacity = me->Parameter.BatteryFullCapacity;
        }

        me->Variable.BatteryCapacityTemp = me->Variable.BatteryCapacityTemp % 3600000;      
    }
    else if (me->Variable.BatteryCapacityTemp <= -3600000)
    {
        // �����ٵĵ�������3600mAS����1mAH
        me->Output.BatteryCapacity += me->Variable.BatteryCapacityTemp / 3600000;
		
		// ���������ܳ��ָ�ֵ
        if (me->Output.BatteryCapacity < 0)
        {
            me->Output.BatteryCapacity = 0;
        }
        
        me->Variable.BatteryCapacityTemp = me->Variable.BatteryCapacityTemp % 3600000;      
    }
    
    // --------------------------------- ��������ٷֱ�SOC --------------------------------------
	// �˴��Գ���������99%����Ϊ����������ֵ����ֹ����һ�ŵ缴99%����
	// SOC��λΪ0.1%
	if(me->Output.FullCapacity > 0)//��ؿ�ʵ�ʳ������
    {       //�������
        if (me->Output.BatteryCapacity <= me->Output.FullCapacity * 100 / 101)//FullCapacity���ʵ�ʿɳ������mAH ��99%��Ϊ����ֵ����
        {
            me->Output.SOC = me->Output.BatteryCapacity * 1000 / (me->Output.FullCapacity * 100 / 101);//�����ٷֱ�1����
        }
        else
        {
            me->Output.SOC = 1000;
        }
    }
    else
    {
        if(g_SystemRecord.BatteryFullCapacity > 0)
        {
            me->Output.FullCapacity = g_SystemRecord.BatteryFullCapacity;
        }
        me->Output.SOC = 0;
    }
	 
	//TIM_SetCompare2(TIM2,(me->Output.SOC*3636)/1000);//PDI ��� ռ�ձȱ�ʾSOC ��һ����� //�˰汾BDI����ֻ��Ԥ����������ü�
	
	
	
}

s32 l_Voltage[4] = {0, 0, 0, 0};

// -------------------------- ��ص���������������1sʱ���� --------------------------
// ���ݵ����ѹ��������
// ����1������طŵ��������ϵ�ʱ����ѹ��ʣ�������һ�����Թ�ϵ��ͬʱ��ŵ�����йء�
void BatteryCapacity_Calibrate(AO_BMS * const me)
{
    
    u16 temp_voltage_value = 0;
    if(g_AO_BMS.Output.BatteryTemperatureHi < 100 && g_SystemParameter.BMS.Battery.BatteryTemperatureCheckMode)
    {
        if(g_SystemParameter.BMS.Discharge.dsg_tmp_low_k < 10 || g_SystemParameter.BMS.Discharge.dsg_tmp_low_k > 100)g_SystemParameter.BMS.Discharge.dsg_tmp_low_k = 50;
        temp_voltage_value = g_SystemParameter.BMS.Discharge.dsg_tmp_low_k * (100 - g_AO_BMS.Output.BatteryTemperatureHi) / 50;
    }
    
    // ���������㷨
    if (g_AO_SH36730x0.Output.BatteryCurrent < -100)      // �����ŵ�ʱ���ŵ��������100mA������ʣ���������
    {
        /*    // �ŵ��������0.5Cʱ��������ѹ�������С���仯
            if (g_AO_BQ769x0.Output.BatteryCurrent < -(s32)(g_SystemRecord.BatteryFullCapacity / 2))
            {
                // 1%������ 0.5C�ŵ磬��ѹ3.1V��1C�ŵ磬��ѹ3.0V,��ѹ���õ���3.01V
                l_Voltage[0] = 3100 - (g_AO_BQ769x0.Output.BatteryCurrent + g_SystemRecord.BatteryFullCapacity / 2) * 100 / g_AO_BQ769x0.Output.BatteryCurrent;
                if (l_Voltage[0] < 3100)
                {
                    l_Voltage[0] = 3100;
                }

                // 3%������0.5C�ŵ磬��ѹ3.2V��1C�ŵ磬��ѹ3.1V��1.5C�ŵ磬��ѹ3.0V
                l_Voltage[1] = 3200 - (g_AO_BQ769x0.Output.BatteryCurrent + g_SystemRecord.BatteryFullCapacity / 2) * 100 / g_AO_BQ769x0.Output.BatteryCurrent;
                if (l_Voltage[1] < 3150)
                {
                    l_Voltage[1] = 3150;
                }

                // 10%������0.5C�ŵ磬��ѹ3.3V��1C�ŵ磬��ѹ3.2V, 1.5C�ŵ磬��ѹ3.1V, 2C�ŵ磬��ѹ3.0V
                l_Voltage[2] = 3300 - (g_AO_BQ769x0.Output.BatteryCurrent + g_SystemRecord.BatteryFullCapacity / 2) * 100 / g_AO_BQ769x0.Output.BatteryCurrent;
                if (l_Voltage[2] < 3200)
                {
                    l_Voltage[2] = 3200;
                }

                // 20%������0.5C�ŵ磬��ѹ3.4V��1C�ŵ磬��ѹ3.3V, 1.5C�ŵ磬��ѹ3.2V, 2C�ŵ磬��ѹ3.1V
                l_Voltage[3] = 3400 - (g_AO_BQ769x0.Output.BatteryCurrent + g_SystemRecord.BatteryFullCapacity / 2) * 100 / g_AO_BQ769x0.Output.BatteryCurrent;
                if (l_Voltage[3] < 3250)
                {
                    l_Voltage[3] = 3250;
                }
            }
            else
            {
                // �ŵ����С��0.5C��������ѹ��0.5C�ŵ�Ϊ׼
                l_Voltage[0] = 3100;            // 1%
                l_Voltage[1] = 3200;            // 3%
                l_Voltage[2] = 3300;            // 10%
                l_Voltage[3] = 3400;            // 20%
            }*/

        // �ŵ��������0.5Cʱ��������ѹ�������С���仯
        if (g_AO_SH36730x0.Output.BatteryCurrent < -(s32)(g_SystemRecord.BatteryFullCapacity / 5))
        {
            // �ŵ����ÿ����0.5C��У׼��ѹ�µ�100mV�����������������ֵ
            // 1%������ 0.5C�ŵ磬��ѹ3.1V��1C�ŵ磬��ѹ3.0V,��ѹ���õ���3.01V
            l_Voltage[0] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 + g_AO_SH36730x0.Output.BatteryCurrent * 50 / g_SystemRecord.BatteryFullCapacity - temp_voltage_value;
            if (l_Voltage[0] < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1)
            {
                l_Voltage[0] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit1;
            }

            // 3%������0.5C�ŵ磬��ѹ3.2V��1C�ŵ磬��ѹ3.1V��1.5C�ŵ磬��ѹ3.0V
            l_Voltage[1] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 + g_AO_SH36730x0.Output.BatteryCurrent * 50 / g_SystemRecord.BatteryFullCapacity - temp_voltage_value;
            if (l_Voltage[1] < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2)
            {
                l_Voltage[1] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit2;
            }

            // 10%������0.5C�ŵ磬��ѹ3.3V��1C�ŵ磬��ѹ3.2V, 1.5C�ŵ磬��ѹ3.1V, 2C�ŵ磬��ѹ3.0V
            l_Voltage[2] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 + g_AO_SH36730x0.Output.BatteryCurrent * 50 / g_SystemRecord.BatteryFullCapacity - temp_voltage_value;
            if (l_Voltage[2] < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3)
            {
                l_Voltage[2] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit3;
            }

            // 20%������0.5C�ŵ磬��ѹ3.4V��1C�ŵ磬��ѹ3.3V, 1.5C�ŵ磬��ѹ3.2V, 2C�ŵ磬��ѹ3.1V
            l_Voltage[3] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 + g_AO_SH36730x0.Output.BatteryCurrent * 50 / g_SystemRecord.BatteryFullCapacity - temp_voltage_value;
            if (l_Voltage[3] < g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4)
            {
                l_Voltage[3] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltageLowLimit4;
            }
        }
        else
        {
            // �ŵ����С��0.5C��������ѹ��0.5C�ŵ�Ϊ׼
            l_Voltage[0] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage1 - temp_voltage_value;            // 1%
            l_Voltage[1] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage2 - temp_voltage_value;            // 3%
            l_Voltage[2] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage3 - temp_voltage_value;            // 10%
            l_Voltage[3] = g_SystemParameter.BMS.CapacityCalibrate.CalibrateVoltage4 - temp_voltage_value;            // 20%
        }

        // For test���ŵ�ʱ���ݵ�ѹ�Ե������в���,�������
        // ����ѹ����3.4V����ʱ��0.5C�ŵ磬��ѹ�����½�����ʱ�洢�ĵ���ֻ�����ֵ��10%
        // ��AH������ĵ��������ڴ���ֵ��������Сʣ�����
        if (g_AO_SH36730x0.Output.SingleMinVoltage < l_Voltage[0] && me->Output.SOC > 10)
        {
            // ����������Ƶ�����1%
//            if (me->Output.BatteryCapacity > (me->Parameter.BatteryDesignCapacity / 100))
            {
                // ÿ1s����1/300��������60s����Լ20%����
                me->Output.BatteryCapacity -= me->Output.FullCapacity / 300;
                if (me->Output.BatteryCapacity < 0)
                {
                    me->Output.BatteryCapacity = 0;
                }
            }
        }
        else if (g_AO_SH36730x0.Output.SingleMinVoltage < l_Voltage[1] && me->Output.SOC > 30)
        {
            // ����������Ƶ�����3.3%
//            if (me->Output.BatteryCapacity > (me->Parameter.BatteryDesignCapacity / 30))
            {
                // ÿ1s����1/500��������60s����Լ12%����
                me->Output.BatteryCapacity -= me->Output.FullCapacity / 500;
                if (me->Output.BatteryCapacity < 0)
                {
                    me->Output.BatteryCapacity = 0;
                }
            }
        }
        else if (g_AO_SH36730x0.Output.SingleMinVoltage < l_Voltage[2] && me->Output.SOC > 100)
        {
            // ����������Ƶ�����10%
//            if (me->Output.BatteryCapacity > (me->Parameter.BatteryDesignCapacity / 10))
            {
                // ÿ1s����1/750��������60s����Լ8%����
                me->Output.BatteryCapacity -= me->Output.FullCapacity / 750;
                if (me->Output.BatteryCapacity < 0)
                {
                    me->Output.BatteryCapacity = 0;
                }
            }
        }
        else if (g_AO_SH36730x0.Output.SingleMinVoltage < l_Voltage[3] && me->Output.SOC > 200)
        {
            // ����������Ƶ�����20%
//            if (me->Output.BatteryCapacity > (me->Parameter.BatteryDesignCapacity / 5))
            {
                // ÿ1s����1/1200��������60s����Լ5%����
                me->Output.BatteryCapacity -= me->Output.FullCapacity / 1200;
                if (me->Output.BatteryCapacity < 0)
                {
                    me->Output.BatteryCapacity = 0;
                }
            }
        }
    }
}


// End of AO_BMS.c


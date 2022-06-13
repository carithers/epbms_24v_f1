/* ==================================================================================

 File name:     AO_SH36730x0.c
 Originator:    BLJ
 Description:   SH36730x0ͨ�ż��������״̬������Ҫ����I2Cͨ�ż��Ĵ�����д
 Take care��    ����״̬����󣬶�ʱ��1���������Զ�ʱ

=====================================================================================
 History:
-------------------------------------------------------------------------------------
 07-14-2016     Version 0.9.1           ����CRCУ�����������֤��
 07-01-2016     Version 0.9.0           ����������ɣ����ֹ��ܿ����Ż��������ڲ���
 01-02-2015     Version 0.0.1           ���Թ���ͨ��
-----------------------------------------------------------------------------------*/

#include "stm32f0xx.h"                          // MM32�����Ĵ�������ͷ�ļ�
#include "string.h"                             // ����memset������
#include "system_core.h"                        // ϵͳ����ͷ�ļ�
#include "qpn_port.h"                           // ״̬��ͷ�ļ�
#include <math.h>


//u8 DATA_DEBUG[30]={0};

/*
*   ״̬�����м�¼
*   ������
*/

void AO_SH36730x0_Record(AO_SH36730x0 * const me, u16 i)
{
    u16 n = 0;

    // ���м�¼����ƶ�һλ
    for (n = (SH36730x0_RECORD_LENGTH - 1); n > 0; n--)
    {
        me->Record[n] = me->Record[n-1];
    }

    // �����¼�¼
    me->Record[0] = i;
}


/*
*  �ڲ����ú�������
*/

void SH36730x0_ParameterUpdate(AO_SH36730x0 * const me);// ����ϵͳ�������ã�����SH36730оƬ�Ĵ�������
void SH36730x0_SampleUpdate(AO_SH36730x0 * const me);  // SH36730x0оƬ����ֵ������º���    
void SH36730x0_UpdateControl(AO_SH36730x0 * const me);// SH36730x0оƬ���Ƹ��º���

/*
*  �������
*/ 
void AO_SH36730x0_ctor(AO_SH36730x0 * const me) {
    QActive_ctor(&me->super, Q_STATE_CAST(&AO_SH36730x0_initial));
}

/*
*   ��ʼת��
*/
QState AO_SH36730x0_initial(AO_SH36730x0 * const me) {
    return Q_TRAN(&AO_SH36730x0_StartWait);
}

/*
*   1.�ȴ�EEPROM��ȡ���,����AFE���� 
*/


QState AO_SH36730x0_StartWait(AO_SH36730x0 * const me) {   
  
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {  
			//SH36730x0_ParameterUpdate(me);//=============================================������------------------
            //QACTIVE_POST((QActive *)&g_AO_SH36730x0, START_SIG, 0);//=========================
            AO_SH36730x0_Record(me, 1);
            me->Variable.ClearSystemStateCount = 0; // ���ϵͳ��ʱ����   
			
            status = Q_HANDLED(); 
        }
        break;
		
        case Q_EXIT_SIG:
        {
            status = Q_HANDLED();
        }
        break;

        case START_SIG:   //AO_EEPROM�к������6 61 63  ����  POST                             
        {      
		    
			SH36730x0_ParameterUpdate(me);//SH36730оƬ��ʼ�������ݳ�ʼ��
			
			//DATA_DEBUG[0]++;//==================================================================================	
            me->Variable.Variable_bits.bit.WriteParameterAllow = 1;  
		    //QACTIVE_POST((QActive *)&g_AO_SH36730x0, START_SIG, 0); 
			
            status = Q_TRAN(&AO_SH36730x0_Normal); 
        }
        break;		
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);            
        }
        break;
    }
    
    return status;
}
	


/* 
*     2.ͨ������״̬ 
*     ����״̬�ĸ�״̬,������ͨѶ��ʱ��ͨѶ�����¼�
*/

u8 count_set=0;    //����AFE����
u8 updata_count=0; //���¿���ֵ����
u8 close_step=0;   //��ȡǰ�رվ������
u8 Read_times = 0; //��ȡ����ֵ����
u8 SHIP_step=0;    //�͹��Ĳ���
u8 Balance_time=0; //����ʱ��
u8 Delay_time=0;   //����ʱ��
u8 Balance_flag=0; //����ʱ��
QState AO_SH36730x0_Normal(AO_SH36730x0 * const me) {
    
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            AO_SH36730x0_Record(me, 2);
	
            status = Q_HANDLED();           
        }
        break;
        
        case Q_INIT_SIG: 
        {   
			//DATA_DEBUG[1]++;//==================================================================================
            status = Q_TRAN(&AO_SH36730x0_ReadParameter); // Ĭ����ת����ȡ�Ĵ���״̬   
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
			
            status = Q_HANDLED();
        }
        break;
        
        case Q_TIMEOUT_SIG:  //��ʱ��0 1s ��ʱ��ʱ
        {
			//DATA_DEBUG[2]++;//===============================================================
            QActive_disarmX((QActive *)me, 0U);   //�رն�ʱ��
			//================================================
			//ͨѶ������߳�ʱʱ��������־
			 count_set=0;
			 updata_count=0;
			 close_step=0;
			 Read_times = 0;
			 SHIP_step=0;
			 Balance_time=0;
			//================================================			
			
			me->Variable.I2CFaultRetryCount++;
			
			if (me->Variable.I2CFaultRetryCount <= 8)//ͨѶʧ�����Դ���С��
			{
				 
				//I2C_BQ769xx_DeviceInit();//ͨ��ʧ�����³�ʼ��Ӳ��
				
                me->Variable.I2CTryRecoverFlag = 1;  //�����޸�I2C���߱�־λ   				
         
				QActive_armX((QActive *)me, 1U, 100U);//��ʱ��1
				
				status = Q_HANDLED();				
			}
			else
			{
				Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_I2C_OVERTIME);// ����I2Cͨ�ų�ʱ��־λ
				
				status = Q_TRAN(&AO_SH36730x0_Fault); //��ת��ͨ�Ź���״̬
			}
        }
        break;  
		
		
        case Q_TIMEOUT1_SIG:  //��ʱ��0��ʱ�����³�ʼ������ 100ms����������
        {	
			QActive_disarmX((QActive *)me, 1U); //�رն�ʱ��1
			
			status = Q_TRAN(&AO_SH36730x0_ReadParameter); 
        }
        break;  
		
		
        case I2C_SH36730x0_FAIL_SIG:// оƬͨѶʧ��//Ӧ��ʧ�ܻ�CRCУ��ʧ��
        {
            //DATA_DEBUG[3]++;//==============================================================================
			QActive_disarmX((QActive *)me, 0U); //�رն�ʱ��0
			
            me->Variable.I2CFaultRetryCount++;  
			
			//================================================
			//ͨѶ������߳�ʱʱ��������־
			 count_set=0;
			 updata_count=0;
			 close_step=0;
			 Read_times = 0;
			 SHIP_step=0;
			//================================================	
			
			if (me->Variable.I2CFaultRetryCount <= 8) //����ʧ��8�� 
			{
				I2C_BQ769xx_DeviceInit();//SH3673xxоƬI2C����ͨ��ģ���ʼ��			
				
				QActive_armX((QActive *)me, 1U, 100U); //������ʱ��1��100ms
				
				status = Q_TRAN(&AO_SH36730x0_ReadParameter); 
			}
			else
			{

				Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_I2C_FAIL);
				// ͨ��ʧ�ܣ���ת��ͨ�Ź���״̬
				status = Q_TRAN(&AO_SH36730x0_Fault);
			}
        }
        break;   		 
                                 
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);                        // ��״̬Ϊtop״̬       
        }
        break;
    }
    
    return status;
}


/*
*    3.��ȡFLAG1 FLAG2����
*/

QState AO_SH36730x0_ReadParameter(AO_SH36730x0 * const me)
 {  
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
			//DATA_DEBUG[4]++;//==================================================================================
            AO_SH36730x0_Record(me, 3);
			//��ȡоƬFLAG���ϱ�־  һ�ζ�ȡ����                                                                                         
			I2C_BQ769x0_Read(&g_I2C_SH36730x0,Addr_ZYBQ,(u8*)&me->SH36730x0Register.FLAG1,0x00 ,1);
			 
            QActive_armX((QActive *)me, 0U, 1000U);//1s��ʱ���� ��ʱ�ڸ�״̬����
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {
            //DATA_DEBUG[5]++;//==================================================================================	
            QActive_disarmX((QActive *)me, 0U);
            me->Variable.CRCCheckRetryCount = 0;//�յ���ȷ����֡ʱ�����CRCУ�������� 
			
            //�ж�SH36730оƬ�ޱ��� ��û�з���ͨѶ��ʱ ���Ź���� ��·���� ���䱣��
            if (me->SH36730x0Register.FLAG1.all == 0x00) 
            {	
				//DATA_DEBUG[6]++;//==================================================================================
				if (me->Variable.Variable_bits.bit.WriteParameterAllow > 0)
				{      
                //DATA_DEBUG[7]++;//==================================================================================					
					//EEPROMϵͳ������ȡ��ɣ���ʼ����SH36730x0оƬ
					status = Q_TRAN(&AO_SH36730x0_WriteParameter);                
				}
				else
				{
					//DATA_DEBUG[8]++;//==================================================================================
					//EEPROMϵͳ����δ��ȡ��� ����ת���ȴ�����ϵͳ����״̬
					status = Q_TRAN(&AO_SH36730x0_WaitWriteParameter); 
				}  
             
            }
            else  // SH36730оƬ���ϱ�־���
            {
                if (me->Variable.ClearSystemStateCount < 5)
                {
                    me->Variable.ClearSystemStateCount++;
                    
                    status = Q_TRAN(&AO_SH36730x0_ClearSystemState);//���FLAG1��־λ
					
                }
                else
                {
					//DATA_DEBUG[10]++;//==================================================================================
                    me->Variable.ClearSystemStateCount = 0;   
                    
                    Protect_SetFaultCodeLv0(&g_Protect, SYS_STAT_NOT_CLEAR);// ���ù��ϱ�־λ,оƬϵͳ�����޷��ָ�  

                    status = Q_TRAN(&AO_SH36730x0_Fault);// ��ת������״̬          
                } 
            }
        }
        break;
 

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // ��״̬ΪNormal״̬   
        }
        break;
    }
    
    return status; 

 }	
 

/*
*    ����SH36730оƬ
*/
//u8 count_set=0;
QState AO_SH36730x0_WriteParameter(AO_SH36730x0 * const me) 
{
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {           
            AO_SH36730x0_Record(me,33); 
			
            //SH36730x0_ParameterUpdate(me);//SH36730оƬ��ʼ�������ݳ�ʼ��		
			         
			switch(count_set) //���²���д��SH36730x0�Ĵ���
			{
				default:
				case 0:         //�����ʹ�� Ӳ�����䱣��ʹ�� ���Ź�ʹ�� ʹ��CTLD�ܽ����ȿ��Ʒŵ�MOS
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x03, (u8*)&me->SH36730x0Register.INT_EN.all, 1);
				    break;
				case 1:         //�����ʹ�� Ӳ�����䱣��ʹ�� ���Ź�ʹ�� ʹ��CTLD�ܽ����ȿ��Ʒŵ�MOS
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x04, (u8*)&me->SH36730x0Register.SCONF1.all, 1);
				    break;
			    case 2:         //ʹ�ܸ�λ�ⲿMCU����(Ĭ��)  ����ALARMӲ����������͵�ƽ
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x05, (u8*)&me->SH36730x0Register.SCONF2.all, 1);
				    break;
				case 3:         //VADCģ��ʹ�� ����VADCת������(50msĬ��) CADCģ��ʹ�� ����CADC�������ɼ� ���� 13bit ����ת��ʱ��62.5ms
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x06, (u8*)&me->SH36730x0Register.SCONF3.all, 1);
					break;
					
			    case 4:         //�رվ���
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x07, (u8*)&me->SH36730x0Register.SCONF4.all, 1);
				    break;
				case 5:         //�رվ���
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x08, (u8*)&me->SH36730x0Register.SCONF5.all, 1);
					break;
				case 6:         //����Ӳ����·������ʱ  Ӳ����·������ѹ���� ��λ�ⲿMCU������ѡ�� CADC�ɼ���Χ����(200mv)
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x09, (u8*)&me->SH36730x0Register.SCONF6.all, 1);
				    break;
				case 7:         //���Ź����ʱ������(30s) ��ŵ�״̬�����ֵ����  Ӳ�����䱣����ʱѡ��
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x0A, (u8*)&me->SH36730x0Register.SCONF7.all, 1);
					break;
			   	case 8:         //Ӳ�����䱣����ֵ����
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x0B, (u8*)&me->SH36730x0Register.SCONF8.all, 1);
					break;
			  	case 9:         //Ӳ�����䱣����ֵ���� (SCONF8 SCONF9 )*5.86mv
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x0C, (u8*)&me->SH36730x0Register.SCONF9.all, 1);
					break;
				case 10:         //����͹��Ŀ���λ
					I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x0D, (u8*)&me->SH36730x0Register.SCONF10.PINn, 1);
					break;
			
			 }
            
            QActive_armX((QActive *)me, 0U, 1000U); 
            //DATA_DEBUG[13]++;//==================================================================================
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);           
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:  
        {
			//DATA_DEBUG[14]++;//==================================================================================
            QActive_disarmX((QActive *)me, 0U);            
			count_set++;
			
			if(count_set>10)
			{
				//DATA_DEBUG[15]++;//==================================================================================
				count_set=0;
				
				QACTIVE_POST((QActive *)&g_AO_BMS, START_SIG,0);  //SH367730���óɹ��������ź�����BMS����,�鿴����BMS����SH36730���󷢲���Щ�ź����ٴ�����AFEоƬ
                 
				//status = Q_TRAN(&AO_SH36730x0_Idle); //��ת������״̬ 
                status = Q_TRAN(&AO_SH36730x0_ReadConfig);               
			}
			else status = Q_TRAN(&AO_SH36730x0_WriteParameter);    
				
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // ��״̬ΪNormal״̬   
        }
        break;
    }
    
    return status;
 }	 

 
 
/*
*���ö�ȡ״̬����  ��ʱû�н���У��ֻ�Ƕ�ȡһ��
*������Ҫ���Զ�ʱ��ȡУ�������Ƿ����쳣
*
*/
 
 u8 read_config=0;
QState AO_SH36730x0_ReadConfig(AO_SH36730x0 * const me) 
{
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {           			         
			switch(read_config) //��д�����ö�ȡһ����У��
			{
				default:
				case 0:         //�����ʹ�� Ӳ�����䱣��ʹ�� ���Ź�ʹ�� ʹ��CTLD�ܽ����ȿ��Ʒŵ�MOS
					me->SH36730x0Register.INT_EN.all=0;
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.INT_EN.all,0x03,1);//FLAG1
				    break;
				case 1:         //�����ʹ�� Ӳ�����䱣��ʹ�� ���Ź�ʹ�� ʹ��CTLD�ܽ����ȿ��Ʒŵ�MOS
					me->SH36730x0Register.SCONF1.all=0;
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF1.all,0x04,1);
				    break;
			    case 2:         //ʹ�ܸ�λ�ⲿMCU����(Ĭ��)  ����ALARMӲ����������͵�ƽ
					me->SH36730x0Register.SCONF2.all=0;
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF2.all,0x05,1);
				    break;
				case 3:         //VADCģ��ʹ�� ����VADCת������(50msĬ��) CADCģ��ʹ�� ����CADC�������ɼ� ���� 13bit ����ת��ʱ��62.5ms
					me->SH36730x0Register.SCONF3.all=0;
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF3.all,0x06,1);	
				break;
					
			    case 4:         //�رվ���
					me->SH36730x0Register.SCONF4.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF4.all,0x07,1);
				    break;
				case 5:         //�رվ���
					me->SH36730x0Register.SCONF5.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF5.all,0x08,1);
					break;
				case 6:         //����Ӳ����·������ʱ  Ӳ����·������ѹ���� ��λ�ⲿMCU������ѡ�� CADC�ɼ���Χ����(400mv)
					me->SH36730x0Register.SCONF6.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF6.all,0x09,1);
				    break;
				case 7:         //���Ź����ʱ������(30s) ��ŵ�״̬�����ֵ����  Ӳ�����䱣����ʱѡ��
					me->SH36730x0Register.SCONF7.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF7.all,0x0A,1);
					break;
			   	case 8:         //Ӳ�����䱣����ֵ����
					me->SH36730x0Register.SCONF8.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF8.all,0x0B,1);
					break;
			  	case 9:         //Ӳ�����䱣����ֵ���� (SCONF8 SCONF9 )*5.86mv
					me->SH36730x0Register.SCONF9.all=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF9.all,0x0C,1);
					break;
				case 10:         //����͹��Ŀ���λ
					me->SH36730x0Register.SCONF10.PINn=0;
				I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.SCONF10.PINn,0x0D,1);
					break;
			
			 }
            
            QActive_armX((QActive *)me, 0U, 1000U); 
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);           
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:  
        {
			
            QActive_disarmX((QActive *)me, 0U);            
			read_config++;
			
			if(read_config>10)
			{
				
				read_config=0;
				
				QACTIVE_POST((QActive *)&g_AO_BMS, START_SIG,0);  

                status = Q_TRAN(&AO_SH36730x0_Idle);               //��ת������״̬
			}
			else status = Q_TRAN(&AO_SH36730x0_ReadConfig);    
				
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // ��״̬ΪNormal״̬   
        }
        break;
    }
    
    return status;
 }
 
//---------------------------------------------------------------------------------------------------------------
 
 

/*
*   ���к���  
*/ 
 
QState AO_SH36730x0_Idle(AO_SH36730x0 * const me) 
{ 
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {            
            AO_SH36730x0_Record(me, 4);
					
            // ��ʱ200ms����ʼ��һ�����ݶ�ȡ
      		
            QActive_armX((QActive *)me, 1U,200U);
            status = Q_HANDLED();           
        }
        break;
				
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 1U);                 // �رն�ʱ��1
			
                                                                
            status = Q_HANDLED();
        }
        break;
		

        case Q_TIMEOUT1_SIG:                                    // ���ڶ�ʱ��1
        {      
			    //DATA_DEBUG[16]++;//==================================================================================
			    QActive_disarmX((QActive *)me, 1U);
			
			
            if (me->State.EnterSHIPModeAsk== 0)            
            {               
				
			    status = Q_TRAN(&AO_SH36730x0_UpdateControl);   // ��ת�����Ƹ���״̬  
				//status = Q_TRAN(&AO_SH36730x0_CloseBalance);
            }
            else                                                //����͹���ģʽ����
            {
                status = Q_TRAN(&AO_SH36730x0_EnterSHIP);       // ��ת������͹���״̬
            }
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // ��״̬ΪNormal״̬   
        }
        break;
    }
    
    return status;
 }

/*
*   ���¿���ֵ������  ���س�ŵ��·
*/

 QState AO_SH36730x0_UpdateControl(AO_SH36730x0 * const me)
{
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            AO_SH36730x0_Record(me, 10);
            
            
			 SH36730x0_UpdateControl(me);//���ϵͳ���ϱ�־λ�����¾�����Ƽ���ŵ�MOS����ֵ//����BMSϵͳ��־�����Ƴ�ŵ�MOS ������״̬
			switch(updata_count)
			{
				case 0:         //��ŵ�MOS���� ���º���Ҳ����
					
				     me->SH36730x0Register.SCONF2.bit.DSG_C=me->State.DSGControl;
				     me->SH36730x0Register.SCONF2.bit.CHG_C=me->State.CHGControl;
				   
				   //--------------------------------����͵�ѹ��ȡʱ���޸Ŀ����õ��ξ���ʱ��--------------------------------------------------			    
				    if((me->State.BatteryBalanceEnable ==1&&((me->SH36730x0Register.SCONF4.all+me->SH36730x0Register.SCONF5.all)>0))||(me->State.test_ForceBatteryBalancePointer>0))
					{
						if(Balance_time<5)//����1s�ڲ��ڸ��� ������ѹ
						{
						  Balance_flag=0;
						  updata_count=3;
						}
						else if(Balance_time==5)//�رվ���
						{
						 me->SH36730x0Register.SCONF4.all=0x00;
						 me->SH36730x0Register.SCONF5.all=0x00;
						  
						  updata_count=1;
						  Balance_flag=0;
						}
						else if(Balance_time==6)//��д����Ĵ���
						{	  
						 me->SH36730x0Register.SCONF4.all=0x00;
						 me->SH36730x0Register.SCONF5.all=0x00;
						  updata_count=1;
						  Balance_flag=0;
						}

						else if(Balance_time==7)//����ѹ
                       { 
						  Balance_flag=1;
						  updata_count=3;
						}
					   else if(Balance_time==8)//����ѹ
                       { 
						  Balance_time=0;
						  Balance_flag=0;
						  updata_count=1;
						}
							
					  Delay_time=0;
					  Balance_time++;
					}
					else
					{  
						Balance_time=0;//�رվ���
						if(Delay_time<=1)
						{
					    me->SH36730x0Register.SCONF4.all=0x00;
						me->SH36730x0Register.SCONF5.all=0x00;
						Balance_flag=0;	
						updata_count=1;
						}
						else if(Delay_time==2)//��д����Ĵ���
						{
						updata_count=3;
						Balance_flag=0;
						}
						else if(Delay_time==3)//��ʼ��ȡ��ѹ
						{
						updata_count=3;
						Balance_flag=1;
						Delay_time=0;
						}

						Delay_time++;
					}
					//-------------------------------------------------------------------------------------------------------
					I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x05,(u8*)&me->SH36730x0Register.SCONF2.all,1);//����MOS
					
					break;
					
				case 1:         //�������
					if(Balance_time==7||Balance_time==6||Delay_time==2||Delay_time==3)//�رվ���
					{
						 me->SH36730x0Register.SCONF4.all=0x00;
						 me->SH36730x0Register.SCONF5.all=0x00;
					}
					I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x07,(u8*)&me->SH36730x0Register.SCONF4.all,1);
				    updata_count=2;
					break;
				case 2:         //�������
					if(Balance_time==7||Balance_time==6||Delay_time==2||Delay_time==3)//�رվ���
					{
						 me->SH36730x0Register.SCONF4.all=0x00;
						 me->SH36730x0Register.SCONF5.all=0x00;
					}
					I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x08,(u8*)&me->SH36730x0Register.SCONF5.all,1);
					updata_count=3;
					break;

				
				default:
					break;
			}

            //DATA_DEBUG[17]++;//==================================================================================        
            QActive_armX((QActive *)me, 0U, 1000U);             // ������ʱ��0,1000ms

            status = Q_HANDLED();
        }
        break;
                                                                
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {
			 //DATA_DEBUG[18]++;//================================================================================
           QActive_disarmX((QActive *)me, 0U);        
			
					
     if(updata_count==3)
		 {
			//DATA_DEBUG[19]++;//==================================================================================
		    updata_count=0;	 
			status = Q_TRAN(&AO_SH36730x0_CloseBalance);  
			
		 }        
		 else
			status = Q_TRAN(&AO_SH36730x0_UpdateControl);
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);           //��״̬Ϊģ��ͨ����״̬   
        }
        break;
    }
    
    return status;
 

 }


/*
*  ��ȡ��ѹǰ�رվ���  
*/
// u8 close_step=0;

 QState AO_SH36730x0_CloseBalance(AO_SH36730x0 * const me)
 {
	QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {		 
			  
        switch(close_step)
			 {
				case 0://0x02	//ϵͳ״̬��־��ȡ
				    I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.BSTATUS,0x02,1);//BSTATUS
				    break;
				case 1://0x00	//���ϱ�־��ȡ
					I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.FLAG1,0x00,1);//FLAG1
				    break;
				case 2://AFE�ڲ��¶ȶ�ȡ ���ϴ󡣡���������
					I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.TEMP[0],0x26,1);//0.17*W-270//==============================================
				    break;
				case 3:
					I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.TEMP[1],0x28,1);//0.17*W-270//==============================================
				    break;
				default:
					break;
			  }
			
            QActive_armX((QActive *)me,0U,1000U);
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {
			//DATA_DEBUG[20]++;//==================================================================================
            QActive_disarmX((QActive *)me, 0U);
			
            close_step++;			
            if(close_step>3)
			{   
			    //DATA_DEBUG[21]++;//==================================================================================
				close_step=0;
				
				status = Q_TRAN(&AO_SH36730x0_ReadValue);//�رվ���ɹ���ת����ѹ������ȡ
			}

			else                                             
				status = Q_TRAN(&AO_SH36730x0_CloseBalance);
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // ��״̬ΪNormal״̬   
        }
        break;
    }
    
    return status;
 
 }
 
 
/*
*    ��ȡ����ֵ
*/
// u8 Read_times = 0;
 QState AO_SH36730x0_ReadValue(AO_SH36730x0 * const me)
{
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {       
		
            AO_SH36730x0_Record(me, 9);  
			//DATA_DEBUG[28]++;//==================================================================================
			//��ȡSH36730x0 �����ѹ���ݣ�0x0EH-0x21H, ֻ����8�����Դ�0x12H��ʼ��ȡ //һ�ζ�2���Ĵ���
			switch(Read_times)
			{
				//case 0://0x10
				case 0://0x12 
				case 1://0x14
				case 2://0x16
				case 3://0x18
				case 4://0x1A
				case 5://0x1C
				case 6://0x1E
			    case 7://0x20
					if((me->SH36730x0Register.FLAG2.bit.VADC>0)&&Balance_flag==1)//&&Balance_flag==1
					{
						I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.CELL[Read_times],0x12+(Read_times)*2,1);
					 
					}
					else if(me->SH36730x0Register.FLAG2.bit.VADC>0)
					{
						//I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.TEMP[0],0x26,1);
						I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.BSTATUS,0x02,1);//BSTATUS
						Read_times=7;
					}
			         break;

				case 8://0x2A
				     Balance_flag=0;
					 if(me->SH36730x0Register.FLAG2.bit.CADC>0)
					 {
						 
						I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.CUR.CURH,0x2A,1);//��ȡ��ŵ����ֵ
					 }
					 else 
					 {
						I2C_BQ769x0_Read(&g_I2C_SH36730x0, Addr_ZYBQ,(u8*)&me->SH36730x0Register.BSTATUS,0x02,1);//BSTATUS
					 }

			         break;
					
				default:
					break;
			}

            
            QActive_armX((QActive *)me, 0U, 1000U);             // ������ʱ��0��1000ms
            status = Q_HANDLED();           
        }
        break;
		   
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {   
		   //DATA_DEBUG[23]++;//==================================================================================		
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
            
            me->State.SampleUpdateCount++;
						
            // ��I2C���Դ�����Ϊ�㣬����ÿ10�β����ɹ�����һ��
            // ��ȷ��Ƶ������ʧ��ʱ�����д���I2C�����������
            if (me->Variable.I2CFaultRetryCount > 0)
            {
                if ((me->State.SampleUpdateCount % 10) == 0)
                {
                    me->Variable.I2CFaultRetryCount--;
                }
            }
             
            //����ֵ���º��������ϼ�⣬����ֵ�����//���ݲ����� Output
			
            //SH36730x0_SampleUpdate(me);   //���ǵ�����ȫ����ȡ����»��Ƕ�һ������һ��
            
            //��������++����ֵ��ʼΪ0�����Ϊ30000������������Ϊ100��ע�ⲻҪС��20��ǰ20�β�����ѹ����Ƿѹ/��ѹ��⣩
            me->Variable.SampleUpdateCount++;                   
            if (me->Variable.SampleUpdateCount > 100)
            {
                me->Variable.SampleUpdateCount = 100;
            }                                     
		   Read_times++; 
       if(Read_times>8)
			{
				
				//DATA_DEBUG[24]++;//==================================================================================
			    Read_times = 0;
				
				SH36730x0_SampleUpdate(me);  //���� 
				 QACTIVE_POST((QActive *)&g_AO_BMS, BMS_UPDATE_SIG, 0);
				
				status = Q_TRAN(&AO_SH36730x0_Idle);// ��ת������״̬
			}
			
			else status = Q_TRAN(&AO_SH36730x0_ReadValue);//û���������
			
        }
        break;  
        
        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);
        }
        break;
    }
    
    return status;

 } 
 
/*
*    �ȴ�����EEPROM��ȡ��ɵ������ź�  
*    ���յ�START_SIG�����ִ��
*    5s��û�н���϶�EEPROM��ȡ����
*    Ӧ���ڴ˺�������TIMEOUT_SIG��ǩ����ʱ�¼�
*/

QState AO_SH36730x0_WaitWriteParameter(AO_SH36730x0 * const me)
 {
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {            
            AO_SH36730x0_Record(me, 321);
            //DATA_DEBUG[11]++;//=================================================================
            // ��ʱ5s����5s��EEPROM��ʷ���ϲ�����δ��ȡ��ɣ���϶�������//5s�ȴ�ʱ��������ѭ���͵δ�ʱ���ж�ֱ����ѭ��������ʱ����5�볬ʱ ʱ�䴥����ʼִ��
            QActive_armX((QActive *)me, 0U, 5000U);//

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;
        
        case START_SIG: // �յ������¼�����������BQ769x0оƬ����
        {
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
            //DATA_DEBUG[12]++;//=========================================================
            // ��λϵͳEEPROM�����Ѷ�ȡ��־λ������д��BQ769x0оƬ������
            me->Variable.Variable_bits.bit.WriteParameterAllow = 1;
            
            // ��ת������ϵͳ����״̬
            status = Q_TRAN(&AO_SH36730x0_WriteParameter); 
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // ��״̬ΪNormal״̬   
        }
        break;
    }
    
    return status;
 
 }			 



/*
*  ���FLAG1���ϱ�־λ
*/
 u8 clear_step=0;
 QState AO_SH36730x0_ClearSystemState(AO_SH36730x0 * const me)
 {
	QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {
            AO_SH36730x0_Record(me,301);
			if(clear_step==0)
			{	
				clear_step=1;
				me->SH36730x0Register.SCONF1.bit.LTCLR=1;// ���SH36730x0оƬ������Ϣ
			}
			else if(clear_step==1)
			{	clear_step=2;
				me->SH36730x0Register.SCONF1.bit.LTCLR=0;	
			}
            I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ, 0x04,(u8*)&me->SH36730x0Register.SCONF1.all, 1);
            
            QActive_armX((QActive *)me, 0U, 1000U);
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);
            
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:
        {
			//DATA_DEBUG[9]++;//==================================================================================
            QActive_disarmX((QActive *)me, 0U);             
            if(clear_step==2)
			{
				clear_step=0;
				status = Q_TRAN(&AO_SH36730x0_ReadParameter);//������λ�ǲ����Ѿ������
			}
			else                                             
				status = Q_TRAN(&AO_SH36730x0_ClearSystemState);
        }
        break;

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);               // ��״̬ΪNormal״̬   
        }
        break;
    }
    
    return status;
 
 }	


/*
*   ����͹���ģʽ
*   ����������ӱAFEоƬ���Զ���λ����͹���
*   ��Ϊ����ϵͳ��Դ��AFE�����LDO������
*/ 
u8 WDT_FLAG=0;
QState AO_SH36730x0_EnterSHIP(AO_SH36730x0 * const me) 
{ 
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG: 
        {            
            AO_SH36730x0_Record(me,12);
                      
			//��Ҫ����ʱ��Ҫ�ر�AFE���Ź���MCU���Ź����ٹرճ�ŵ�MOS(ʵ���ڽ���͹���ǰMOS��ر�),֮�����������λ������bootloader
			if(g_SystemState.State.bit.EnterBootloaderAsk ==1)
			{	
				
			 switch(WDT_FLAG)
			 {
				default:
                case 0:
                {
                 me->SH36730x0Register.SCONF1.bit.WDT_EN  = 0;//AFE�رտ��Ź�
                 I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ,0x04,(u8*)&me->SH36730x0Register.SCONF1.all,1);  // NVIC_SystemReset(); //�����λ
				}
				break;
				
				 case 1:
                {
                 me->SH36730x0Register.SCONF2.bit.CHG_C = 0;
				 me->SH36730x0Register.SCONF2.bit.DSG_C = 0;
                 I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x05,(u8*)&me->SH36730x0Register.SCONF2.all,1);//�ر�MOS
				}
				break;
			 }  
			}
            else
			{
				switch (SHIP_step) // ����͹���,�м���������        
				{
					default:
						
					case 0:
					{
					 me->SH36730x0Register.SCONF2.bit.CHG_C = 0;
					 me->SH36730x0Register.SCONF2.bit.DSG_C = 0;
					 I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x05,(u8*)&me->SH36730x0Register.SCONF2.all,1);//�ر�MOS
					 
					 SHIP_step = 1;
					}
					break;
					
					case 1:
					{  
					   me->SH36730x0Register.SCONF10.PINn = 0x33;//���µ͹��Ĳ���  
					   I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ,0x0D,(u8*)&me->SH36730x0Register.SCONF10.PINn,1);
						
						SHIP_step = 2;
					}
					break;

					case 2:
					{  me->SH36730x0Register.SCONF1.bit.PD_EN = 1;
					   I2C_BQ769x0_Write(&g_I2C_SH36730x0,Addr_ZYBQ,0x04,(u8*)&me->SH36730x0Register.SCONF1.all,1);	
						
						SHIP_step = 3;
					}
					break;    

				}       
			
		   }
		
            QActive_armX((QActive *)me, 0U, 1000U);// ������ʱ��0��1000ms

            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
            QActive_disarmX((QActive *)me, 1U);                 // �رն�ʱ��1
			
            status = Q_HANDLED();
        }
        break;
        
        case I2C_SH36730x0_FINISH_SIG:                          //����ɹ�����͹���ģʽ�豸�ϵ�
        {
            QActive_disarmX((QActive *)me, 0U);                 // �رն�ʱ��0
			
			
			if(g_SystemState.State.bit.EnterBootloaderAsk ==1){ WDT_FLAG++;}
			     
			if(WDT_FLAG==2)
			{
		    	NVIC_SystemReset(); //�����λ		
			}
			
			if (SHIP_step == 3)  
			{   
				Output_LOCK_Update(&g_Output, 0);//�ر�MCU��Դ���豸�ػ�
				SHIP_step=0;
				QActive_armX((QActive *)me, 1U, 10000U);         // ִ�гɹ�
	
				//status = Q_TRAN(&AO_SH36730x0_EnterSHIP);
				status = Q_HANDLED();
			}
			else
			{
				
				status = Q_TRAN(&AO_SH36730x0_EnterSHIP); 
			}
			
        }
        break;

		case Q_TIMEOUT_SIG:
		{  
			SHIP_step=0;
            g_SystemState.State.bit.IWDGReloadEnable = 0;//��ֹι��
            Output_LOCK_Update(&g_Output, 0);//�ر�MCU��Դ���豸�ػ�
            //NVIC_SystemReset(); //MCU
            status = Q_TRAN(&AO_SH36730x0_EnterSHIP);
           // status = Q_HANDLED();
		}
		break;
		
		case I2C_SH36730x0_FAIL_SIG: //����ʧ��
		{   Output_LOCK_Update(&g_Output, 0);//�ر�MCU��Դ���豸�ػ�
            g_SystemState.State.bit.IWDGReloadEnable = 0;//��ֹMCUι��
            
		     //NVIC_SystemReset(); 
            
            status = Q_HANDLED();
		}
		break;		
		

        default: 
        {
            status = Q_SUPER(&AO_SH36730x0_Normal);// ��״̬
        }
        break;
    }
    
    return status;
}


/*
*   ͨѶ����,ֱ�ӽ���͹���״̬,���չر��豸
*/

QState AO_SH36730x0_Fault(AO_SH36730x0 * const me) {
 
    QState status;
    
    switch (Q_SIG(me)) 
    {
        case Q_ENTRY_SIG:           
        {
            AO_SH36730x0_Record(me, 100);

            //��λBQ769x0оƬʧ�ܱ�־λ�������Ϲ��ϱ�־λ�ڹ��Ϸ�������ʱ����λ������˴�����
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_FAIL);
            
            QActive_armX((QActive *)me, 0U, 1000U);     // ������ʱ��0��1000ms
            
            status = Q_HANDLED();           
        }
        break;
        
        case Q_EXIT_SIG:
        {
            QActive_disarmX((QActive *)me, 0U);         // �رն�ʱ��0
            
            status = Q_HANDLED();
        }
        break;  
        
        case Q_TIMEOUT_SIG:
        {    
			QActive_armX((QActive *)me, 0U, 1000U);     // ������ʱ��0��1000ms
			
			me->Variable.FaultEnterSHIPDelay++;
			
            //���յ�����͹���ģʽ��������ת������͹���״̬��������ת��������ȡ����ֵ״̬
            if (me->State.EnterSHIPModeAsk == 0)                // ����͹���ģʽ������Ч//me->State.EnterSHIPModeAsk = 1
            {               
				if (me->Variable.FaultEnterSHIPDelay > 3)		// ��ʱ3s�����͹���״̬
				{
					status = Q_TRAN(&AO_SH36730x0_EnterSHIP);   // ��ת������͹���״̬
				}
				else
				{
					
					status = Q_HANDLED();
				}
            }
            else  // ����͹���ģʽ������Ч����ֱ��ִ�н���͹��ĺ���
            {
                status = Q_TRAN(&AO_SH36730x0_EnterSHIP);      
            }			
        }
        break;
        
        default: 
        {
            status = Q_SUPER(&QHsm_top);  // ��״̬Ϊtop״̬       
        }
        break;
    }
    
    return status;
}


 
//==========================================================================================================================================// 
u16 soft_uv_filter_cnt0 = 0;
u16 soft_ov_filter_cnt0 = 0;
u16 bq_ov_filter_cnt0 = 0;//��ѹ����
u16 bq_uv_filter_cnt0 = 0;
u16 bq_ocd_filter_cnt0 = 0;

/*
*   AFEоƬ�ϵ��������ݳ�ʼ��
*   
*/
void SH36730x0_ParameterUpdate(AO_SH36730x0 * const me)
{    
    //�ж������־λ
    me->SH36730x0Register.INT_EN.all = 0x00;//0x6C 110 1100
    
	//ϵͳ����ʹ������
	me->SH36730x0Register.SCONF1.bit.LTCLR   = 0;//1->0���FLAG1
	me->SH36730x0Register.SCONF1.bit.CTLD_EN = 0;//CTLD����ʹ�ܿ���λ	�ŵ�MOS���ƶ�//===========================================================�ر�
	me->SH36730x0Register.SCONF1.bit.PD_EN   = 0;//�͹���״̬����λ
	me->SH36730x0Register.SCONF1.bit.WDT_EN  = 1;//���Ź�ʹ�ܿ���λ	   �κ���ЧͨѶ������ι��  ���(Ĭ��30s)ʱ�رճ�ŵ�MOS 
    me->SH36730x0Register.SCONF1.bit.SC_EN   = 1;//Ӳ����·ʹ�ܱ�������λ
	me->SH36730x0Register.SCONF1.bit.OV_EN   = 1;//Ӳ������ʹ�ܱ�������λ
	me->SH36730x0Register.SCONF1.bit.CHGR_EN = 0;//��������ʹ�ܿ���λ//
	me->SH36730x0Register.SCONF1.bit.LOAD_EN = 0;//���ؼ��ʹ�ܿ���λ//
                                                   //��������
	me->SH36730x0Register.SCONF2.bit.RESETorPF = 0;//Ĭ��Ϊ0 �����Ź����������50ms����ͨѶRESET�ܽŷ�����λ�ź�,1������������
	me->SH36730x0Register.SCONF2.bit.ALARM_C   = 0;//Ӳ����������ʱ���� ֻ����͵�ƽ
	me->SH36730x0Register.SCONF2.bit.DSG_C     = 0;//�ŵ�MOS����λ//=====
	me->SH36730x0Register.SCONF2.bit.CHG_C     = 0;//���MOS����λ//=====
	
    //VADC��VADC �ɼ�����
	me->SH36730x0Register.SCONF3.bit.VADC_EN = 1;//CADCʹ�ܿ���λ
	me->SH36730x0Register.SCONF3.bit.VADC_C  = 1;//VADCֻ�ɼ���ѹ
    me->SH36730x0Register.SCONF3.bit.CBIT_C  = 1;//CADC�ɼ�����ѡ�� 13 bit
	me->SH36730x0Register.SCONF3.bit.CADC_M  = 1;//CADC�ɼ���ʽ����Ϊ�����ɼ�  
	me->SH36730x0Register.SCONF3.bit.CADC_EN = 1;//VADCʹ�ܿ���λ
	me->SH36730x0Register.SCONF3.bit.SCAN_C  = 1;//VADCת������ѡ��λ	50ms
	
	//����ʱ��ֹ����
	me->SH36730x0Register.SCONF4.all = 0x00;   //3~10 8����ذ�       
    me->SH36730x0Register.SCONF5.all = 0x00;   
	 
	me->SH36730x0Register.SCONF6.bit.SCVn  = 1;//Ӳ����·������ѹ����ѡ��λ[2:3]   200mv         //0 ->100  1->200 2->300 3->400
	me->SH36730x0Register.SCONF6.bit.RSNSn = 1;//�ɼ���Χ0~ 200mv   ����0.5��ŷ ������Χ0~400A   //0->400 1->200  2->100 3->50
    me->SH36730x0Register.SCONF6.bit.SCTn  = 0;//Ӳ����·����������ʱ����ѡ��λ[0:1] 50us	
	me->SH36730x0Register.SCONF6.bit.RSTn  = 3;//��λ����ʱ��1S
	  
    me->SH36730x0Register.SCONF7.bit.SHSn  = 0;//��ŵ�״̬�����ֵ����λ
	me->SH36730x0Register.SCONF7.bit.WDTtn = 1;//���Ź�ʱ������ 0:30s  1:10s 2:2s 3:500ms Ĭ��ֵ����0//�������ó�10s
    me->SH36730x0Register.SCONF7.bit.OVTn  = 7;//Ӳ�����䱣����ʱ
	
	me->SH36730x0Register.SCONF8.all  = 0x02;  //Ӳ�����䱣����ֵ//����3.8V//========
	me->SH36730x0Register.SCONF9.all  = 0x7F;  //Ӳ�����䱣����ֵ
	
	me->SH36730x0Register.SCONF10.PINn= 0x00;  //���͹��Ŀ���λ


}


/*
*SH36730x0оƬ����ֵ������º���
*����Ӳ����·���ϣ�ͨѶ���ϼ������·�������ϴ���
*/

//s32 CUR_data_test = 0; // OutputDisableLevel1
//int JR_count=0,all_count=0;
u16 UV_RECHARGE=0;//Ƿѹ����־
void SH36730x0_SampleUpdate(AO_SH36730x0 * const me)
{   s32 CUR_data = 0; // OutputDisableLevel1
	u16 t_data = 0;
	s16 temp_data=0; //�¶�ת����ʱ����
    s32 t_BatteryMaxVoltage = 0;                                // ��ߵ����ѹ����λmV
    s32 t_BatteryMinVoltage = 0;                                // ��͵����ѹ����λmV
    u16 t_BatteryMaxVoltagePointer = 0;                         // ��ߵ�ѹ������
    u16 t_BatteryMinVoltagePointer = 0;                         // ��͵�ѹ������                                                          
    //u16 t_data1=0,t_data2=0;
	
    u8 i=0;
      

  //  all_count++;//�ܲ���----------------------------------------------------------------------
	
    // ------------------------- FLAG1-2�Ĵ������¹�����Ϣ -----------------------//ע��FLAG2 ��λ��־λ ��λֵΪ1
    if (((me->SH36730x0Register.FLAG1.all & 0xF)) > 0)//+(me->SH36730x0Register.FLAG2.all & 0x7)��ʱ����FLAG2
    {
		g_SystemMonitor.System.Reverse.Data[8]++;  
      
        if (me->SH36730x0Register.FLAG1.bit.TWI > 0)//ͨѶ������ʱ
        {
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_I2C_OVERTIME);
			
			g_SystemMonitor.System.Reverse.Data[11]++;  
        }
        
        //���Ź���� ,�����ȴ�һ���ʱ�����û�лָ�����ͨѶι���ͻḴλϵͳ��ȫ�������������ⲿMCU��
        if (me->SH36730x0Register.FLAG1.bit.WDT > 0)
        {   //����豸����û��Ҫ���͹�����,
            Protect_SetFaultCodeLv0(&g_Protect, FAULT_BQ769_OVRD_ALERT);
		    g_SystemMonitor.System.Reverse.Data[12]++;  
        }
        
		    // SH36730x0 ����籣�� Ӳ��
        if (me->SH36730x0Register.FLAG1.bit.OV > 0)//����һ����о��ѹ����������ֵ�����ñ�־
        {
			if (me->Variable.SampleUpdateCount > 20)//��������ǰ20�η�ֹ��
            {
                Protect_SetFaultCodeLv1(&g_Protect, FAULT_BQ769_OV);
            }
			g_SystemMonitor.System.Reverse.Data[10]++;  
        }  

           // SH36730x0 �ŵ��·��· Ӳ��
        if (me->SH36730x0Register.FLAG1.bit.SC > 0)
        {
            Protect_SetFaultCodeLv1(&g_Protect, FAULT_BQ769_SCD);
			
			g_SystemMonitor.System.Reverse.Data[10]++;  //g_SystemMonitor.System.Reverse.Data[15]
        }       

    }
//==============================================��������====================================================================================	
 
// ��ʱ��ʩ��ÿ�ζ����ݲ�������һ�ε����Ŵ������ʵ�ֵ�������ʵʱ���Ĺ���	 //����ֵ����С������ֱ������1000��ȡС������
	me->Parameter.CCGain = (s32)g_SystemParameter.System.Calibration.CurrentSensorMaxCurrent*1000 /g_SystemParameter.System.Calibration.CurrentSensorMaxVoltage;//����������ֵ����
	me->Parameter.CCOffset = 0;

	 if(me->SH36730x0Register.FLAG2.bit.CADC > 0)//CADCת�����,��ȡ��Ӳ������
	 { 
		 me->SH36730x0Register.FLAG2.bit.CADC=0;
		 me->SH36730x0Register.FLAG2.bit.VADC=0;
		 
		 
		// JR_count++;//����-------------------------------------------
		 
		 
		if(me->SH36730x0Register.CUR.CURH&0x10)//�ŵ�                                          
		{                                                                                      //R�������� ��ֵΪ2��1mR����=0.5mR=0.0005R=5/10000R����ʱҪ����Ϊŷķ����
			CUR_data=((me->SH36730x0Register.CUR.CURH<<8)+me->SH36730x0Register.CUR.CURL)|0xFFFFE000; 
			
			//CUR_data=(1000000*CUR_data)/8192;
			
			//CUR_data=(1000*CUR_data)/(16384/me->Parameter.CCGain);                                                                                  //0~400mV ��ʽ I=(1000*CUR)/(8192*R) //mA   //����ʽt_data=(1000000*t_data)/4096;  //0~1000A ����+-244mA
            CUR_data=(125*CUR_data*me->Parameter.CCGain)/2048;                                                                                   //0~200mV ��ʽ I=(1000*CUR)/(16384*R)//mA  //����ʽt_data=(1000000*t_data)/8192;  //0~500A  ����+-122mA
		    me->Output.BatteryCurrent=CUR_data;	                                              //0~100mv ��ʽ I=(1000*CUR)/(32768*R)//mA  //����ʽt_data=(1000000*t_data)/16384; //0~250A  ����+-61mA
		}                                                                                     //0~50mv ��ʽ I=(1000*CUR)/(65536*R) //mA  //����ʽt_data=(1000000*t_data)/32768;  //0~125A  ����+-31mA
		else                                                                                  //���                                           
		{	
			CUR_data=((me->SH36730x0Register.CUR.CURH<<8)+me->SH36730x0Register.CUR.CURL)&0xFFF;
			
			//CUR_data=(1000000*CUR_data)/8192;
			
			//CUR_data=(1000*CUR_data)/((16384)/(me->Parameter.CCGain*1000));
			CUR_data=(125*CUR_data*me->Parameter.CCGain)/2048;
			
			me->Output.BatteryCurrent=CUR_data;	
		}
		
//=========================================================================================
		// To be update: ���0�������������һ�����������ڵ���0ƫ���¿���ʱ�����仯���졣
        // ����������С���ݵ�ص������趨������8000mAH��أ�����������Ϊ8mA���ң���0.1%
        if (CUR_data < 280 && CUR_data > -280)
        {
            me->Output.BatteryCurrent = 0;
        }
        else
        {
            me->Output.BatteryCurrent = CUR_data;
        }
	
	    
//=========================================================================================		
		if((me->SH36730x0Register.CUR.CURH & 0x10) && ((-me->Output.BatteryCurrent)> 2000))//�ŵ����2Aʱ���ⲻ������ע��ɲ��ʱ��������䣬�����ٴο���ʱҪ����ʮ���ж�(BMS.c L:694)
		{
		  me->State.BatteryBalanceEnable =0;                                               //�رվ���
		}
		
		//�ŵ��ض�·                                                                         380A*1000  = 380000mA
        if((me->SH36730x0Register.CUR.CURH&0x10)&&((-me->Output.BatteryCurrent)> (g_SystemParameter.BMS.Battery.ShortCutDownCurrent*1000)))
		{
             if(bq_ocd_filter_cnt0++ > 5) Protect_SetFaultCodeLv1(&g_Protect,FAULT_BQ769_SCD);//��·����
		}
        //�ŵ��������                                                                  //��ֵ150A	*1000=150000   
        else if ((me->Output.BatteryCurrent<0)&&((-me->Output.BatteryCurrent)> (g_SystemParameter.BMS.Battery.OverCutDownCurrent*1000))) 
		{ 
				                                                    
          if(bq_ocd_filter_cnt0++ > 40) Protect_SetFaultCodeLv1(&g_Protect,FAULT_BQ769_OCD);//����5�η��͹��� 
			
		  g_SystemMonitor.System.Reverse.Data[9]++;  
			
        }//���������� 20A    
		else if((me->Output.BatteryCurrent>0)&&((me->Output.BatteryCurrent)> ( g_SystemParameter.BMS.Charge.ChargeOverCurrent*1000)))
		{
		
		  if(bq_ocd_filter_cnt0++ > 100) Protect_SetFaultCodeLv1(&g_Protect,FAULT_BAT_CHARGE_OVER_CURRENT);//����������
		
		}
		else 
		{
		  bq_ocd_filter_cnt0 = 0;
        }
    }
//============================================�����������=======================================================================================



//=============================================�¶ȴ���========================================================	

	temp_data=me->SH36730x0Register.TEMP[0].TEMPnH & 0xF;                //0.17*temp-270
    temp_data=temp_data<<8;
    temp_data+=me->SH36730x0Register.TEMP[0].TEMPnL;
	
	temp_data=temp_data*17-27000;
	me->Output.InnerTemperature[0]=temp_data/10;
	
	temp_data=me->SH36730x0Register.TEMP[1].TEMPnH & 0xF;                //0.17*temp-270
    temp_data=temp_data<<8;
    temp_data+=me->SH36730x0Register.TEMP[1].TEMPnL;
	
	temp_data=temp_data*17-27000;
	me->Output.InnerTemperature[1]=temp_data/10;
		 
	 
	
//======================================= ��ѹ����===============================================================================================
	me->Output.BatteryVoltage=0;//����ܵ�ѹ
	me->Variable.CellNumberPointer = 0;//�Ѽ��㵥���ѹ����
//===========================================================================================================================	
	    for (i = 0; i < 8; i++)        //��ѹ��CEEL3��ʼ��ȡ   
    {                                     
        if (me->Parameter.CellSelect[i] == 1)   //CellSelect8����ʼֵ�����޸�,��ӦӲ������AFE��VC3~VC10��Ӧ  1~8�� 
        {
            t_data = me->SH36730x0Register.CELL[i].CELLnH &0xF; //����λ
            t_data = t_data << 8;
            t_data += me->SH36730x0Register.CELL[i].CELLnL;     //�Ͱ�λ
                                                    
            me->Output.SingleVoltage[me->Variable.CellNumberPointer] = (t_data*6*1000)/4096 ;
		    //�����ѹ��ͼ����ܵ�ѹ
            me->Output.BatteryVoltage += me->Output.SingleVoltage[me->Variable.CellNumberPointer];
			
            me->Variable.CellNumberPointer++; 
        }
		
        if (me->Variable.CellNumberPointer >= me->Parameter.CellNumber)
        {
			 me->Output.SingleVoltage[8] =  me->Output.SingleVoltage[0] ;//=================������,һֱ�ѵ�һ����ѹ��ʾ�ڵھŴ�
            break;          //��ȡ��ʵ�ʵ�о�� ����forѭ��
        }
    }
//===========================�������һ����ѹ���˴���==============================================================		
		if((me->SH36730x0Register.CUR.CURH & 0x10) && ((-me->Output.BatteryCurrent)> 50000))//�ŵ���´���50Aʱ���˵�һ����ѹ��VC6	
		{
		 me->Output.SingleVoltage[0] =  me->Output.SingleVoltage[5] ;//�õ�������ѹ�����һ����ѹ
		
		}
		
//===========================================================================================

    // ---------- Ѱ�ҵ���鵥�������ߵ�ѹֵ -----------------------------
    
    t_BatteryMaxVoltage = me->Output.SingleVoltage[0];
    t_BatteryMinVoltage = me->Output.SingleVoltage[0];
    t_BatteryMaxVoltagePointer = 0;
    t_BatteryMinVoltagePointer = 0; 
    
    for (i = 1; i < me->Parameter.CellNumber; i++)// ������������飬Ѱ�������͵�ѹ
    {
         if (me->Output.SingleVoltage[i] > t_BatteryMaxVoltage)
         {
             t_BatteryMaxVoltage = me->Output.SingleVoltage[i];
             t_BatteryMaxVoltagePointer = i;
         }
         
         if (me->Output.SingleVoltage[i] < t_BatteryMinVoltage)
         {
             t_BatteryMinVoltage = me->Output.SingleVoltage[i];
             t_BatteryMinVoltagePointer = i;
         }       
    }
    
    // ����������ֵ����������ṹ�壬ע���Ӧ�ڼ���Ϊ�������ϣ�ʵ�ʵڼ����������1��2��3......��
    me->Output.SingleMaxVoltage = t_BatteryMaxVoltage;//��ߵ�ѹֵ
    me->Output.SingleMaxVoltagePointer = t_BatteryMaxVoltagePointer + 1;//����
   
    me->Output.SingleMinVoltage = t_BatteryMinVoltage;//��͵�ѹֵ
    me->Output.SingleMinVoltagePointer = t_BatteryMinVoltagePointer + 1;//����       
	
	
//------------------------------������� -------------------------------------------

	//�жϵ�����͵�ѹ�Ƿ�С�ڵ���Ƿѹ����ֵ(mv)     Ƿѹ�ж�
    if ((me->Output.SingleMinVoltage < g_SystemParameter.BMS.Battery.CellUnderVoltage))//2400mv
    {    
        g_SystemMonitor.System.Reverse.Data[7]++;
//======================================================================

        if ((me->Variable.SampleUpdateCount > 20)) //�����ϵ���βɼ���20�����ݣ�����Ƿѹ40������Ƿѹ���ϣ�Լ8��
        {
            g_Protect.Variable.Count.UnderVoltageCount ++;    //���Ƿѹ��ʱ
            if(g_Protect.Variable.Count.UnderVoltageCount > (g_SystemParameter.BMS.Protect.CellUnderVoltageDelay*5))
            {
                Protect_SetFaultCodeLv1(&g_Protect, FAULT_BQ769_UV);//����Ƿѹ����
            }
        }
	} else {
        g_Protect.Variable.Count.UnderVoltageCount = 0;
    }
    
                                       
    //��ص����ѹ
    if (me->Output.SingleMaxVoltage > g_SystemParameter.BMS.Battery.CellOverVoltage)
    {   
        if (me->Variable.SampleUpdateCount > 20 && soft_ov_filter_cnt0++ > 40)//���Ǽ����ѹ��ʱ
        {
            Protect_SetFaultCodeLv1(&g_Protect, FAULT_BQ769_OV);//���ù�ѹ����
        }
    } 
	else { soft_ov_filter_cnt0 = 0;}


	               
	
    //���ݸ��´���++�����ڱ�ʶ��ȡʱ�䣬���ڵ�������
    me->Variable.ReadValueCount++;
    me->Output.ReadValueTime = g_SystemState.Output.System1msInterruptCount;//1msʱ���жϼ��� //�������ʱ��  	
}


// ------------------------- ��ʱ���¿���������ؾ��⣬��ŵ��· --------------------------------
void SH36730x0_UpdateControl(AO_SH36730x0 * const me)
{
    // ���ϵͳ״̬�Ĵ���
    // ���ֹ��ϣ���Ӧ��־λ����λ��ֱ��д��1������ñ�־λ
    // ����λ�Զ��������ȡ����ֵʱ��������λ����λ�����¼���ϡ��˴��������Ӧ����λΪ1�Ĺ��ϣ�
    //me->SH36730x0Register.SCONF1.bit.LTCLR = 1; //1->�������ò������FLAG1��־λ

    // --------------------------- ���ݵ��������ߵ�ѹ�����þ���Ĵ��� ---------------------------------

    // ����ǰ������һ��
    me->SH36730x0Register.SCONF4.all = 0x00; //3~10 8����ذ�       
    me->SH36730x0Register.SCONF5.all = 0x00;   

	if (me->State.test_ForceBatteryBalancePointer == 0)	//������ǿ��ָ���������δʹ�ܣ�����������
	{
		if (me->State.BatteryBalanceEnable == 1)// ʹ�ܾ���
		{
			// Ĭ�ϵ�ص����ѹ������ƫ����趨ֵ����������
			if ((me->Output.SingleMaxVoltage - me->Output.SingleMinVoltage) > me->Parameter.BallanceErrVoltage)     
			{
				me->State.BatteryBalancing = 1; // ����ģ�鿪ʼ����
				me->State.BatteryBalancePointer = me->Output.SingleMaxVoltagePointer;// �����ⵥ����
				

			switch (me->Output.SingleMaxVoltagePointer)
		    {
			case 8:
			{
				me->SH36730x0Register.SCONF4.bit.CB10 = 1;
			}
			break;
			
			case 7:
			{
				me->SH36730x0Register.SCONF4.bit.CB9 = 1;
			}
			break;
								
			case 6:
			{
				me->SH36730x0Register.SCONF4.bit.CB8 = 1;
			}
			break;      

			case 5:
			{
				me->SH36730x0Register.SCONF4.bit.CB7 = 1;
			}
			break;
			
			case 4:  
			{
				me->SH36730x0Register.SCONF4.bit.CB6 = 1;
			}
			break;
								
			case 3:
			{
				me->SH36730x0Register.SCONF5.bit.CB5 = 1;
			}
			break;      

			case 2:
			{
				me->SH36730x0Register.SCONF5.bit.CB4 = 1;
			}
			break;    

			case 1:
			{
				me->SH36730x0Register.SCONF5.bit.CB3 = 1;
			}
			break;  							
		   }
	     }
	    else
	    {
		  // �����ѹ�������δ�������ƣ���ʹ�ܾ���
		  me->State.BatteryBalancing = 0;
		  me->State.BatteryBalancePointer = 0;            
	    }   
	   }
	   else //����ʹ�ܱ�־δ��λ BatteryBalanceEnable
	   {

		 me->State.BatteryBalancing = 0;//����״̬��־λ 0δ����
		 me->State.BatteryBalancePointer = 0;        
	   }
	}
	else    //ǿ�ƿ������� �����ô���
	{
		me->State.BatteryBalancing = 1;                     // ����ģ�鿪ʼ����
		me->State.BatteryBalancePointer = me->State.test_ForceBatteryBalancePointer; // ������������
		
		// ǿ��ָ���������ʹ��
		// �˴�����BQ769x0оƬ������ӱȽ��ر�������Դ������ݲ�ͬ������������ؾ����Ĵ�������

		switch (me->State.test_ForceBatteryBalancePointer)
		{
			case 8:
			{
				me->SH36730x0Register.SCONF4.bit.CB10 = 1;
			}
			break;
			
			case 7:
			{
				me->SH36730x0Register.SCONF4.bit.CB9 = 1;
			}
			break;
								
			case 6:
			{
				me->SH36730x0Register.SCONF4.bit.CB8 = 1;
			}
			break;      

			case 5:
			{
				me->SH36730x0Register.SCONF4.bit.CB7 = 1;
			}
			break;
			
			case 4:  
			{
				me->SH36730x0Register.SCONF4.bit.CB6 = 1;
			}
			break;
								
			case 3:
			{
				me->SH36730x0Register.SCONF5.bit.CB5 = 1;
			}
			break;      

			case 2:
			{
				me->SH36730x0Register.SCONF5.bit.CB4 = 1;
			}
			break;    

			case 1:
			{
				me->SH36730x0Register.SCONF5.bit.CB3 = 1;
			}
			break;  

//        if(me->SH36730x0Register.SCONF4.all>0)	
//		{
//		I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x07,(u8*)&me->SH36730x0Register.SCONF4.all,1);
//			me->SH36730x0Register.SCONF4.all=0;
//		}
//         if(me->SH36730x0Register.SCONF5.all>0)	
//		{
//		I2C_BQ769x0_Write(&g_I2C_SH36730x0, Addr_ZYBQ, 0x08,(u8*)&me->SH36730x0Register.SCONF5.all,1);
//			me->SH36730x0Register.SCONF5.all=0;
//		}			

		
		}

	}

	// ���Ƴ�ŵ�mosfet
    if (Protect_GetFaultCode(&g_Protect) == 0)    // �޹��Ϸ���
    {
        // ��س�����ʹ��
        if (me->State.CHGControl == 1)
        {
            me->SH36730x0Register.SCONF2.bit.CHG_C = 1;
        }
        else
        {
            me->SH36730x0Register.SCONF2.bit.CHG_C = 0;
        }
        
        // ��طŵ����ʹ��
        if (me->State.DSGControl == 1)
        {
           me->SH36730x0Register.SCONF2.bit.DSG_C = 1;		
        }
        else
        {
           me->SH36730x0Register.SCONF2.bit.DSG_C = 0;
        }       
    }
    else
    {
        //���й��Ϸ���ʱ��ǿ�ƹر����
        me->SH36730x0Register.SCONF2.bit.CHG_C = 0;
        me->SH36730x0Register.SCONF2.bit.DSG_C = 0;       
    }      
}


// End of AO_SH367930x0.c


 

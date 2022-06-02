/*=====================================================================================
 File name:		fault_record.h

 Originator:	GLB

 Description:	Fault counts record and fault history

=======================================================================================
 History:
---------------------------------------------------------------------------------------
 01-27-2014		Version 1.0.0
-------------------------------------------------------------------------------------*/


#include "stm32f0xx.h"                     		// STM32�����Ĵ�������ͷ�ļ�

#define	FAULT_HISTORY_LENGTH			128			// ������ʷ��¼����Ϊ128��
#define	FAULT_COUNT_LENGTH				128			// �����ۼƴ�����¼����Ϊ128��

// ���ϼ�¼ÿ��ռ4Bytes��һ��128����¼
struct FAULTHISTORY_BITS {	
	u32	TimeStamp:24;
	u32	FaultCode:8;
};

union FAULTHISTORY_UNION {
	u32								all;
	struct FAULTHISTORY_BITS		bit;
};

// ���ϴ���ÿ��ռ2BYte��һ��128����¼
struct FAULTCOUNT_BITS {
	u32	FaultCodeCount:24;
	u32	FaultCode:8;
};

union FAULTCOUNT_UNION {
	u32							all;
	struct FAULTCOUNT_BITS		bit;
};


typedef struct FAULTHISTORY {
	union FAULTHISTORY_UNION	FaultHistory[FAULT_HISTORY_LENGTH];
	union FAULTCOUNT_UNION		FaultCount[FAULT_COUNT_LENGTH];
}FAULT_HISTORY_structDef;



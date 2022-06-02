/*=====================================================================================
 File name:		fault_record.h

 Originator:	GLB

 Description:	Fault counts record and fault history

=======================================================================================
 History:
---------------------------------------------------------------------------------------
 01-27-2014		Version 1.0.0
-------------------------------------------------------------------------------------*/


#include "stm32f0xx.h"                     		// STM32器件寄存器定义头文件

#define	FAULT_HISTORY_LENGTH			128			// 故障历史记录长度为128个
#define	FAULT_COUNT_LENGTH				128			// 故障累计次数记录长度为128个

// 故障记录每个占4Bytes，一共128个记录
struct FAULTHISTORY_BITS {	
	u32	TimeStamp:24;
	u32	FaultCode:8;
};

union FAULTHISTORY_UNION {
	u32								all;
	struct FAULTHISTORY_BITS		bit;
};

// 故障次数每个占2BYte，一共128个记录
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


